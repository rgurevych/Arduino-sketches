//Mega Display Clock by Rostyslav Gurevych

#include "MAX7219.h"
#include "dig3.h"   // Font for the clock. Select between dig1, dig2, dig3
#include <Wire.h>
#include <GyverTimer.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <DST_RTC.h>


#define MAX_CS 15  // CS pin
#define MOSI   13  // MOSI pin
#define CLK    14  // CLK pin  
#define DW 6       // number of 7219 chips horizontaly 
#define DH 3       // number of 7219 chips vertically
#define DWW (DW*4)  
#define DHH (DH*2)  

//WiFi settings
const char* ssid = "Gurevych_2";
const char* password = "3Gurevych+1Mirkina";

// Define EU rules for DST
const char rulesDST[] = "EU";

// Initialize defices
MaxDisp<MAX_CS, DW, DH> disp;
RTC_DS3231 rtc;
DST_RTC dst_rtc;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Set timers
GTimer WiFiCheckTimer(MS, 2000);
GTimer oneSecondTimer(MS, 1000);
GTimer halfSecondTimer(MS, 500);

// Set variables
DateTime now;
byte secs, mins, hrs;
boolean dots = true;
boolean WiFiInitialConnected = false, WiFiConnected;
boolean autoUpdateTimeDoneFlag = false;
byte timezone = 2;
long utcOffsetInSeconds = 3600*timezone;


void setup() {

// Pin mode for LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

delay(1500);

// Setting up display  
  disp.begin();
  disp.setBright(10);
  disp.textDisplayMode(GFX_ADD);
  disp.clear();

  Serial.begin(9600);
  
// Setting up RTC module
  rtc.begin();
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.print("Adjusting date and time");
  }
  now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();
  Serial.println("RTC ready");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

// Setting up NTP time client
  timeClient.begin();
  timeClient.setTimeOffset(utcOffsetInSeconds);
}


void loop() {
  wifiTick();
  timeTick();
}


void timeTick(){
  if (halfSecondTimer.isReady()){
    if(dots){
      secs++;
    }
    dots = !dots;
  
    if (secs > 59){
      now = rtc.now();
      secs = now.second();
      mins = now.minute();
      hrs = now.hour();
      automaticTimeUpdate();
    }  
    displayTime();

    if(dots) printTimeSerial();
  } 
}

void automaticTimeUpdate() {
  if (mins == 30 && !autoUpdateTimeDoneFlag) {
    if (WiFiConnected) {
      updateTime();
    }
    autoUpdateTimeDoneFlag = true;
  }
  else if (mins != 30) {
    autoUpdateTimeDoneFlag = false;
  }
}


void wifiTick(){
  if(WiFiCheckTimer.isReady()){
    checkWiFi(); 
  
    if(!WiFiInitialConnected){
      if(!WiFiConnected){
        Serial.print(".");   
      }
      else{
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);
        WiFiInitialConnected = true;
      }
    }
    digitalWrite(LED_BUILTIN, !WiFiConnected);
  }
}


void checkWiFi(){
  if(WiFi.status() == WL_CONNECTED){
    WiFiConnected = true;
  }
  else {
    WiFiConnected = false;
  }
}

void printTimeSerial(){
  Serial.print(hrs); Serial.print(":");
  Serial.print(mins); Serial.print(":");
  Serial.println(secs);
}


void updateTime(){
  Serial.println("Updating time from NTP server...");
  timeClient.update();
  
  int8_t new_hour = timeClient.getHours();
  int8_t new_minute = timeClient.getMinutes();
  int8_t new_second = timeClient.getSeconds();

  //Get a time structure
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int8_t new_day = ptm->tm_mday;
  int8_t new_month = ptm->tm_mon+1;
  int8_t new_year = ptm->tm_year-100;

  if (new_year + 2000 == now.year()) {
    DateTime newTime = dst_rtc.calculateTime(DateTime(2000+new_year, new_month, new_day, new_hour, new_minute, new_second));  
    rtc.adjust(newTime);
    Serial.println("Time updated successfully");
  }
  else{
    Serial.println("Something went wrong, time was not updated");
  }
  
}

//void running() {
//  // бегущая
//  disp.setScale(3);
//  char* txt = "Лайк, подписка, колокольчик! =) AlexGyver Show";
//  int w = strlen(txt) * 5 * 3 + disp.W();
//  for (int x = disp.W(); x > -w; x--) {
//    disp.clear();
//    disp.setCursor(x, 6);
//    disp.print(txt);
//    disp.update();
//    delay(10);
//  }
//}


void drawDigit(byte dig, int x) {
  disp.drawBitmap(x, 0, (const uint8_t*)pgm_read_dword(&(digs[dig])), d_width, 36, 0);
}

void displayTime() {
  byte minsShift = 0, minsLastShift = 0, hoursShift = 0, hoursLastShift = 0;
  disp.clear();
  
  if(hrs > 9){
    if(hrs / 10 == 2 && hrs % 10 != 0) hoursShift = 2; 
    drawDigit(hrs / 10, -3 + hoursShift);
  }
  
  if (hrs % 10 == 1) hoursLastShift = 2;
  drawDigit(hrs % 10, d_width - 1 - hoursLastShift);
  
  if (dots) {
    disp.setByte(11, 2, 0b1100011);
    disp.setByte(11, 4, 0b1100011);
  }
  if(mins / 10 == 1) minsShift = 2;
  drawDigit(mins / 10, 95 - d_width * 2 - 4 - minsShift);
  if(mins % 10 == 1) minsLastShift = 2;
  drawDigit(mins % 10, 95 - d_width - minsShift*2 - minsLastShift);
  disp.update();
}
