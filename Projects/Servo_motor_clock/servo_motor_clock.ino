#include <GyverTimer.h>         //Library for working with timer
#include <SoftwareSerial.h>     //Library for working with serial port
#include <GyverButton.h>        //Library for working with button
#include <Wire.h>               //Library for working with LED display
#include <LiquidCrystal_I2C.h>  //Library for working with LED display
#include <RTClib.h>             //Library for working with RTC clock module
#include <Servo.h>              //Library for working with Servo motors


// Pins:
#define BUTTON1_PIN 7       //Button 1 pin
#define BUTTON2_PIN 8       //Button 1 pin
#define BACKLIGHT 5         //LCD backlight pin
#define SERVO_HOURS_PIN 9   //Hours servo pin
#define SERVO_MINS_PIN 10   //Minutes servo pin
#define SERVO_SECS_PIN 11   //Seconds servo pin


// Timer durations
#define MEASURE_TIMEOUT 15000             //Interval between measure occurs
#define PRINT_TIMEOUT 30000               //Interval between serial printout occurs
#define WARMING_UP_TIMEOUT 175000         //Duration of warming up period
#define BLINK_TIMEOUT 750                //LED blinking interval
#define RESET_MODE_TIMEOUT 10000          //Timeout before the mode is reset to default screen


// Settings
#define RESET_CLOCK 0                     //Should the clock be reset on start
#define DEBUG 1                           //Debug mode, in which the data is printed to serial port


// Timers:
GTimer printTimer(MS, PRINT_TIMEOUT);
GTimer clockTimer(MS, 1000);
GTimer resetModeTimer(MS);


// Buttons
GButton button1(BUTTON1_PIN);
GButton button2(BUTTON2_PIN);


//Liquid Crystal LCD:
LiquidCrystal_I2C lcd(0x27, 16, 2);


//Real Time Clock:
RTC_DS3231 rtc;
DateTime now;


//Servo motors;
Servo servoHours;
Servo servoMins;
Servo servoSecs;


// Initial variables:
int8_t hrs, mins, secs, days, months;
int years;
byte motorHour = 0; 
byte motorMin = 0;
byte motorSec = 0;

byte LCD_BRIGHTNESS = 5;

byte mode = 0;

// Flags
boolean lcdBacklight = true;
boolean changeModeFlag = false;
boolean setTimeFlag = false;
boolean stopHands = false;
boolean setHours = true;
boolean setMins = false;
boolean setSecs = false;
boolean setDays = false;
boolean setMonths = false;
boolean setYears = false;


