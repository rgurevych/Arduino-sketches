//Wall clock by Rostyslav Gurevych

//---------- Define pins and settings

#define STRIP_PIN 5                 // LED strip control pin
#define BUTTON_1_PIN 8              // button 1
#define BUTTON_2_PIN 9              // button 1


#define COLOR_DEBTH 3
#define RESET_CLOCK 0               // Should the RTC be reset?
#define NUMLEDS 20                  // Number of LEDs in the strip
#define INIT_ADDR 1023              // Number of EEPROM initial cell
#define INIT_KEY 50                 // First launch key
#define DATE_DISPLAY_TIMEOUT 10000  // How long the date is displayed after pressing Button1
#define NIGHT_MODE_ENABLED 1        // Night mode enabled
#define NIGHT_START 23              // Begin of night mode
#define NIGHT_END 9                 // End of night mode


//---------- Include libraries
#include <microLED.h>
#include <GyverTimer.h>
#include <VirtualButton.h>
#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>


//---------- Initialize devices
microLED<NUMLEDS, STRIP_PIN, MLED_NO_CLOCK, LED_WS2811, ORDER_RGB, CLI_LOW> strip;
RTC_DS3231 rtc;
VButton btn1;
VButton btn2;


//---------- Timers
GTimer oneSecondTimer(MS, 1000);
GTimer blinkTimer(MS, 250);
GTimer timeoutTimer(MS);


//---------- Variables
DateTime now;
boolean blinkFlag = true;
byte mode = 0;
byte setupMode = 0;
byte secs, mins, hrs, month, day, newHrs, newMins, newSecs, newMonth, newDay;
byte current_bright = 200;
byte secColorIndex = 9, minColorIndex = 13, hourColorIndex = 4, dateColorIndex = 1;
boolean nightModeFlag;
word year, newYear;
uint32_t hrsColor, minColor, secColor, dateColor;

uint32_t ledColors[] = {0xFFFFFF, 0xC0C0C0, 0xFF0000, 0x800000,
                        0xFFFF00, 0x808000, 0x0033FF, 0xFF3000,
                        0x00FF00, 0x008000, 0x00FFFF, 0x008080, 
                        0xFF8000, 0x000080, 0xFF00FF, 0x800080};

//    mWhite =    0xFFFFFF,    // белый
//    mSilver =    0xC0C0C0,    // серебро
//    mGray =        0x808080,    // серый
//    mRed =        0xFF0000,    // красный
//    mMaroon =    0x800000,    // бордовый
//    mOrange =    0xFF3000,    // оранжевый
//    mYellow =    0xFF8000,    // жёлтый
//    mOlive =    0x808000,    // олива
//    mLime =        0x00FF00,    // лайм
//    mGreen =    0x008000,    // зелёный
//    mAqua =        0x00FFFF,    // аква
//    mTeal =        0x008080,    // цвет головы утки чирка
//    mBlue =        0x0000FF,    // голубой
//    mNavy =        0x000080,    // тёмно-синий
//    mMagenta =    0xFF00FF,    // розовый
//    mPurple =    0x800080,    // пурпурный


void setup() {
  Serial.begin(9600);
  
  //Pin modes
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);

  
  // Setting up RTC module and display time
  rtc.begin();
  if (RESET_CLOCK || rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();
  day = now.day();
  month = now.month();
  year = now.year();


  // EEPROM
  if (EEPROM.read(INIT_ADDR) != INIT_KEY) {   // First launch
    EEPROM.write(INIT_ADDR, INIT_KEY);
    EEPROM.put(0, secColorIndex);
    EEPROM.put(1, minColorIndex);
    EEPROM.put(2, hourColorIndex);
    EEPROM.put(3, dateColorIndex);
    EEPROM.put(5, current_bright);
    EEPROM.put(6, NIGHT_MODE_ENABLED);
  }
  EEPROM.get(0, secColorIndex);
  EEPROM.get(1, minColorIndex);
  EEPROM.get(2, hourColorIndex);
  EEPROM.get(3, dateColorIndex);
  EEPROM.get(5, current_bright);
//  EEPROM.get(6, NIGHT_MODE_ENABLED);


  // Initial colors setup
  secColor = ledColors[secColorIndex];
  minColor = ledColors[minColorIndex];
  hrsColor = ledColors[hourColorIndex];
  dateColor = ledColors[dateColorIndex];


  // Setting up LED strip
  strip.setBrightness(current_bright);
  strip.clear();

  // Timeout timer setup
  timeoutTimer.setTimeout(30000);
}


