//Unified device by Rostyslav Gurevych

//---------- Define pins and settings
#define BUTTON_1_PIN 2                         //Button 1 pin
#define BUTTON_2_PIN 3                         //Button 2 pin
#define RELAY_1_PIN 6                          //Relay 1 pin
#define RELAY_2_PIN 7                          //Relay 2 pin
#define MIN_GUARD_TIMER_VALUE 30               //Minimum guard timer value (in minutes)
#define MAX_GUARD_TIMER_VALUE 60               //Maximum guard timer value (in minutes)
#define DEFAULT_GUARD_TIMER_VALUE 40           //Default guard timer value on startup (in minutes)
#define MIN_SELF_DESTRUCT_TIMER_VALUE 60       //Minimum self-destruction timer value (in minutes)
#define MAX_SELF_DESTRUCT_TIMER_VALUE 600      //Maximum self-destruction timer value (in minutes)
#define DEFAULT_SELF_DESTRUCT_TIMER_VALUE 60   //Default self-destruction timer value on startup (in minutes)

//#define DISARMED_LED_BLINK_INTERVAL 250    //How often LED blinks in Disarmed mode
//#define ARMED_LED_BLINK_INTERVAL 125       //How often LED blinks in Armed mode
//#define DISARMED_LED_SERIES_INTERVAL 3000  //How often the series of LED blinks is shown in Disarmed mode
//#define ARMED_LED_SERIES_INTERVAL 5000     //How often the series of LED blinks is shown in Armed mode
//#define MODE_CHANGE_INDICATION 2000        //How long the LED will be on when mode is changed
//#define OPERATION_MULTIPLICATOR 600        //Period of time equal to one unit of delay counter (seconds)
#define DEMO_MODE 1                        //Demo mode enabled (all times are reduced to seconds)


//---------- Include libraries
#include <GyverTimer.h>
#include <EncButton.h>
#include <Wire.h>


//---------- Initialize devices
Button leftBtn(BUTTON_1_PIN, INPUT_PULLUP);
Button rightBtn(BUTTON_2_PIN, INPUT_PULLUP);
VirtButton bothBtn;


//---------- Timers
//GTimer blinkTimer(MS);
//GTimer blinkSeriesTimer(MS);
GTimer oneSecondTimer(MS, 1000);
GTimer explosionTimer(MS);

//---------- Variables
boolean safetyGuardActiveFlag = false, selfDestructActiveFlag = false;
boolean ledFlag = false;
//boolean armedModeFlag = false;
boolean startUpFlag = true;
//boolean ledBlinkFlag = false;
//boolean modeChangeFlag = false;
//byte setDelayCounter = DEFAULT_TIMER_VALUE;
//byte blinkCounter;
unsigned int safetyGuardTimeout, safetyGuardTimeoutCounter, selfDestructTimeout, selfDestructTimeoutCounter;
byte mode = 0;



void setup() {
  Serial.begin(9600);

  //Pin modes
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);


  //Startup preparation and check
  safetyGuardDisable();
  detonateDisable();
  ledFlag = true;
  ledSwitch();
  delay(500);
  ledFlag = false;
  ledSwitch();

  //Variables
  safetyGuardTimeout = DEFAULT_GUARD_TIMER_VALUE;
  selfDestructTimeout = DEFAULT_SELF_DESTRUCT_TIMER_VALUE;
  mode = 1;
}


void loop() {
  buttonTick();
  timersCountdown();
  operationTick();
  ledSwitch();
}


void buttonTick(){
  leftBtn.tick();
  rightBtn.tick();
  bothBtn.tick(leftBtn, rightBtn);
  
  if(leftBtn.click()){
    ledFlag = true;
  }

  if(rightBtn.click()){
    ledFlag = false;
  }

  if(bothBtn.hold()){
    safetyGuardCountdownStart();
    selfDestructCountdownStart();
  }
}

//
//void modeChangeIndication(){
//  modeChangeTimer.start();
//  modeChangeFlag = true;
//}


//void ledTick(){
//  if(modeChangeFlag){
//    if(modeChangeTimer.isReady()){
//      ledFlag = false;
//      modeChangeFlag = false;
//      blinkSeriesTimer.reset();
//    }
//    else{
//      ledFlag = true;
//      return;
//    }
//  }
//  
//  if(blinkSeriesTimer.isReady() && (mode == 1 || mode == 2)){
//    ledBlinkFlag = true;
//    blinkCounter = 1;
//    blinkTimer.reset();
//  }
//
//  if(ledBlinkFlag){
//    if(blinkCounter > setDelayCounter){
//      ledBlinkFlag = false;
//    }
//    
//    if(blinkTimer.isReady()){
//      if(ledFlag){
//        ledFlag = false;
//        blinkCounter ++;
//        blinkSeriesTimer.reset();
//      }
//      else{
//        ledFlag = true;
//      }
//    }
//  }
//}


void operationTick(){
  if(safetyGuardActiveFlag){
    if(safetyGuardTimeoutCounter == 0){
      Serial.println(F("Deactivating Safety guard, device armed"));
      safetyGuardDisable();
      safetyGuardActiveFlag = false;
    }
  }

  if(selfDestructActiveFlag){
    if(selfDestructTimeoutCounter == 0){
      Serial.print(F("Self-destruct timeout is reached! "));
      if(safetyGuardActiveFlag){
        Serial.println(F("Safety guard is still on, detonation blocked!"));
      }
      else{
      Serial.println(F("Detonating!!!"));
      detonateEnable();
      }
      selfDestructActiveFlag = false;
    }
  }
}


void safetyGuardCountdownStart(){
  if(!safetyGuardActiveFlag){
    safetyGuardTimeoutCounter = safetyGuardTimeout;
    if(!DEMO_MODE) safetyGuardTimeoutCounter *= 60;
    Serial.print(F("Activating Safety guard, timeout: "));
    Serial.print(safetyGuardTimeoutCounter);
    Serial.println(F(" s"));
    safetyGuardEnable();
    safetyGuardActiveFlag = true;
  }
}

void selfDestructCountdownStart(){
  if(!selfDestructActiveFlag){
    selfDestructTimeoutCounter = selfDestructTimeout;
    if(!DEMO_MODE) selfDestructTimeoutCounter *= 60;
    Serial.print(F("Starting Self-destruct timer with timeout: "));
    Serial.print(selfDestructTimeoutCounter);
    Serial.println(F(" s"));
    selfDestructActiveFlag = true;
  }
}

void timersCountdown(){
  if(oneSecondTimer.isReady()){
    
    if(safetyGuardActiveFlag){
      if(safetyGuardTimeoutCounter > 0){
        safetyGuardTimeoutCounter --;
      }
      Serial.print(F("Safety guard remaining time: "));
      Serial.print(safetyGuardTimeoutCounter);
      Serial.println(F(" s"));
    }

    if(selfDestructActiveFlag){
      if(selfDestructTimeoutCounter > 0){
        selfDestructTimeoutCounter --;
      }
      Serial.print(F("Self-destruct remaining time: "));
      Serial.print(selfDestructTimeoutCounter);
      Serial.println(F(" s"));
    }
  }
  
}


void safetyGuardEnable(){
  digitalWrite(RELAY_1_PIN, LOW);
}


void safetyGuardDisable(){
  digitalWrite(RELAY_1_PIN, HIGH);
}


void detonateEnable(){
  digitalWrite(RELAY_2_PIN, LOW);
}


void detonateDisable(){
  digitalWrite(RELAY_2_PIN, HIGH);
}


void ledSwitch() {
  digitalWrite(LED_BUILTIN, ledFlag);
}
