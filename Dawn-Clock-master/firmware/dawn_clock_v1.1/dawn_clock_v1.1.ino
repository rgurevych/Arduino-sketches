/*
   Клик в режиме часов -> установка будильника
   Клик в режиме установки будильника -> режим часов
   Удержание в режиме часов -> вкл/выкл будильник
   Удержание в режиме установки будильника -> установка времени
   Клик/удержание в режиме установки времени -> режим часов

   Режим часов: двоеточие моргает раз в секунду
   Установка будильника: цифры вместе с двоеточием моргают
   Установка часов: двоеточие горит, цифры моргают
*/

// *************************** INCLUDES  ***************************
#include <GyverEncoder.h>
#include <GyverTM1637.h>
#include <CyberLib.h>
#include <GyverButton.h>
#include <EEPROM.h>
#include <Wire.h>
#include <RTClib.h>
#include <GyverTimer.h>

// *************************** SETTINGS ***************************
#define DAWN_TIME 10      // Dawn duration
#define ALARM_TIMEOUT 60  // Timeout for switching off the alarm
#define ALARM_BLINK 0     // 1 - blink with lamp, 0 - no blink
#define BUZZ_FREQ 800     // Buzzer frequency (Hz)
#define DAWN_MIN 60       // Minimum lamp brightness (0 - 255)
#define DAWN_MAX 200      // Maximum lamp brightness (0 - 255)
#define DISPLAY_BRIGHT 1  // Display brightness
#define LED_BRIGHT 50     // LED brightness (0 - 255)
#define ENCODER_TYPE 1    // тип энкодера (0 или 1). Типы энкодеров расписаны на странице проекта

// ************ PINS ************
#define CLKe 9        // encoder S1
#define DTe 8         // encoder S2
#define SWe 10        // encoder Key

#define CLK 12        // display
#define DIO 11        // display

#define ZERO_PIN 2    // (Z-C) detector for dimmer
#define DIM_PIN 5     // DIM(PWM) pin for dimmer

#define BUZZ_PIN 7    // Buzzer
#define LED_PIN 6     // LED
#define BUTTON_PIN 4  // Button

// ***************** OBJECTS *****************
Encoder enc(CLKe, DTe, SWe);  // Encoder
GyverTM1637 disp(CLK, DIO);   // LED display
GButton button(BUTTON_PIN);   // Button
RTC_DS3231 rtc;               // RTC module

// ***************** TIMERS *****************
GTimer_ms halfsTimer(500);
GTimer_ms blinkTimer(800);
GTimer_ms timeoutTimer(15000);
GTimer_ms dutyTimer((long)DAWN_TIME * 60 * 1000 / (DAWN_MAX - DAWN_MIN));
GTimer_ms alarmTimeout((long)ALARM_TIMEOUT * 1000);

// ***************** VARIABLES *****************
boolean dotFlag, alarmFlag, minuteFlag, blinkFlag, newTimeFlag, buzz;
int8_t hrs = 21, mins = 55, secs;
int8_t alm_hrs, alm_mins;
int8_t dwn_hrs, dwn_mins;
byte mode;                    // 0 - clock, 1 - alarm setup, 2 - clock setup
byte alarmMode;               // 0 - dawn and buzz, 1 - buzz only, 2 - dawn only

boolean dawn_start = false;
boolean alarm = false;
boolean alarmHoldFlag = false;
volatile int tic, duty;

