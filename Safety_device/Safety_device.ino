//Safety device

//---------- Define pins and settings
#define BUTTON_1_PIN 3                     //Button pin
#define SERVO_PIN 5                        //Servo pin
#define SAFETY_ON_ANGLE 150                  //Servo angle in closed position (safety on)
#define SAFETY_OFF_ANGLE 5               //Servo angle in open position (safety off)
#define SERVO_SPEED 180                    //Servo speed
#define MAX_DELAY_COUNTER 6                //Maximum value of delay counter (number of possible states)
#define DISARMED_LED_BLINK_INTERVAL 250    //How often LED blinks in Disarmed mode
#define ARMED_LED_BLINK_INTERVAL 100       //How often LED blinks in Armed mode
#define DISARMED_LED_SERIES_INTERVAL 3000  //How often the series of LED blinks is shown in Disarmed mode
#define ARMED_LED_SERIES_INTERVAL 5000     //How often the series of LED blinks is shown in Armed mode


//---------- Include libraries
#include <GyverTimer.h>
#include <VirtualButton.h>
#include <ServoSmooth.h>

//---------- Initialize devices
VButton btn1;
ServoSmooth mainServo;

//---------- Timers
//GTimer startUpTimer(MS, 3000);
GTimer blinkTimer(MS);
GTimer blinkSeriesTimer(MS);
//GTimer blinkDisarmedTimer(MS, 3000);

//---------- Variables
boolean ledFlag = true;
boolean armedModeFlag = false;
boolean startUpFlag = true;
boolean ledBlinkFlag = false;
byte setDelayCounter = 2;
byte blinkCounter = 1;
byte mode = 0;



void setup() {
  //Pin modes
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  mainServo.attach(SERVO_PIN);
  //mainServo.smoothStart();
  
//  mainServo.setAccel(0);
//  mainServo.setSpeed(SERVO_SPEED);

//  digitalWrite(LED_BUILTIN, true);
//  delay(3000);
//  //digitalWrite(LED_BUILTIN, false);
//  closeSafetyGate();
//  startUpTimer.start();
  blinkTimer.setInterval(DISARMED_LED_BLINK_INTERVAL);
  blinkSeriesTimer.setInterval(DISARMED_LED_SERIES_INTERVAL);
  mode = 1;

}


void loop() {
  //mainServo.tick();
//  if(startUpFlag){
//    if(startUpTimer.isReady()){
//      startUpFlag = false;
//      ledFlag = false;
//      closeSafetyGate();      
//    }
//    else{
//      openSafetyGate();
//    }   
//  }

  buttonTick();
  ledTick();
  ledSwitch();
}

void buttonTick(){
  btn1.poll(!digitalRead(BUTTON_1_PIN));
  
  if(mode == 1){    
    if(btn1.click()){
      setDelayCounter ++;
      if(setDelayCounter > MAX_DELAY_COUNTER){
        setDelayCounter = 1;
      }
    }
  }

  if(btn1.held()){
    mode ++;
    if(mode > 2){
      mode = 1;
    }
  }
}


void ledTick(){
  if(blinkSeriesTimer.isReady()){
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

void openSafetyGate(){
  mainServo.write(SAFETY_OFF_ANGLE);
}

void closeSafetyGate(){
  mainServo.write(SAFETY_ON_ANGLE);
}

void ledSwitch() {
  digitalWrite(LED_BUILTIN, ledFlag);
}
