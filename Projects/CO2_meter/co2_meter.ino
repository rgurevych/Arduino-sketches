#include <GyverTimer.h>         //Library for working with timer
#include <SoftwareSerial.h>     //Library for working with serial port
#include <DHT.h>                //Library for working with DHT sensor
#include <MHZ19_uart.h>         //Library for working with MH-19Z sensor
#include <GyverButton.h>        //Library for working with button
#include <Wire.h>               //Library for working with LED display
#include <LiquidCrystal_I2C.h>  //Library for working with LED display
#include <RTClib.h>             //Library for working with RTC clock module


// Pins:
#define DHTPIN 2            //DHT-22 signal pin
#define RX_PIN 3            //Serial rx pin no
#define TX_PIN 4            //Serial tx pin no
#define BUTTON1_PIN 7       //Button 1 pin
#define BUTTON2_PIN 8       //Button 1 pin
#define RED_LED_PIN 11      //Red LED pin
#define YELLOW_LED_PIN 10   //Yellow LED pin
#define GREEN_LED_PIN 9     //Green LED pin
#define BACKLIGHT 5         //LCD backlight pin


// Timer durations
#define MEASURE_TIMEOUT 15000             //Interval between measure occurs
#define PRINT_TIMEOUT 30000               //Interval between serial printout occurs
#define WARMING_UP_TIMEOUT 175000         //Duration of warming up period
#define BLINK_TIMEOUT 1000                //LED blinking interval


// Settings
#define RESET_CLOCK 0                     //Should the clock be reset on start
#define DEBUG 1                           //Debug mode, in which the data is printed to serial port


// Timers:
GTimer measureTimer(MS, MEASURE_TIMEOUT);
GTimer printTimer(MS, PRINT_TIMEOUT);
GTimer fiveMinTimer(MS, 300000);
GTimer hourlyTimer(MS, 3600000);
GTimer warmingTimer(MS);
GTimer blinkTimer(MS, BLINK_TIMEOUT);


// Buttons
GButton button1(BUTTON1_PIN);
GButton button2(BUTTON2_PIN);


// CO2 sensor:
MHZ19_uart mhz19;


// DHT Sensor:
DHT dht(DHTPIN, DHT22);


//Liquid Crystal LCD:
LiquidCrystal_I2C lcd(0x27, 16, 2);


//Real Time Clock:
RTC_DS3231 rtc;
DateTime now;


// Initial variables:
int8_t hrs, mins, secs;
long t = PRINT_TIMEOUT/1000;
int current_ppm = 0;
int max_5min_ppm = 0;
int min_5min_ppm = 5000;
int max_hour_ppm = 0;
int min_hour_ppm = 5000;

int current_temp = 0;
int max_5min_temp = 0;
int min_5min_temp = 1000;
int max_hour_temp = 0;
int min_hour_temp = 1000;

int current_hum = 0;
int max_5min_hum = 0;
int min_5min_hum = 1000;
int max_hour_hum = 0;
int min_hour_hum = 1000;

int yellow_ppm_level = 850;
int red_ppm_level = 1200;
int alarm_ppm_level = 1600;

int minHourPpmArray[12], maxHourPpmArray[12];
int minDayPpmArray[24], maxDayPpmArray[24];
int minHourTempArray[12], maxHourTempArray[12];
int minDayTempArray[24], maxDayTempArray[24];
int minHourHumArray[12], maxHourHumArray[12];
int minDayHumArray[24], maxDayHumArray[24];

boolean ledState = HIGH;

byte LCD_BRIGHTNESS = 200;

// Flags
boolean warmedUpFlag = false;
boolean lcdBacklight = true;