void loop(){
  buttonTick();
  timeTick();
  updateStrip();  
}


void timeTick(){
  if (oneSecondTimer.isReady()){
    secs++;
  
    if (secs > 59){
      now = rtc.now();
      secs = now.second();
      mins = now.minute();
      hrs = now.hour();
      day = now.day();
      month = now.month();
      year = now.year();

      checkNightMode();
    }
  }
  
}

void checkNightMode(){
  if ((hrs >= NIGHT_START && hrs <= 23) || (hrs >= 0 && hrs < NIGHT_END)) {
    nightModeFlag = true;
  }
  else {
    nightModeFlag = false;
  }
}


void buttonTick(){
  btn1.poll(!digitalRead(BUTTON_1_PIN));
  btn2.poll(!digitalRead(BUTTON_2_PIN));

  if (mode == 0){
    if (nightModeFlag){
      if (btn1.click() || btn2.click()) nightModeFlag = false;
    }
    
    if (btn1.click()) mode = 1;                                             // Switch to date showing mode if Button1 is clicked once

    if (btn2.click()) {                                                     // Switch the brightness if Button2 is clicked once
      if (current_bright >= 250) {
        current_bright = 50;
      }
      else {
        current_bright += 25;
      }
      strip.setBrightness(current_bright);
    }

    if (btn1.held()){                                                       // Switch to time setting mode
      mode = 2;
      setupMode = 0;
      now = rtc.now();
      newSecs = now.second();
      newMins = now.minute();
      newHrs = now.hour();
      timeoutTimer.start();
    }

    if (btn2.timeout(5000)) {                                               // Save the new brightness value after 5s if it was modified
      if (EEPROM.read(5) != current_bright) EEPROM.put(5, current_bright);
    }

    if (btn2.held()) {                                                      // Switch to color selection mode
      mode = 4;
      setupMode = 5;
    }
  }

  if (mode == 1){                                                           // Return the mode to time display after 8s of showing date
    if (btn1.timeout(DATE_DISPLAY_TIMEOUT)) mode = 0;

    if (btn1.held()){                                                       // Switch to date setting mode
      mode = 3;
      setupMode = 2;
      now = rtc.now();
      newMonth = now.month();
      newDay = now.day();
      newYear = now.year();
      timeoutTimer.start();
    }
  }

  
  if (mode == 2){                                                           // Time setting mode
    if (btn1.click()) {
      timeoutTimer.start();
      setupMode++;
      if (setupMode > 1) setupMode = 0;
    }

    if (btn2.click()) {
      timeoutTimer.start();
      if (setupMode == 0){
        newHrs ++;
        if (newHrs > 23) newHrs = 0;
      }

      if (setupMode == 1){
        newMins ++;
        if (newMins > 59) newMins = 0;
      }
    }

    if (btn1.held()){
      now = rtc.now();
      uint16_t currentYear = now.year();
      uint8_t currentMonth = now.month();
      uint8_t currentDay = now.day();
      rtc.adjust(DateTime(currentYear, currentMonth, currentDay, newHrs, newMins, 0));
      now = rtc.now();
      secs = now.second();
      mins = now.minute();
      hrs = now.hour();
      mode = 0;
      setupMode = 0;
      timeoutTimer.stop();
    }
  }


  if (mode == 3){                                                           // Date setting mode
    if (btn1.click()) {
      timeoutTimer.start();
      setupMode++;
      if (setupMode > 4) setupMode = 2;
    }

    if (btn2.click()) {
      timeoutTimer.start();
      if (setupMode == 2){
        newDay ++;
        if (newDay > 31) newDay = 1;
      }

      if (setupMode == 3){
        newMonth ++;
        if (newMonth > 12) newMonth = 1;
      }

      if (setupMode == 4){
        newYear ++;
        if (newYear > 2050) newYear = 2020;
      }
    }

    if (btn1.held()){
      now = rtc.now();
      uint8_t currentHrs = now.hour();
      uint8_t currentMins = now.minute();
      uint8_t currentSecs = now.second();
      rtc.adjust(DateTime(newYear, newMonth, newDay, currentHrs, currentMins, currentSecs));
      mode = 0;
      setupMode = 0;
      timeoutTimer.stop();
    }
  }


  if (mode == 4){                                                           // Date setting mode
    if (btn1.click()) {
      timeoutTimer.start();
      setupMode++;
      if (setupMode > 8) setupMode = 5;
    }

    if (btn2.click()) {
      timeoutTimer.start();
      if (setupMode == 5){
        hourColorIndex ++;
        if (hourColorIndex > 15) hourColorIndex = 0;
        hrsColor = ledColors[hourColorIndex];
      }

      if (setupMode == 6){
        minColorIndex ++;
        if (minColorIndex > 15) minColorIndex = 0;
        minColor = ledColors[minColorIndex];
      }

      if (setupMode == 7){
        secColorIndex ++;
        if (secColorIndex > 15) secColorIndex = 0;
        secColor = ledColors[secColorIndex];
      }

      if (setupMode == 8){
        dateColorIndex ++;
        if (dateColorIndex > 15) dateColorIndex = 0;
        dateColor = ledColors[dateColorIndex];
      }
    }

    if (btn1.held()){
      if (EEPROM.read(0) != secColorIndex) EEPROM.put(0, secColorIndex);
      if (EEPROM.read(1) != minColorIndex) EEPROM.put(1, minColorIndex);
      if (EEPROM.read(2) != hourColorIndex) EEPROM.put(2, hourColorIndex);
      if (EEPROM.read(3) != dateColorIndex) EEPROM.put(3, dateColorIndex);
      mode = 0;
      setupMode = 0;
      timeoutTimer.stop();
    }
  }

    
  if (timeoutTimer.isReady()) {
      mode = 0;
      setupMode = 0;    
    }
}


