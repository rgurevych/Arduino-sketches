//Mega Display Clock by Rostyslav Gurevych

#include "MAX7219.h"
#include "dig3.h"   // Font for the clock. Select between dig1, dig2, dig3
#include <Wire.h>
#include <GyverTimer.h>
#include <RTClib.h>


#define MAX_CS 15  // CS pin
#define DW 6      // number of 7219 chips horizontaly 
#define DH 3      // number of 7219 chips vertically

#define DWW (DW*4)  
#define DHH (DH*2)  

// Initialize defices
MaxDisp<MAX_CS, DW, DH> disp;
RTC_DS3231 rtc;

// Set timers
GTimer oneSecondTimer(MS, 1000);
GTimer halfSecondTimer(MS, 500);

// Set variables
DateTime now;
byte secs, mins, hrs;
boolean dots = true;


void setup() {
  Serial.begin(9600);
  
  randomSeed(analogRead(0));
  disp.begin();
  disp.setBright(15);
  disp.textDisplayMode(GFX_ADD);


// Setting up RTC module and display time
  rtc.begin();
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.print("Adjusting date and time");
  }
  now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();
  Serial.print("Setup ready");
}

void loop() {
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
    }
    printTimeSerial();
    displayTime();
  }
  
}


void printTimeSerial(){
  Serial.print(hrs); Serial.print(":");
  Serial.print(mins); Serial.print(":");
  Serial.println(secs);
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
#ifdef __AVR__
  disp.drawBitmap(x, 0, (const uint8_t*)pgm_read_word(&(digs[dig])), d_width, 36, 0);
#else
  disp.drawBitmap(x, 0, (const uint8_t*)pgm_read_dword(&(digs[dig])), d_width, 36, 0);
#endif
}

void displayTime() {
  disp.clear();
  if (hrs > 9) drawDigit(hrs / 10, 0);
  drawDigit(hrs % 10, d_width + 2);
  if (dots) {
    disp.setByte(11, 2, 0b0011101);
    disp.setByte(11, 4, 0b1100011);
  }
  drawDigit(mins / 10, 95 - d_width * 2 - 3);
  drawDigit(mins % 10, 95 - d_width);
  disp.update();
}
