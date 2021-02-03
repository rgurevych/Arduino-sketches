//Wall clock by Rostyslav Gurevych

//---------- Define pins and settings

#define STRIP_PIN 5                 // LED strip control pin
#define BUTTON_PIN 8                // Debugging button
#define CLK 12                      // display
#define DIO 11                      // display
#define LED_PIN 13

#define COLOR_DEBTH 3
#define RESET_CLOCK 0               //Should the RTC be reset?
#define NUMLEDS 10                  //Number of LEDs in the strip


//---------- Include libraries
#include <microLED.h>
#include <GyverTimer.h>
#include <GyverTM1637.h>
#include <GyverButton.h>
#include <Wire.h>
#include <RTClib.h>


//---------- Initialize devices
microLED<NUMLEDS, STRIP_PIN, -1, LED_WS2812, ORDER_GRB, CLI_LOW> strip;
GyverTM1637 disp(CLK, DIO);   // LED display
RTC_DS3231 rtc;               // RTC module
GButton button(BUTTON_PIN);   // Button1


//---------- Timers
GTimer_ms oneSecondTimer(1000);
GTimer_ms halfsTimer(500);
GTimer_ms timeoutTimer(20000);


//---------- Variables
DateTime now;
boolean dotFlag, ledFlag = false;
byte mode = 0; 
byte timeSetMode = 0;
byte counter = 0;
int8_t hrs = 21, mins = 55, secs;


void setup() {
  Serial.begin(9600);
  
  //Pin modes
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  // Setting up display
  disp.clear();
  disp.brightness(5);

  // Setting up RTC module and display time
  rtc.begin();
  if (RESET_CLOCK || rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();

  // Setting up LED strip
  strip.setBrightness(140);
  strip.clear();
}

void loop() {
  updateStrip();
  clockTick();
  button.tick();
  modeSwitch();
  settings();
}

void updateStrip(){
  if(oneSecondTimer.isReady()){
    strip.fill(mRed);
    for (int i = 0; i < NUMLEDS; i++) {
      if ((i + 1) % 5 == 0) {
        strip.set(i, mRGB(150, 0, 0));
      }
      else {
        strip.set(i, mRGB(30, 0, 0));
      }
    }

    strip.set(counter, mSilver);
    counter ++;
    if (counter > 9) counter = 0;
  
    strip.show();
  }
}

void modeSwitch(){
  if (mode == 0){
    if(button.isHolded()){
      mode = 1;
    }
  }
}

void clockTick() {
  if (halfsTimer.isReady()) {
    dotFlag = !dotFlag;
    if (mode == 0){
      disp.point(dotFlag);                 // switch the dots
      disp.displayClock(hrs, mins);        // needed to avoid display lags
    }
  
    if (dotFlag) {          // recalculate time every second
      secs++;
      if (secs > 59) {      // read time every minute
        now = rtc.now();
        secs = now.second();
        mins = now.minute();
        hrs = now.hour();
      }
    }
  }
}


void settings(){
  if (mode == 1) {
    if (timeoutTimer.isReady()){
      mode = 0;   // return to mode 0 if timeout
      disp.clear();
      rtc.adjust(DateTime(2021, 1, 1, hrs, mins, secs));
    }

//    if (timeSetMode == 2 && button.isHolded()){
//      mode = 0;   // return to mode 0 if button is double clicked
//      timeSetMode = 0;
//      disp.clear();
//      rtc.adjust(DateTime(2021, 1, 1, hrs, mins, 0));
//    }
    
    disp.point(false);

    if (timeSetMode == 0){
      if (dotFlag){
        disp.displayClock(hrs, 0);
      }
      else{
        disp.clear();
      }
      
      if (button.isClick()){
        timeoutTimer.reset();
        hrs++;
        if (hrs > 23) hrs = 0;
      }

      else if (button.isHolded()){
        timeoutTimer.reset();
        timeSetMode ++;
        rtc.adjust(DateTime(2021, 1, 1, hrs, mins, secs));
      }
    }
    
    else if (timeSetMode == 1){
      if (dotFlag){
        disp.displayClock(0, mins);
      }
      else{
        disp.clear();
      }
      
      if (button.isClick()){
        timeoutTimer.reset();
        mins++;
        if (mins > 59) mins = 0;
      }

      else if (button.isHolded()){
        timeoutTimer.reset();
        timeSetMode ++;
        rtc.adjust(DateTime(2021, 1, 1, hrs, mins, secs));
      }
    }

    else if (timeSetMode == 2){
      if (dotFlag){
        disp.displayInt(secs);
      }
      else{
        disp.clear();
      }
      
      if (button.isClick()){
        timeSetMode = 0;
        mode = 0;
        rtc.adjust(DateTime(2021, 1, 1, hrs, mins, 0));
      }
    }
  }
}