void setup() {
  // Serial
  Serial.begin(9600);

  //Pin modes
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BACKLIGHT, OUTPUT);

  // Start warming up timer
  warmingTimer.setTimeout(WARMING_UP_TIMEOUT);

  //LCD
  analogWrite(BACKLIGHT, LCD_BRIGHTNESS);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // MH-Z19
  mhz19.begin(RX_PIN, TX_PIN);
  mhz19.setAutoCalibration(false);  //  uncomment this to disable autocalibration
  mhz19.getStatus();    // first request, should return -1
  delay(500);
  lcd.setCursor(0, 0);
  if (mhz19.getStatus() == 0) {
    lcd.print(F("MH-Z19 OK"));
    if (DEBUG) {
      Serial.println(F("MH-Z19 OK"));
      }
  } 
  else {
    lcd.print(F("MH-Z19 ERROR"));
    if (DEBUG) {
    Serial.println(F("MH-Z19 ERROR"));
    }
  }
  
  // DHT
  dht.begin();
  lcd.setCursor(0, 1);
  if (isnan(measureTemp()) || isnan(measureHum())) {
    lcd.print(F("DHT ERROR"));
    if (DEBUG) {
    Serial.println(F("DHT ERROR"));
    }
  }
  else {
    lcd.print(F("DHT OK"));
    if (DEBUG) {
    Serial.println(F("DHT OK"));
    }
  }

  // RTC module
  if (RESET_CLOCK || rtc.lostPower())
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();
}


void loop(){

  button1.tick();
  button2.tick();
  checkButtons();
  measure();
  printCurrentValues();
  updateData();
  switchLed(current_ppm);
  
}


int measureCO2(){
  int ppm = mhz19.getPPM();
  return ppm;
}


int measureTemp(){
  int temp = dht.readTemperature() * 10;
  return temp;
}