void setup() {
  Serial.begin(9600);
  pinMode(DIM_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Setting PWM for dimmer
  pinMode(ZERO_PIN, INPUT);
  attachInterrupt(0, detect_up, FALLING);
  StartTimer1(timer_interrupt, 40);        // One PWM period time
  StopTimer1();                            // stop timer

  // Setting up encoder type
  enc.setType(ENCODER_TYPE);     // Set encoder type
  
  // Setting up display
  disp.clear();
  disp.brightness(DISPLAY_BRIGHT);

  // Setting up RTC module and display time
  rtc.begin();
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  DateTime now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();

  mode = 0;
  buzz = true;
  disp.displayClock(hrs, mins);

  // Read out alarm settings
  alm_hrs = EEPROM.read(0);
  alm_mins = EEPROM.read(1);
  alarmFlag = EEPROM.read(2);
  alarmMode = EEPROM.read(3);
  alm_hrs = constrain(alm_hrs, 0, 23);
  alm_mins = constrain(alm_mins, 0, 59);
  calculateDawn();      // calculate dawn start time
  alarmFlag = constrain(alarmFlag, 0, 1);
  alarmMode = constrain(alarmMode, 0, 2);
}

void loop() {
  encoderTick();  // check encoder
  buttonTick();   // check button
  clockTick();    // perform time calculations
  alarmTick();    // check alarm
  settings();     // settings
}

void calculateDawn() {
  // Dawn start calculation
  if (alm_mins > DAWN_TIME) {
    dwn_hrs = alm_hrs;
    dwn_mins = alm_mins - DAWN_TIME;
  }
  else {
    dwn_hrs = alm_hrs - 1;
    if (dwn_hrs < 0) dwn_hrs = 23;
    dwn_mins = 60 - (DAWN_TIME - alm_mins);
  }
}


//---------------------- Interruption handling --------------------------
void timer_interrupt() {
  if (duty > 0) {
    tic++; 
    if (tic > (255 - duty))
      digitalWrite(DIM_PIN, 1);
  }
}

void detect_up() {
  if (duty > 0) {
    tic = 0; 
    ResumeTimer1();
    attachInterrupt(0, detect_down, RISING);
  }
}

void detect_down() {
  if (duty > 0) {
    tic = 0;
    StopTimer1();
    digitalWrite(DIM_PIN, 0);
    attachInterrupt(0, detect_up, FALLING);
  }
}


void settings() {
  // *********** Save settings if needed when returning to mode 0 ************
  if (mode == 0) {
    if (newTimeFlag) {             // If new time was set - setup new time
      newTimeFlag = false;
      secs = 0;
      rtc.adjust(DateTime(2014, 1, 21, hrs, mins, 0)); 
    }

    if (minuteFlag && !alarm) {    // if it's a new minute and not alarm show time
      minuteFlag = false;
      disp.displayClock(hrs, mins);
    }
  }
  
  // *********** Alarm setup mode **********
  if (mode == 1) {
    if (timeoutTimer.isReady()) {
      mode = 0;                               // return to mode 0 if timeout
      updateAlarm();
    }
    if (enc.isRight()) {
      alm_mins++;
      if (alm_mins > 59) {
        alm_mins = 0;
        alm_hrs++;
        if (alm_hrs > 23) alm_hrs = 0;
      }
    }
    if (enc.isLeft()) {
      alm_mins--;
      if (alm_mins < 0) {
        alm_mins = 59;
        alm_hrs--;
        if (alm_hrs < 0) alm_hrs = 23;
      }
    }
    if (enc.isRightH()) {
      alm_hrs++;
      if (alm_hrs > 23) alm_hrs = 0;
    }
    if (enc.isLeftH()) {
      alm_hrs--;
      if (alm_hrs < 0) alm_hrs = 23;
    }
    if (enc.isTurn() && !blinkFlag) {
      disp.displayClock(alm_hrs, alm_mins);
      timeoutTimer.reset();
    }
    if (blinkTimer.isReady()) {
      if (blinkFlag) {
        blinkFlag = false;
        blinkTimer.setInterval(700);
        disp.point(1);
        disp.displayClock(alm_hrs, alm_mins);
      } else {
        blinkFlag = true;
        blinkTimer.setInterval(300);
        disp.point(0);
        disp.clear();
      }
    }
  }

  // *********** Clock setup mode **********
  if (mode == 2) {
    if (timeoutTimer.isReady()) mode = 0;   // return to mode 0 if timeout

    if (enc.isRight()) {
      mins++;
      if (mins > 59) {
        mins = 0;
        hrs++;
        if (hrs > 23) hrs = 0;
      }
    }
    
    if (enc.isLeft()) {
      mins--;
      if (mins < 0) {
        mins = 59;
        hrs--;
        if (hrs < 0) hrs = 23;
      }
    }
    
    if (enc.isRightH()) {
      hrs++;
      if (hrs > 23) hrs = 0;
    }
    
    if (enc.isLeftH()) {
      hrs--;
      if (hrs < 0) hrs = 23;
    }
    
    if (enc.isTurn() && !blinkFlag) {
      disp.displayClock(hrs, mins);
      timeoutTimer.reset();
      newTimeFlag = true;             // enable flag for new time saving only in case encoder was turned
    }
    
    if (blinkTimer.isReady()) {
      disp.point(1);
      if (blinkFlag) {
        blinkFlag = false;
        blinkTimer.setInterval(700);
        disp.displayClock(hrs, mins);
      } else {
        blinkFlag = true;
        blinkTimer.setInterval(300);
        disp.clear();
      }
    }
  }
}

void buttonTick(){
  button.tick();  //tick the button

  if (button.isPress()){
    if (dawn_start || alarm){         // if the button is pressed during dawn or alarm - switch off both modes
      dawn_start = false;
      alarm = false;
      alarmHoldFlag = true;
      duty = 0;
      digitalWrite(DIM_PIN, 0);
      if (buzz) noTone(BUZZ_PIN);
      return;
    }
  }
  
  if (button.isSingle() && mode == 0){  //tap the button to see current alarm mode
    showAlarmMode();
  }
  
  if (button.isHolded()){
    if (mode == 0 && !dawn_start && !alarm) {   // button is holded and it's not dawn and not alarm
      disp.point(0);
      alarmFlag = !alarmFlag;                   // switch alarm flag
      if (alarmFlag) {
        disp.scrollByte(_empty, _o, _n, _empty, 70);
        analogWrite(LED_PIN, LED_BRIGHT);
      } else {
        disp.scrollByte(_empty, _o, _F, _F, 70);
        digitalWrite(LED_PIN, 0);
      }
      EEPROM.update(2, alarmFlag);
      delay(1000);
      disp.displayClockScroll(hrs, mins, 70);
    }
  }

  if (button.isDouble()){
    alarmMode++;
    if (alarmMode > 2) alarmMode = 0;
    EEPROM.update(3, alarmMode);
    showAlarmMode();
  }
}


void showAlarmMode(){
  byte normal_mode[] = {_L, _i, _G, _H, _t, _empty, _o, _n, _empty, _B, _E, _E, _P, _empty, _o, _n};
  byte dark_mode[] = {_L, _i, _G, _H, _t, _empty, _o, _F, _F, _empty, _B, _E, _E, _P, _empty, _o, _n};
  byte silent_mode[] = {_L, _i, _G, _H, _t, _empty, _o, _n, _empty, _B, _E, _E, _P, _empty, _o, _F, _F};

  disp.clear();
  disp.point(1);
  disp.displayClockScroll(alm_hrs, alm_mins, 70);
  delay(2000);
  disp.point(0);
  
  if (alarmMode == 0){
    disp.runningString(normal_mode, sizeof(normal_mode), 200);
  }
  else if (alarmMode == 1){
    disp.runningString(dark_mode, sizeof(dark_mode), 200);
  }
  else if (alarmMode == 2){
    disp.runningString(silent_mode, sizeof(silent_mode), 200);
  }
  
  disp.displayClockScroll(hrs, mins, 70);
}


void encoderTick() {
  enc.tick();   // tick encoder
  // *********** Encoder click **********
  if (enc.isClick() && mode > 0) {        // click encoder not in clock mode
    minuteFlag = true;
    mode = 0;
    calculateDawn();
    updateAlarm();
    disp.displayClock(hrs, mins);
    timeoutTimer.reset();
  }

  // *********** Encoder is holded **********
  if (enc.isHolded()) {       // Hold the button to switch modes
    minuteFlag = true;
    mode++;
    if (mode > 2) mode = 0;
    disp.displayClock(hrs, mins);
    timeoutTimer.reset();
  }
}


void updateAlarm(){
  int8_t _alm_hrs = EEPROM.read(0);
  int8_t _alm_mins = EEPROM.read(1);
  if (_alm_hrs != alm_hrs || _alm_mins != alm_mins){
    EEPROM.update(0, alm_hrs);
    EEPROM.update(1, alm_mins);
  }
}


void alarmTick() {
  if (dawn_start && alarmFlag) {
    if (dutyTimer.isReady()) {    // raise duty step by step according to the timer
      duty++;
      duty = constrain(duty, DAWN_MIN, DAWN_MAX);
    }
  }
  
  if (alarm) {                    // alarm time came
    if (alarmTimeout.isReady()) { // alarm timeout came
      dawn_start = false;
      alarm = false;
      duty = 0;
      digitalWrite(DIM_PIN, 0);
      if (buzz) noTone(BUZZ_PIN);
    }
    
    if (blinkTimer.isReady()) {   // blink with digits on LED screen and beep with buzzer
      if (blinkFlag) {
        blinkFlag = false;
        blinkTimer.setInterval(700);
        disp.point(1);
        disp.displayClock(hrs, mins);
        if (ALARM_BLINK) duty = DAWN_MAX;
        if (buzz) tone(BUZZ_PIN, BUZZ_FREQ);
      } 
      else {
        blinkFlag = true;
        blinkTimer.setInterval(300);
        disp.point(0);
        disp.clear();
        if (ALARM_BLINK) duty = DAWN_MIN;
        if (buzz) noTone(BUZZ_PIN);
      }
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

    if (alarmFlag) {
      if (dotFlag) analogWrite(LED_PIN, LED_BRIGHT);    // blink with LED if alarm is enabled
      else digitalWrite(LED_PIN, LOW);
    }

    if (dotFlag) {          // recalculate time every second
      secs++;
      if (secs > 59) {      // read time every minute
        DateTime now = rtc.now();
        secs = now.second();
        mins = now.minute();
        hrs = now.hour();
        minuteFlag = true;
      }

      // recalculate dawn and alarm start time every minute
      if (minuteFlag) {
        if (alarmMode == 2){
          buzz = false;
        }
        else {
          buzz = true;
        }

        
        if (dwn_hrs == hrs && dwn_mins == mins && alarmFlag && !dawn_start && mode == 0 && alarmMode != 1) {
          dawn_start = true;
          duty = DAWN_MIN;
        }
        if (alm_hrs == hrs && alm_mins == mins && alarmFlag && !alarm && mode == 0 && !alarmHoldFlag) {
          alarm = true;
          alarmTimeout.reset();
        }
        if (alm_hrs == hrs && alm_mins == mins && alarmHoldFlag) {
          alarmHoldFlag = false;
        }
      }
    }
  }
}