void updateStrip(){
  
  strip.clear();
  
  if (nightModeFlag){
    strip.show();
    return;
  }
  
  if(blinkTimer.isReady()){
    blinkFlag = !blinkFlag;
  }
     
  if (mode == 0) {  
    fillStrip(secs, 0, secColor);
    fillStrip(mins, 7, minColor);
    fillStrip(hrs, 14, hrsColor);
  }

  else if (mode == 1) {
    fillStrip((year-2000), 0, dateColor);
    fillStrip(month, 7, dateColor);
    fillStrip(day, 14, dateColor);
  }


  else if (mode == 2) {
    
    if (setupMode == 0){
      if (blinkFlag) fillStrip(newHrs, 14, hrsColor);
    }
    else fillStrip(newHrs, 14, hrsColor);
    
    if (setupMode == 1){
      if (blinkFlag) fillStrip(newMins, 7, minColor);
    }
    else fillStrip(newMins, 7, minColor);

    fillStrip(secs, 0, secColor);
  }


  else if (mode == 3) {
    
    if (setupMode == 2){
      if (blinkFlag) fillStrip(newDay, 14, dateColor);
    }
    else fillStrip(newDay, 14, dateColor);
    
    if (setupMode == 3){
      if (blinkFlag) fillStrip(newMonth, 7, dateColor);
    }
    else fillStrip(newMonth, 7, dateColor);

    if (setupMode == 4){
      if (blinkFlag) fillStrip((newYear-2000), 0, dateColor);
    }
    else fillStrip((newYear-2000), 0, dateColor);
  }


  else if (mode == 4) {
    
    if (setupMode < 8) {
    
      if (setupMode == 5){
        if (blinkFlag) fillStrip(37, 14, hrsColor);
      }
      else fillStrip(37, 14, hrsColor);
      
      if (setupMode == 6){
        if (blinkFlag) fillStrip(77, 7, minColor);
      }
      else fillStrip(77, 7, minColor);

      if (setupMode == 7){
        if (blinkFlag) fillStrip(77, 0, secColor);
      }
      else fillStrip(77, 0, secColor);
    }

    else {
      if (blinkFlag) {
        fillStrip(37, 14, dateColor);
        fillStrip(77, 7, dateColor);
        fillStrip(77, 0, dateColor);
      }
      
    }
  }
    
    strip.show();
}


void fillStrip(int Number, char startLed, uint32_t ledColor) {
  constrain(Number, 0, 99);
  boolean tensBin[] = {0,0,0,0};
  boolean onesBin[] = {0,0,0,0};
  convertDecToBin(Number % 10, onesBin);
  convertDecToBin(Number / 10, tensBin);
  
  
  for (int i = 3; i >= 0; i--) {
    if (onesBin[i]==1) {
      strip.set((i+startLed), mHEX(ledColor));
      }
    }
    
  
  for (int i = 3; i >= 1; i--) {
    if (tensBin[i]==1) {
      strip.set((7-i+startLed), mHEX(ledColor));
      }
    }
}


void convertDecToBin(int Dec, boolean Bin[]) {
  for(int i = 4 ; i >= 0 ; i--) {
    if(pow(2, i)<=Dec) {
      Dec = Dec - pow(2, i);
      Bin[4-(i+1)] = 1;
    } else {
    }
  }
}