int measureHum(){
  int hum = dht.readHumidity() * 10;
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
    
    if (current_temp > max_5min_temp) max_5min_temp = current_temp;
    if (current_temp < min_5min_temp) min_5min_temp = current_temp;

    if (current_hum > max_5min_hum) max_5min_hum = current_hum;
    if (current_hum < min_5min_hum) min_5min_hum = current_hum;
  }
    
  if (fiveMinTimer.isReady()){
    for (byte i = 0; i < 11; i++) {
      minHourPpmArray[i] = minHourPpmArray[i + 1];
      maxHourPpmArray[i] = maxHourPpmArray[i + 1];
      minHourTempArray[i] = minHourTempArray[i + 1];
      maxHourTempArray[i] = maxHourTempArray[i + 1];
      minHourHumArray[i] = minHourHumArray[i + 1];
      maxHourHumArray[i] = maxHourHumArray[i + 1];
    }
    
    minHourPpmArray[11] = min_5min_ppm;
    maxHourPpmArray[11] = max_5min_ppm;
    minHourTempArray[11] = min_5min_temp;
    maxHourTempArray[11] = max_5min_temp;
    minHourHumArray[11] = min_5min_hum;
    maxHourHumArray[11] = max_5min_hum;
     
    max_5min_ppm = 0;
    min_5min_ppm = 5000;
    max_5min_temp = 0;
    min_5min_temp = 1000;
    max_5min_hum = 0;
    min_5min_hum = 1000;
  }

    if (hourlyTimer.isReady()){

    max_hour_ppm = calculateMaxFromArray(maxHourPpmArray, 12);
    min_hour_ppm = calculateMinFromArray(minHourPpmArray, 12);
    max_hour_temp = calculateMaxFromArray(maxHourTempArray, 12);
    min_hour_temp = calculateMinFromArray(minHourTempArray, 12);
    max_hour_hum = calculateMaxFromArray(maxHourHumArray, 12);
    min_hour_hum = calculateMinFromArray(minHourHumArray, 12);
    
    for (byte i = 0; i < 23; i++) {
      minDayPpmArray[i] = minDayPpmArray[i + 1];
      maxDayPpmArray[i] = maxDayPpmArray[i + 1];
      minDayTempArray[i] = minDayTempArray[i + 1];
      maxDayTempArray[i] = maxDayTempArray[i + 1];
      minDayHumArray[i] = minDayHumArray[i + 1];
      maxDayHumArray[i] = maxDayHumArray[i + 1];
    }
    minDayPpmArray[23] = min_hour_ppm;
    maxDayPpmArray[23] = max_hour_ppm;
    minDayTempArray[23] = min_hour_temp;
    maxDayTempArray[23] = max_hour_temp;
    minDayHumArray[23] = min_hour_hum;
    maxDayHumArray[23] = max_hour_hum;
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


void printCurrentValues(){  
  if (printTimer.isReady()){
    now = rtc.now();
    secs = now.second();
    mins = now.minute();
    hrs = now.hour();
    
    if (DEBUG) {
    Serial.print(String(t));
    Serial.print(F(" - "));
    Serial.print(hrs);
    Serial.print(F(":"));
    Serial.print(mins);
    Serial.print(F(":"));
    Serial.print(secs);
    
    Serial.print(F(" - CO2: "));
    if (!warmedUpFlag){
      Serial.print(F("**"));
      }
    Serial.print(current_ppm); 
    Serial.print(F(" ppm\t"));
    Serial.print(F("Humidity: ")); 
    Serial.print(current_hum / 10.0, 1); 
    Serial.print(F(" %\t")); 
    Serial.print(F("Temperature: ")); 
    Serial.print(current_temp / 10.0, 1); 
    Serial.println(F(" *C."));
    t += PRINT_TIMEOUT/1000;
    }
  
    //print to LCD:
    if (warmedUpFlag) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("CO2:"));
    lcd.print(current_ppm);
    lcd.print(F(" ppm"));
    lcd.setCursor(0, 1);
    lcd.print(F("T:"));
    lcd.print(current_temp / 10.0, 1);
    lcd.print(F("C H:"));
    lcd.print(current_hum / 10.0, 1);
    lcd.print(F("%"));
    }
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


void checkButtons(){
  if (button1.isClick()){
    printOutAllArrays();
  }

  if (button2.isClick()){
    switchLcdBacklight();
  }

  if (button2.isHolded()){
    updateLcdBrightness();
  }
}


// Switch LCD backlight on and off
void switchLcdBacklight(){
  lcdBacklight = !lcdBacklight;
  lcd.setBacklight(lcdBacklight);
}


// Change the LCD backlight brightness
void updateLcdBrightness(){
  if (LCD_BRIGHTNESS == 200) {
    LCD_BRIGHTNESS = 0;
  }
  else {
    LCD_BRIGHTNESS += 25;
  }

  if (!lcdBacklight) {
    switchLcdBacklight();
  }
  
  analogWrite(BACKLIGHT, LCD_BRIGHTNESS);
}


void printOutAllArrays(){
  // Print out hourly array values
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

    Serial.print(F("maxHourTempArray = "));
    for (byte i = 0; i < 12; i++) {
      Serial.print(maxHourTempArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("minHourTempArray = "));
    for (byte i = 0; i < 12; i++) {
      Serial.print(minHourTempArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("maxHourHumArray = "));
    for (byte i = 0; i < 12; i++) {
      Serial.print(maxHourHumArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("minHourHumArray = "));
    for (byte i = 0; i < 12; i++) {
      Serial.print(minHourHumArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));
    
    // Print out daily array values
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

    Serial.print(F("maxDayTempArray = "));
    for (byte i = 0; i < 24; i++) {
      Serial.print(maxDayTempArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("minDayTempArray = "));
    for (byte i = 0; i < 24; i++) {
      Serial.print(minDayTempArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("maxDayHumArray = "));
    for (byte i = 0; i < 24; i++) {
      Serial.print(maxDayHumArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("minDayHumArray = "));
    for (byte i = 0; i < 24; i++) {
      Serial.print(minDayHumArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    // Print out min and max values for last 5 minutes
    Serial.print(F("Max CO2 value for last 5 minutes: ")); 
    Serial.print(max_5min_ppm); 
    Serial.print(F("; Min CO2 value for last 5 minutes:")); 
    Serial.println(min_5min_ppm);

    Serial.print(F("Max Temp value for last 5 minutes: ")); 
    Serial.print(max_5min_temp / 10.0, 1); 
    Serial.print(F("; Min Temp value for last 5 minutes:")); 
    Serial.println(min_5min_temp / 10.0, 1);

    Serial.print(F("Max Humidity value for last 5 minutes: ")); 
    Serial.print(max_5min_hum / 10.0, 1); 
    Serial.print(F("; Min Humidity value for last 5 minutes:")); 
    Serial.println(min_5min_hum / 10.0, 1);
    
    // Calculate and print out min and max values for last hour
    int current_max_hour_ppm = calculateMaxFromArray(maxHourPpmArray, 12);
    int current_min_hour_ppm = calculateMinFromArray(minHourPpmArray, 12);
    int current_max_hour_temp = calculateMaxFromArray(maxHourTempArray, 12);
    int current_min_hour_temp = calculateMinFromArray(minHourTempArray, 12);
    int current_max_hour_hum = calculateMaxFromArray(maxHourHumArray, 12);
    int current_min_hour_hum = calculateMinFromArray(minHourHumArray, 12);

    current_max_hour_ppm = max(current_max_hour_ppm, max_5min_ppm);
    current_min_hour_ppm = min(current_min_hour_ppm, min_5min_ppm);
    current_max_hour_temp = max(current_max_hour_temp, max_5min_temp);
    current_min_hour_temp = min(current_min_hour_temp, min_5min_temp);
    current_max_hour_hum = max(current_max_hour_hum, max_5min_hum);
    current_min_hour_hum = min(current_min_hour_hum, min_5min_hum);

    Serial.print(F("Max CO2 value for last hour: ")); 
    Serial.print(current_max_hour_ppm); 
    Serial.print(F("; Min CO2 value for last hour:")); 
    Serial.println(current_min_hour_ppm);

    Serial.print(F("Max Temp value for last hour: ")); 
    Serial.print(current_max_hour_temp / 10.0, 1); 
    Serial.print(F("; Min Temp value for last hour:")); 
    Serial.println(current_min_hour_temp / 10.0, 1);

    Serial.print(F("Max Humidity value for last hour: ")); 
    Serial.print(current_max_hour_hum / 10.0, 1); 
    Serial.print(F("; Min Humidity value for last hour:")); 
    Serial.println(current_min_hour_hum / 10.0, 1);
    
    // Calculate and print out min and max values for last day
    int current_max_day_ppm = calculateMaxFromArray(maxDayPpmArray, 24);
    int current_min_day_ppm = calculateMinFromArray(minDayPpmArray, 24);
    int current_max_day_temp = calculateMaxFromArray(maxDayTempArray, 24);
    int current_min_day_temp = calculateMinFromArray(minDayTempArray, 24);
    int current_max_day_hum = calculateMaxFromArray(maxDayHumArray, 24);
    int current_min_day_hum = calculateMinFromArray(minDayHumArray, 24);

    current_max_day_ppm = max(current_max_day_ppm, current_max_hour_ppm);
    current_min_day_ppm = min(current_min_day_ppm, current_min_hour_ppm);
    current_max_day_temp = max(current_max_day_temp, current_max_hour_temp);
    current_min_day_temp = min(current_min_day_temp, current_min_hour_temp);
    current_max_day_hum = max(current_max_day_hum, current_max_hour_hum);
    current_min_day_hum = min(current_min_day_hum, current_min_hour_hum);

    Serial.print(F("Max CO2 value for last 24 hours: ")); 
    Serial.print(current_max_day_ppm); 
    Serial.print(F("; Min CO2 value for last 24 hours:")); 
    Serial.println(current_min_day_ppm);
    
    Serial.print(F("Max Temp value for last 24 hours: ")); 
    Serial.print(current_max_day_temp / 10.0, 1); 
    Serial.print(F("; Min Temp value for last 24 hours:")); 
    Serial.println(current_min_day_temp / 10.0, 1);
    
    Serial.print(F("Max Humidity value for last 24 hours: ")); 
    Serial.print(current_max_day_hum / 10.0, 1); 
    Serial.print(F("; Min Humidity value for last 24 hours:")); 
    Serial.println(current_min_day_hum / 10.0, 1);
}