void setup() {
  // Serial
  if (DEBUG){
  Serial.begin(9600);
  }

  //Pin modes
  pinMode(BACKLIGHT, OUTPUT);

  // RTC module
  rtc.begin();
  if (RESET_CLOCK || rtc.lostPower()){
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //LCD
  analogWrite(BACKLIGHT, LCD_BRIGHTNESS);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  //Servo motors
  servoHours.attach(SERVO_HOURS_PIN);
  servoMins.attach(SERVO_MINS_PIN);
  servoSecs.attach(SERVO_SECS_PIN);


  
}

void loop(){

  button1.tick();
  button2.tick();
  checkButtons();
  displayScreen();
}


void printCurrentValues(){  
  if (printTimer.isReady() && DEBUG){
    now = rtc.now();
    secs = now.second();
    mins = now.minute();
    hrs = now.hour();
    
    
  }
}


void printMainScreen(){
  if (clockTimer.isReady()) {
    // get time
    now = rtc.now();
    secs = now.second();
    mins = now.minute();
    hrs = now.hour();
    days = now.day();
    months = now.month();
    years = now.year();

    // print time to LCD
    lcd.setCursor(4, 0);
    if (hrs < 10) lcd.print(F("0"));
    lcd.print(hrs);
    lcd.print(F(":"));

    if (mins < 10) lcd.print(F("0"));
    lcd.print(mins);
    lcd.print(F(":"));

    if (secs < 10) lcd.print("0");
    lcd.print(secs);

    lcd.setCursor(3, 1);
    if (days < 10) lcd.print(F("0"));
    lcd.print(days);
    lcd.print(F("."));

    if (months < 10) lcd.print(F("0"));
    lcd.print(months);
    lcd.print(F("."));

    lcd.print(years);
  }

  if (!stopHands) {
  turnMotors();
  }
}


// Turn the motors if necessary
void turnMotors(){
  byte currentHour, currentMin, currentSec;
  if (hrs > 12) currentHour = (hrs - 12) * 15; else currentHour = hrs * 15;
  currentMin = mins * 3;
  currentSec = secs * 3;
  
  if (currentHour != motorHour){
    motorHour = currentHour;
    servoHours.write(180 - currentHour);
    //servoHours.write(1);
  }
  
  if (currentMin != motorMin){
    motorMin = currentMin;
    servoMins.write(180 - currentMin);
    //servoMins.write(1);
  }

  if (currentSec != motorSec){
    motorSec = currentSec;
    servoSecs.write(180 - currentSec);
    //servoSecs.write(0);
  }
 
}


// Check each button state
void checkButtons(){
  if (!setTimeFlag && button1.isClick()){
      switchMode();
      stopHands = !stopHands;
  }

  if (!setTimeFlag && button1.isHolded()){
    setTimeFlag = true;
    switchMode();
  }

  if (button2.isClick()){
    switchLcdBacklight();
  }

  if (button2.isHolded()){
    updateLcdBrightness();
  }
}


// Switch mode
void switchMode(){
  if (setTimeFlag){
    mode = 1;
  }
  else{
    mode = 0;
  }
  lcd.clear();
  changeModeFlag = true;
  resetModeTimer.setTimeout(RESET_MODE_TIMEOUT);
}


// Switch LCD backlight on and off
void switchLcdBacklight(){
  lcdBacklight = !lcdBacklight;
  lcd.setBacklight(lcdBacklight);
}


// Change the LCD backlight brightness
void updateLcdBrightness(){
  if (LCD_BRIGHTNESS >= 55) {
    LCD_BRIGHTNESS = 5;
  }
  else {
    LCD_BRIGHTNESS += 10;
  }

  if (!lcdBacklight) {
    switchLcdBacklight();
  }
  
  analogWrite(BACKLIGHT, LCD_BRIGHTNESS);
}


// Change info on the screen depending on current mode
void displayScreen(){

  if (resetModeTimer.isReady()){
    mode = 0;
    lcd.noBlink();
    lcd.clear();
    setTimeFlag = false;
    setHours = true;
    setMins = false;
    setSecs = false;
    setDays = false;
    setMonths = false;
    setYears = false;
  }
  
  switch(mode){
    case 0:
      lcd.noBlink();
      printMainScreen();
      resetModeTimer.stop();
      changeModeFlag = false;
      break;

    case 1:
      if (changeModeFlag) {
        printMainScreen();
        setTime();
      }
      break;
  }
}


void setTime(){
  if (setTimeFlag && button1.isHolded()){
    switchTimeSetMode();
  }

  if (setTimeFlag && button1.isSingle()){
    now = rtc.now();
    secs = now.second();
    mins = now.minute();
    hrs = now.hour();
    years = now.year();
    months = now.month();
    days = now.day();

    if (setHours){
      if (hrs == 23) {
        hrs = 0;
      }
      else {
        hrs ++;
      }
    }

     else if (setMins){
      if (mins == 59) {
        mins = 0;
      }
      else {
        mins ++;
      }
    }

     else if (setSecs){
      secs = 0;
    }

    else if (setDays){
      if (days == 31) {
        days = 0;
      }
      else {
        days ++;
      }
    }

    else if (setMonths){
      if (months == 12) {
        months = 0;
      }
      else {
        months ++;
      }
    }

    else if (setYears){
      if (years == 2030) {
        years = 2020;
      }
      else {
        years ++;
      }
    }

    rtc.adjust(DateTime(years, months, days, hrs, mins, secs));
    resetModeTimer.setTimeout(RESET_MODE_TIMEOUT * 2);
  }
  
  if (setHours){
    lcd.setCursor(5, 0);
    lcd.blink();
  }

  else if (setMins){
    lcd.setCursor(8, 0);
    lcd.blink();
  }

  else if (setSecs){
    lcd.setCursor(11, 0);
    lcd.blink();
  }

  else if (setDays){
    lcd.setCursor(4, 1);
    lcd.blink();
  }

  else if (setMonths){
    lcd.setCursor(7, 1);
    lcd.blink();
  }

  else if (setYears){
    lcd.setCursor(12, 1);
    lcd.blink();
  }
}


void switchTimeSetMode(){
  if (setHours){
    setHours = false;
    setMins = true;
    setSecs = false;
    setDays = false;
    setMonths = false;
    setYears = false;
  }
  else if (setMins){
    setHours = false;
    setMins = false;
    setSecs = true;
    setDays = false;
    setMonths = false;
    setYears = false;  
  }
  else if (setSecs){
    setHours = false;
    setMins = false;
    setSecs = false;
    setDays = true;
    setMonths = false;
    setYears = false;  
  }
  else if (setDays){
    setHours = false;
    setMins = false;
    setSecs = false;
    setDays = false;
    setMonths = true;
    setYears = false;  
  }
  else if (setMonths){
    setHours = false;
    setMins = false;
    setSecs = false;
    setDays = false;
    setMonths = false;
    setYears = true;  
  }
  else {
    setHours = true;
    setMins = false;
    setSecs = false;
    setDays = false;
    setMonths = false;
    setYears = false;
  }
  
  resetModeTimer.setTimeout(RESET_MODE_TIMEOUT * 2);
}
