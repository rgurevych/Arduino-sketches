#include <GyverTimer.h>         //Library for work with timer
#include <SoftwareSerial.h>     //Library for work with serial port
#include <DHT.h>                //Library for work with DHT sensor
#include <MHZ19_uart.h>         //Library for work with MH-19Z sensor
// #include <Wire.h>               //Library for work with LED display


// Pins:
#define DHTPIN 2   //DHT-22 signal pin
#define RX_PIN 8   //Serial rx pin no
#define TX_PIN 9   //Serial tx pin no


// Timers:
GTimer measureTimer(MS, 30000);
GTimer printTimer(MS, 15000);
GTimer hourlyTimer(MS, 3600000);


// CO2 sensor:
MHZ19_uart mhz19;


// DHT Sensor:
DHT dht(DHTPIN, DHT22);


// Initial variables:
long t = 0;
int current_ppm = 0;
int max_ppm = 0;
int min_ppm = 5000;
float current_temp = 0;
float current_hum = 0;


void setup() {
  // Serial
  Serial.begin(9600);

  // MH-Z19
  mhz19.begin(RX_PIN, TX_PIN);
  //mhz19.setAutoCalibration(false);  //  uncomment this to disable autocalibration
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
  if (measureTimer.isReady()){
    current_ppm = measureCO2();
    current_temp = measureTemp();
    current_hum = measureHum();
    
    if (current_ppm > max_ppm) max_ppm = current_ppm;
    if (current_ppm < min_ppm) min_ppm = current_ppm;
  }

  if (printTimer.isReady()){
    Serial.print(String(t)); Serial.print(" - CO2: "); Serial.print(current_ppm); Serial.print(" ppm\t");
    Serial.print("Humidity: "); Serial.print(current_hum); Serial.print(" %\t"); Serial.print("Temperature: "); Serial.print(current_temp); Serial.println(" *C.");
    t += 15;
  }

  if (hourlyTimer.isReady()){
    Serial.print("Max value for last hour: "); Serial.print(max_ppm); Serial.print("; Min value for last hour:"); Serial.println(min_ppm);
    max_ppm = 0;
    min_ppm = 5000;
  }
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
