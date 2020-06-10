#include <GyverTimer.h>         //Library for work with timer
#include <SoftwareSerial.h>     //Library for work with serial port
#include <DHT.h>                //Library for work with DHT sensor
#include <MHZ19_uart.h>         //Library for work with MH-19Z sensor
// #include <Wire.h>               //Library for work with LED display


// Pins:
#define DHTPIN 2            //DHT-22 signal pin
#define RX_PIN 3            //Serial rx pin no
#define TX_PIN 4            //Serial tx pin no
#define BUTTON1_PIN 7       //Button 1 pin
#define RED_LED_PIN 11      //Red LED pin
#define YELLOW_LED_PIN 10   //Yellow LED pin
#define GREEN_LED_PIN 9     //Green LED pin


// Timer durations
#define MEASURE_TIMEOUT 15000             //Interval between measure occurs
#define PRINT_TIMEOUT 30000               //Interval between serial printout occurs
#define WARMING_UP_TIMEOUT 175000         //Duration of warming up period
#define BLINK_TIMEOUT 1000                //LED blinking interval


// Timers:
GTimer measureTimer(MS, MEASURE_TIMEOUT);
GTimer printTimer(MS, PRINT_TIMEOUT);
GTimer fiveMinTimer(MS, 300000);
GTimer hourlyTimer(MS, 3600000);
GTimer warmingTimer(MS);
GTimer blinkTimer(MS, BLINK_TIMEOUT);


// CO2 sensor:
MHZ19_uart mhz19;


// DHT Sensor:
DHT dht(DHTPIN, DHT22);


// Initial variables:
long t = PRINT_TIMEOUT/1000;
int current_ppm = 0;
int max_5min_ppm = 0;
int min_5min_ppm = 5000;
int max_hour_ppm = 0;
int min_hour_ppm = 5000;
float current_temp = 0;
float current_hum = 0;

int yellow_ppm_level = 850;
int red_ppm_level = 1200;
int alarm_ppm_level = 1600;

int minHourPpmArray[12], maxHourPpmArray[12];
int minDayPpmArray[24], maxDayPpmArray[24];

boolean ledState = HIGH;

// Flags
boolean warmedUpFlag = false;

void setup() {
  // Serial
  Serial.begin(9600);

  //Pin modes
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  // Start warming up timer
  warmingTimer.setTimeout(WARMING_UP_TIMEOUT);

  // MH-Z19
  mhz19.begin(RX_PIN, TX_PIN);
  mhz19.setAutoCalibration(false);  //  uncomment this to disable autocalibration
  mhz19.getStatus();    // first request, should return -1
  delay(500);
  if (mhz19.getStatus() == 0) {
    Serial.println(F("MH-Z19 OK"));
  } 
  else {
    Serial.println(F("MH-Z19 ERROR"));
  }
  
  // DHT
  dht.begin();
  if (isnan(measureTemp()) || isnan(measureHum())) {
    Serial.println(F("DHT ERROR"));
  }
  else {
    Serial.println(F("DHT OK"));
  }
}


void loop(){
  
  measure();
  printSerialCurrentValues();
  updateData();
  switchLed(current_ppm);
  
}


int measureCO2(){
  int ppm = mhz19.getPPM();
  return ppm;
}


float measureTemp(){
  float temp = dht.readTemperature();
  return temp;
}


float measureHum(){
  float hum = dht.readHumidity();
  return hum;
}


void measure(){  
  if (measureTimer.isReady()){
    current_ppm = measureCO2();
    current_temp = measureTemp();
    current_hum = measureHum();
  }
}


