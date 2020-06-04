#include <GyverTimer.h>         //Library for work with timer
#include <SoftwareSerial.h>     //Library for work with serial port
#include <DHT.h>                //Library for work with DHT sensor
// #include <Wire.h>               //Library for work with LED display


// Pins:
#define DHTPIN 2


// Timers:
GTimer measureTimer(MS, 30000);
GTimer printTimer(MS, 15000);
GTimer hourlyTimer(MS, 3600000);


// CO2 sensor:
SoftwareSerial mySerial(8,9); // RX,TX
byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
unsigned char response[9];


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
  mySerial.begin(9600);
  
  // DHT
  dht.begin();
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
  mySerial.write(cmd, 9);
  memset(response, 0, 9);
  mySerial.readBytes(response, 9);
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc+=response[i];
  crc = 255 - crc;
  crc++;
  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    Serial.println("CRC error: " + String(crc) + " / "+ String(response[8]));
    while (mySerial.available()) {
        mySerial.read();       
        }
    return -1;
  } 
  else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    unsigned int ppm = (256*responseHigh) + responseLow;
    return ppm;
  }
}


float measureTemp(){
  float temp = dht.readTemperature();
  return temp;
}


float measureHum(){
  float hum = dht.readHumidity();
  return hum;
}
