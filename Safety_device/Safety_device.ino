//Safety device by Rostyslav Gurevych

//---------- Define pins and constants
#define BUTTON_1_PIN 3                     //Button pin
#define SERVO_PIN 5                        //Servo pin
#define SAFETY_ON_ANGLE 160                //Servo angle in closed position (safety on)
#define SAFETY_OFF_ANGLE 5                 //Servo angle in open position (safety off)
#define MIN_TIMER_UNIT_VALUE 2             //Minimum timer value (in units)
#define MAX_TIMER_UNIT_VALUE 3             //Maximum timer value (in units)
#define DEFAULT_TIMER_VALUE 2              //Default timer value on startup (in units)
#define DISARMED_LED_BLINK_INTERVAL 250    //How often LED blinks in Disarmed mode
#define ARMED_LED_BLINK_INTERVAL 125       //How often LED blinks in Armed mode
#define DISARMED_LED_SERIES_INTERVAL 3000  //How often the series of LED blinks is shown in Disarmed mode
#define ARMED_LED_SERIES_INTERVAL 5000     //How often the series of LED blinks is shown in Armed mode
#define MODE_CHANGE_INDICATION 2000        //How long the LED will be on when mode is changed
#define OPERATION_MULTIPLICATOR 600        //Period of time equal to one unit of delay counter (seconds)


//---------- Include libraries
#include <GyverTimer.h>
#include <VirtualButton.h>
#include <ServoSmooth.h>

//---------- Initialize devices
VButton btn1;
ServoSmooth mainServo;

//---------- Timers
GTimer blinkTimer(MS);
GTimer blinkSeriesTimer(MS);
GTimer modeChangeTimer(MS);
GTimer operationTimer(MS);

//---------- Variables
boolean ledFlag = false;
boolean armedModeFlag = false;
boolean startUpFlag = true;
boolean ledBlinkFlag = false;
boolean modeChangeFlag = false;
byte setDelayCounter = DEFAULT_TIMER_VALUE;
byte blinkCounter;
byte mode = 0;



void setup() {
  //Pin modes
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  mainServo.attach(SERVO_PIN);

  //Startup check
  ledFlag = true;
  ledSwitch();
  closeSafetyGate();
  delay(MODE_CHANGE_INDICATION);
  openSafetyGate();
  delay(MODE_CHANGE_INDICATION);
  closeSafetyGate();
  ledFlag = false;
  ledSwitch();

  //Start timers
  blinkTimer.setInterval(DISARMED_LED_BLINK_INTERVAL);
  blinkSeriesTimer.setInterval(DISARMED_LED_SERIES_INTERVAL);
  modeChangeTimer.setTimeout(MODE_CHANGE_INDICATION);
  mode = 1;
}


void loop() {
  buttonTick();
  operationTick();
  ledTick();
  ledSwitch();
}


void buttonTick(){
  btn1.poll(!digitalRead(BUTTON_1_PIN));
  
  if(mode == 1){    
    if(btn1.click()){
      setDelayCounter ++;
      if(setDelayCounter > MAX_TIMER_UNIT_VALUE){
        setDelayCounter = MIN_TIMER_UNIT_VALUE;
      }
    }
  }

  if(btn1.held()){
    mode ++;
    
    if(mode > 2){
      mode = 1;
    }
    
    if(mode == 1){
      blinkTimer.setInterval(DISARMED_LED_BLINK_INTERVAL);
      blinkSeriesTimer.setInterval(DISARMED_LED_SERIES_INTERVAL);
      operationTimer.stop();
      closeSafetyGate();
    }

    else if(mode == 2){
      blinkTimer.setInterval(ARMED_LED_BLINK_INTERVAL);
      blinkSeriesTimer.setInterval(ARMED_LED_SERIES_INTERVAL);
      operationTimer.setTimeout(setDelayCounter * OPERATION_MULTIPLICATOR * 1000L);
      operationTimer.start();
    }
    
    blinkSeriesTimer.reset();
    ledBlinkFlag = false;
    modeChangeIndication();
  }
}


void modeChangeIndication(){
  modeChangeTimer.start();
  modeChangeFlag = true;
}


void ledTick(){
  if(modeChangeFlag){
    if(modeChangeTimer.isReady()){
      ledFlag = false;
      modeChangeFlag = false;
      blinkSeriesTimer.reset();
    }
    else{
      ledFlag = true;
      return;
    }
  }
  
  if(blinkSeriesTimer.isReady() && (mode == 1 || mode == 2)){
    ledBlinkFlag = true;
    blinkCounter = 1;
    blinkTimer.reset();
  }

  if(ledBlinkFlag){
    if(blinkCounter > setDelayCounter){
      ledBlinkFlag = false;
    }
    
    if(blinkTimer.isReady()){
      if(ledFlag){
        ledFlag = false;
        blinkCounter ++;
        blinkSeriesTimer.reset();
      }
      else{
        ledFlag = true;
      }
    }
  }
}


void operationTick(){
  if(mode == 2){
    if(operationTimer.isReady()){
      mode = 3;
      modeChangeIndication();
      openSafetyGate();
    }
  }
}


void openSafetyGate(){
  mainServo.write(SAFETY_OFF_ANGLE);
}


void closeSafetyGate(){
  mainServo.write(SAFETY_ON_ANGLE);
}


void ledSwitch() {
  digitalWrite(LED_BUILTIN, ledFlag);
}
