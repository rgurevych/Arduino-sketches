//Wall clock by Rostyslav Gurevych

//---------- Define pins and settings

#define STRIP_PIN 5                 // LED strip control pin
#define BUTTON_1_PIN 8              // button 1
#define BUTTON_2_PIN 9              // button 1
#define CLK 12                      // display
#define DIO 11                      // display
#define IR_PIN 10                   // IR receiver
#define LED_PIN 13

#define COLOR_DEBTH 3
#define RESET_CLOCK 0               //Should the RTC be reset?
#define NUMLEDS 10                  //Number of LEDs in the strip
#define MODES_NUMBER 4              //Number of modes
#define EFFECTS_NUMBER 2            //Number of available effects


//---------- Include libraries
#include <microLED.h>
#include <GyverTimer.h>
#include <GyverTM1637.h>
#include <GyverButton.h>
#include <Wire.h>
#include <RTClib.h>
#include <IRremote.h>


//---------- Initialize devices
microLED<NUMLEDS, STRIP_PIN, -1, LED_WS2812, ORDER_GRB, CLI_LOW> strip;
GyverTM1637 disp(CLK, DIO);           // LED display
RTC_DS3231 rtc;                       // RTC module
GButton button1(BUTTON_1_PIN);        // Button1
GButton button2(BUTTON_2_PIN);        // Button1


//---------- Timers
GTimer_ms oneSecondTimer(1000);
GTimer_ms halfsTimer(500);
GTimer_ms timeoutTimer(20000);


//---------- Variables
DateTime now;
boolean dotFlag, lcdFlag = true, stripFlag = true;
byte mode = 0;
byte effect = 1;
byte timeSetMode = 0;
byte counter = 0;
int8_t hrs = 21, mins = 55, secs;
byte DISPLAY_BRIGHTNESS = 5;


void setup() {
  Serial.begin(9600);
  
  //Pin modes
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  // Setting up display
  disp.clear();
  disp.brightness(DISPLAY_BRIGHTNESS);

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

  // Initialize IR reciever
  IrReceiver.begin(IR_PIN);
}

void loop(){
  checkIR(); 
  checkButtons();
  clockTick();
  settings();
}


void checkIR(){
    if (IrReceiver.decode()) {
    double code = IrReceiver.decodedIRData.decodedRawData;

    if (code == 0xF609FF00){
      DISPLAY_BRIGHTNESS ++;
      if (DISPLAY_BRIGHTNESS > 7) DISPLAY_BRIGHTNESS = 7;
      disp.brightness(DISPLAY_BRIGHTNESS);
    } 

    else if (code == 0xE21DFF00){
      DISPLAY_BRIGHTNESS --;
      if (DISPLAY_BRIGHTNESS < 1) DISPLAY_BRIGHTNESS = 1;
      disp.brightness(DISPLAY_BRIGHTNESS);
    }  
    
    else if (code == 0xF20DFF00){
      mode = 0;
    }

    else if (code == 0xE01FFF00){
      mode = 10;
    }

    else if (code == 0xB24DFF00){
      effect++;
      if(effect > EFFECTS_NUMBER - 1) effect = 0;
    }

    else {
      Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    }
    
    IrReceiver.resume();
  }
}


void updateStrip(){
  
  if(effect == 0 || mode == 10){
    strip.clear();
  }
  
  else{
    if(effect == 1){
      strip.fill(mRed);
      for (int i = 0; i < NUMLEDS; i++) {
        if ((i + 1) % 5 == 0) {
          strip.set(i, mRGB(150, 0, 0));
        }
        else {
          strip.set(i, mRGB(30, 0, 0));
        }
      }
  
      strip.set(secs%10, mSilver);
      counter ++;
      if (counter > 9) counter = 0;
    }
  }

  strip.show();
}


void checkButtons(){
  button1.tick();
  button2.tick();

  if(button1.isClick()){
    timeoutTimer.reset();
    effect++;
    if(effect > EFFECTS_NUMBER - 1) effect = 0;
  }
  
  if(button1.isHolded()){
    timeoutTimer.reset();
    mode++;
    if(mode > MODES_NUMBER - 1) mode = 0;
  }
}


void clockTick() {
  if (halfsTimer.isReady()) {
    dotFlag = !dotFlag;
    
    if (dotFlag) {          // recalculate time every second
      secs++;
      if (secs > 59) {      // read time every minute
        now = rtc.now();
        secs = now.second();
        mins = now.minute();
        hrs = now.hour();
      }
      
      updateStrip();
    }
    
    if (mode == 0){
      disp.point(dotFlag);                 // switch the dots
      disp.displayClock(hrs, mins);        // needed to avoid display lags
    }

    else if (mode == 1){
      disp.point(true);

      if (dotFlag){
        disp.display(0, hrs/10);
        disp.display(1, hrs%10);
        disp.displayByte(2, _empty);
        disp.displayByte(3, _empty);
      }
      else{
        disp.displayByte(_empty, _empty, _empty, _empty);
      }
    }

    else if (mode == 2){
      disp.point(true);

      if (dotFlag){
        disp.displayInt(mins);
      }
      else{
        disp.displayByte(_empty, _empty, _empty, _empty);
      }
    }

    else if (mode == 3){
      disp.point(!dotFlag);
    
      if (dotFlag){
        disp.displayInt(secs);
      }
      else{
        disp.displayByte(_empty, _empty, _empty, _empty);
      }
    }
  
    else if (mode == 10){
      disp.point(false);
      disp.clear();
    }
  }
}


void settings(){
  if (mode == 1) {
    if (timeoutTimer.isReady()){
      mode = 0; // return to mode 0 if timeout
      disp.clear();
      return;
    }
    
    if (button2.isClick()){
      timeoutTimer.reset();
      hrs++;
      if (hrs > 23) hrs = 0;
      rtc.adjust(DateTime(2021, 1, 1, hrs, mins, secs));
    }
  }

  else if (mode == 2){    
    if (timeoutTimer.isReady()){
      mode = 0; // return to mode 0 if timeout
      disp.clear();
      return;
    }
    
    if (button2.isClick()){
      timeoutTimer.reset();
      mins++;
      if (mins > 59) mins = 0;
      rtc.adjust(DateTime(2021, 1, 1, hrs, mins, secs));
    }
  }

  else if (mode == 3){
      if (timeoutTimer.isReady()){
      mode = 0; // return to mode 0 if timeout
      disp.clear();
      return;
    }
    
    if (button2.isClick()){
      timeoutTimer.reset();
      secs = 0;
      rtc.adjust(DateTime(2021, 1, 1, hrs, mins, secs));
    }
  }
}
