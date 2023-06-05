//Wall clock by Rostyslav Gurevych

//---------- Define pins and settings

#define STRIP_PIN 5                 // LED strip control pin
#define BUTTON_1_PIN 8              // button 1
#define BUTTON_2_PIN 9              // button 1
#define CLK 12                      // display
#define DIO 11                      // display
#define LED_PIN 13

#define COLOR_DEBTH 3
#define RESET_CLOCK 0               //Should the RTC be reset?
#define NUMLEDS 20                  //Number of LEDs in the strip
#define MODES_NUMBER 4              //Number of modes
#define EFFECTS_NUMBER 4            //Number of available effects


//---------- Include libraries
#include <microLED.h>
#include <GyverTimer.h>
#include <VirtualButton.h>
#include <Wire.h>
#include <RTClib.h>


//---------- Initialize devices
microLED<NUMLEDS, STRIP_PIN, MLED_NO_CLOCK, LED_WS2811, ORDER_RGB, CLI_LOW> strip;
RTC_DS3231 rtc;                       // RTC module
VButton btn1;
VButton btn2;
//GButton button1(BUTTON_1_PIN);        // Button1
//GButton button2(BUTTON_2_PIN);        // Button1


//---------- Timers
GTimer oneSecondTimer(MS, 1000);
//GTimer_ms halfsTimer(500);
//GTimer_ms timeoutTimer(20000);


//---------- Variables
//DateTime now;
//boolean dotFlag, lcdFlag = true, stripFlag = true;
//byte mode = 0;
//byte effect = 1;
//byte timeSetMode = 0;
byte secs, mins, hrs;
uint32_t hrsColor, minColor, secColor;

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
  pinMode(LED_PIN, OUTPUT);
  
  // Setting up RTC module and display time
  rtc.begin();
  if (RESET_CLOCK || rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  DateTime now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();

  // Setting up LED strip
  strip.setBrightness(200);
  strip.clear();
}


void loop(){
  btn1.poll(!digitalRead(BUTTON_1_PIN));
  btn2.poll(!digitalRead(BUTTON_2_PIN));
  
  if (oneSecondTimer.isReady()){
  updateStrip();
  }
}


void updateStrip(){
  
    strip.clear();
    secs++;

    secColor = ledColors[9];
    minColor = ledColors[13];
    hrsColor = ledColors[4];

    if (btn1.press()){
      secs = 0;
      Serial.println("Button 1 pressed");
    }

    if (btn2.press()){
      mins = 0;
      Serial.println("Button 2 pressed");
    }
  
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
    
    fillStrip(secs, 0, secColor);
    fillStrip(mins, 7, minColor);
    fillStrip(hrs, 14, hrsColor);
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


//void clockTick() {
//  if (halfsTimer.isReady()) {
//    dotFlag = !dotFlag;
//    
//    if (dotFlag) {          // recalculate time every second
//      secs++;
//      if (secs > 59) {      // read time every minute
//        now = rtc.now();
//        secs = now.second();
//        mins = now.minute();
//        hrs = now.hour();
//      }
//      
//      updateStrip();
//    }
//    
//    if (mode == 0){
//      disp.point(dotFlag);                 // switch the dots
//      disp.displayClock(hrs, mins);        // needed to avoid display lags
//    }
//
//    else if (mode == 1){
//      disp.point(true);
//
//      if (dotFlag){
//        disp.display(0, hrs/10);
//        disp.display(1, hrs%10);
//        disp.displayByte(2, _empty);
//        disp.displayByte(3, _empty);
//      }
//      else{
//        disp.displayByte(_empty, _empty, _empty, _empty);
//      }
//    }
//
//    else if (mode == 2){
//      disp.point(true);
//
//      if (dotFlag){
//        disp.displayInt(mins);
//      }
//      else{
//        disp.displayByte(_empty, _empty, _empty, _empty);
//      }
//    }
//
//    else if (mode == 3){
//      disp.point(!dotFlag);
//    
//      if (dotFlag){
//        disp.displayInt(secs);
//      }
//      else{
//        disp.displayByte(_empty, _empty, _empty, _empty);
//      }
//    }
//  
//    else if (mode == 10){
//      disp.point(false);
//      disp.displayByte(_empty, _empty, _empty, _empty);
//    }
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