void updateData(){
  if (warmedUpFlag){
    if (current_ppm > max_5min_ppm) max_5min_ppm = current_ppm;
    if (current_ppm < min_5min_ppm) min_5min_ppm = current_ppm;
  }
  
  if (fiveMinTimer.isReady()){
    Serial.print(F("Max value for last 5 minutes: ")); Serial.print(max_5min_ppm); Serial.print(F("; Min value for last 5 minutes:")); Serial.println(min_5min_ppm);

    for (byte i = 0; i < 11; i++) {
      minHourPpmArray[i] = minHourPpmArray[i + 1];
      maxHourPpmArray[i] = maxHourPpmArray[i + 1];
    }
    minHourPpmArray[11] = min_5min_ppm;
    maxHourPpmArray[11] = max_5min_ppm;

    Serial.print(F("maxHourPpmArray = "));
    for (byte i = 0; i < 12; i++) {
      Serial.print(maxHourPpmArray[i]);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("minHourPpmArray = "));
    for (byte i = 0; i < 12; i++) {
      Serial.print(minHourPpmArray[i]);
      Serial.print(F("; "));
    }
    Serial.println(F(""));
     
    max_5min_ppm = 0;
    min_5min_ppm = 5000;
  }

    if (hourlyTimer.isReady()){

    max_hour_ppm = calculateMaxFromArray(maxHourPpmArray, 12);
    min_hour_ppm = calculateMinFromArray(minHourPpmArray, 12);
    
    Serial.print(F("Max value for last hour: ")); 
    Serial.print(max_hour_ppm); 
    Serial.print(F("; Min value for last hour:")); 
    Serial.println(min_hour_ppm);

    for (byte i = 0; i < 23; i++) {
      minDayPpmArray[i] = minDayPpmArray[i + 1];
      maxDayPpmArray[i] = maxDayPpmArray[i + 1];
    }
    minDayPpmArray[23] = min_hour_ppm;
    maxDayPpmArray[23] = max_hour_ppm;

    Serial.print(F("maxDayPpmArray = "));
    for (byte i = 0; i < 24; i++) {
      Serial.print(maxDayPpmArray[i]);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("minDayPpmArray = "));
    for (byte i = 0; i < 24; i++) {
      Serial.print(minDayPpmArray[i]);
      Serial.print(F("; "));
    }
    Serial.println(F(""));
  }
}


int calculateMaxFromArray(int *dataArray, byte arrSize) {
  int max_value = 0;
  for (byte i = 0; i < arrSize; i++) {
    if (dataArray[i] > max_value){
      max_value = dataArray[i];
    }
  }

  return max_value;
}


int calculateMinFromArray(int *dataArray, byte arrSize) {
  int min_value = 5000;
  for (byte i = 0; i < arrSize; i++) {
    if (dataArray[i] < min_value && dataArray[i] != 0){
      min_value = dataArray[i];
    }
  }

  return min_value;
}


void printSerialCurrentValues(){  
  if (printTimer.isReady()){
    Serial.print(String(t)); 
    Serial.print(F(" - CO2: "));
    if (!warmedUpFlag){
      Serial.print(F("**"));
    }
    Serial.print(current_ppm); 
    Serial.print(F(" ppm\t"));
    Serial.print(F("Humidity: ")); 
    Serial.print(current_hum); 
    Serial.print(F(" %\t")); 
    Serial.print(F("Temperature: ")); 
    Serial.print(current_temp); 
    Serial.println(F(" *C."));
    t += PRINT_TIMEOUT/1000; 
  }
}

void switchLed(int ppm_level){
  if (warmingTimer.isReady()){
    warmedUpFlag = true;
  }
  
  if (!warmedUpFlag) {
    blinkLed(YELLOW_LED_PIN);
  }
  
  else if (ppm_level <= yellow_ppm_level){
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
  }

  else if (ppm_level > yellow_ppm_level && ppm_level <= red_ppm_level){
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
  }

  else if (ppm_level > red_ppm_level && ppm_level <= alarm_ppm_level){
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
  }

  else {
    blinkLed(RED_LED_PIN);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
  }
}

void blinkLed(byte led_pin){
  if (blinkTimer.isReady()){
      digitalWrite(led_pin, ledState);
      ledState =! ledState;
    } 
}
