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


//---------- Include libraries
#include <microLED.h>
#include <GyverTimer.h>
#include <VirtualButton.h>
#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>


//---------- Initialize devices
microLED<NUMLEDS, STRIP_PIN, MLED_NO_CLOCK, LED_WS2811, ORDER_RGB, CLI_LOW> strip;
RTC_DS3231 rtc;                       // RTC module
VButton btn1;
VButton btn2;


//---------- Timers
GTimer oneSecondTimer(MS, 1000);
GTimer displayTimer(MS, 250);
//GTimer_ms timeoutTimer(20000);


//---------- Variables
//DateTime now;
//boolean dotFlag, lcdFlag = true, stripFlag = true;
byte mode = 0;
//byte effect = 1;
byte setupMode = 0;
byte secs, mins, hrs, month, day, new_hour, new_min, new_second, new_month, new_day, new_year;
byte current_bright = 200;
byte secColorIndex = 9, minColorIndex = 13, hourColorIndex = 4, dateColorIndex = 1;
boolean NIGHT_MODE_ENABLED = 0;
word year;
uint32_t hrsColor, minColor, secColor, dateColor;

uint32_t ledColors[] = {0xFFFFFF, 0xC0C0C0, 0x808080, 0xFF0000, 
                        0x800000, 0xFF3000, 0xFF8000, 0x808000, 
                        0x00FF00, 0x008000, 0x00FFFF, 0x008080, 
                        0x0000FF, 0x000080, 0xFF00FF, 0x800080};

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
  DateTime now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();


  // EEPROM
  if (EEPROM.read(INIT_ADDR) != INIT_KEY) {   // первый запуск
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
  EEPROM.get(6, NIGHT_MODE_ENABLED);


  // Initial colors setup
  secColor = ledColors[secColorIndex];
  minColor = ledColors[minColorIndex];
  hrsColor = ledColors[hourColorIndex];
  dateColor = ledColors[dateColorIndex];


  // Setting up LED strip
  strip.setBrightness(current_bright);
  strip.clear();
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
      DateTime now = rtc.now();
      secs = now.second();
      mins = now.minute();
      hrs = now.hour();
    }

    Serial.print("Hour: ");
    Serial.print(hrs);
    Serial.print("     Minute: ");
    Serial.print(mins);
    Serial.print("     Second: ");
    Serial.print(secs);
    Serial.println();
  }
  
}

void buttonTick(){
  btn1.poll(!digitalRead(BUTTON_1_PIN));
  btn2.poll(!digitalRead(BUTTON_2_PIN));

  if (mode == 0){
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

    if (btn2.timeout(5000)) {                                               // Save the new brightness value after 5s if it was modified
      if (EEPROM.read(5) != current_bright) EEPROM.put(5, current_bright);
    }
      
  }

  if (mode == 1){                                                           // Return the mode to time display after 8s of showing date
    if (btn1.timeout(8000)) mode = 0;
  }

  
}


void updateStrip(){
     
    if (mode == 0) {  
      strip.clear();
      fillStrip(secs, 0, secColor);
      fillStrip(mins, 7, minColor);
      fillStrip(hrs, 14, hrsColor);
    }

    else if (mode == 1) {
      DateTime now = rtc.now();
      day = now.day();
      month = now.month();
      year = now.year();
      strip.clear();
      fillStrip((year-2000), 0, dateColor);
      fillStrip(month, 7, dateColor);
      fillStrip(day, 14, dateColor);
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

//void checkButtons(){
//  button1.tick();
//  button2.tick();
//
//  if(button1.isClick()){
//    timeoutTimer.reset();
//    effect++;
//    if(effect > EFFECTS_NUMBER - 1) effect = 0;
//  }
//  
//  if(button1.isHolded()){
//    timeoutTimer.reset();
//    mode++;
//    if(mode > MODES_NUMBER - 1) mode = 0;
//  }
//}




//
//void settings(){
//  if (mode == 1) {
//    if (timeoutTimer.isReady()){
//      mode = 0; // return to mode 0 if timeout
//      disp.clear();
//      return;
//    }
//    
//    if (button2.isClick()){
//      timeoutTimer.reset();
//      hrs++;
//      if (hrs > 23) hrs = 0;
//      rtc.adjust(DateTime(2021, 1, 1, hrs, mins, secs));
//    }
//  }
//
//  else if (mode == 2){    
//    if (timeoutTimer.isReady()){
//      mode = 0; // return to mode 0 if timeout
//      disp.clear();
//      return;
//    }
//    
//    if (button2.isClick()){
//      timeoutTimer.reset();
//      mins++;
//      if (mins > 59) mins = 0;
//      rtc.adjust(DateTime(2021, 1, 1, hrs, mins, secs));
//    }
//  }
//
//  else if (mode == 3){
//      if (timeoutTimer.isReady()){
//      mode = 0; // return to mode 0 if timeout
//      disp.clear();
//      return;
//    }
//    
//    if (button2.isClick()){
//      timeoutTimer.reset();
//      secs = 0;
//      rtc.adjust(DateTime(2021, 1, 1, hrs, mins, secs));
//    }
//  }
//}
