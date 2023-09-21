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
#define DEFAULT_ACCELERATION 6                 //Default acceleration limit value

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
#include <GyverOLED.h>


//---------- Initialize devices
Button leftBtn(BUTTON_1_PIN, INPUT_PULLUP);
Button rightBtn(BUTTON_2_PIN, INPUT_PULLUP);
VirtButton bothBtn;
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;


//---------- Timers
//GTimer blinkTimer(MS, 500);
GTimer updateScreenTimer(MS, 500);
GTimer oneSecondTimer(MS, 1000);
GTimer explosionTimer(MS);

//---------- Variables
boolean safetyGuardActiveFlag = false, selfDestructActiveFlag = false;
boolean ledFlag = false;
boolean blinkFlag = true;
//boolean armedModeFlag = false;
//boolean startUpFlag = true;
//boolean ledBlinkFlag = false;
//boolean modeChangeFlag = false;
//byte setDelayCounter = DEFAULT_TIMER_VALUE;
//byte blinkCounter;
unsigned int safetyGuardTimeout, safetyGuardTimeoutCounter, selfDestructTimeout, selfDestructTimeoutCounter;
byte mode = 0, oldMode = 0;
byte pointer = 2;



void setup() {
  Serial.begin(9600);

  //Pin modes
  safetyGuardDisable();
  detonateDisable();
    
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  bothBtn.setHoldTimeout(2000);

  //OLED
  oled.init();
  oled.clear();
  oled.setContrast(200);
  drawIntroScreen();
  
  //Startup preparation and check
  safetyGuardDisable();
  detonateDisable();
  ledFlag = true;
  ledSwitch();
  delay(1000);
  ledFlag = false;
  ledSwitch();

  //Variables
  safetyGuardTimeout = DEFAULT_GUARD_TIMER_VALUE;
  selfDestructTimeout = DEFAULT_SELF_DESTRUCT_TIMER_VALUE;
  mode = 1;

  drawDefaultScreen();
}


void loop() {
  buttonTick();
  timersCountdown();
  changeMode();
  updateScreen();
  operationTick();
}


void buttonTick(){
  leftBtn.tick();
  rightBtn.tick();
  bothBtn.tick(leftBtn, rightBtn);
  
  if(mode == 1){
    if(leftBtn.hold()){
      mode = 2;
      pointer = 2;
    }
    
    if(bothBtn.hold()){
      safetyGuardCountdownStart();
      selfDestructCountdownStart();
      mode = 3;
    }
  }

  else if(mode == 2){
    if(leftBtn.click()){
      pointer++;
      if (pointer > 4) pointer = 2;
    }

    if(leftBtn.hold()){
      mode = 1;
    }
  }

  else if(mode == 3 || mode == 4 || mode == 5){
    if(bothBtn.hold()){
      safetyGuardActiveFlag = false;
      selfDestructActiveFlag = false;
      detonateDisable();
      safetyGuardDisable();
      mode = 1;
    }
  }
//  if(leftBtn.click()){
//    ledFlag = true;
////    drawScreen();
//  }
//
//  if(rightBtn.click()){
//    ledFlag = false;
//    oled.clear();
//  }
}


void operationTick(){
  if(safetyGuardActiveFlag){
    if(safetyGuardTimeoutCounter == 0){
      Serial.println(F("Deactivating Safety guard, device armed"));
      safetyGuardDisable();
      safetyGuardActiveFlag = false;
      mode = 4;
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
      mode = 5;
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
//      Serial.print(F("Safety guard remaining time: "));
//      Serial.print(safetyGuardTimeoutCounter);
//      Serial.println(F(" s"));
    }

    if(selfDestructActiveFlag){
      if(selfDestructTimeoutCounter > 0){
        selfDestructTimeoutCounter --;
      }
//      Serial.print(F("Self-destruct remaining time: "));
//      Serial.print(selfDestructTimeoutCounter);
//      Serial.println(F(" s"));
    }
  //drawScreen();
  }
  
}


void changeMode(){
  if(mode != oldMode){

    updateScreenTimer.reset();
    
    if(mode == 1){
      oled.setCursor(48, 0);
      oled.print(F("IDLE         "));

      oled.setCursor(90, 2);
      oled.print(safetyGuardTimeout);
      if(DEMO_MODE) oled.print(F(" s"));
      else oled.print(F(" m"));

      oled.setCursor(90, 3);
      oled.print(selfDestructTimeout);
      if(DEMO_MODE) oled.print(F(" s"));
      else oled.print(F(" m"));

      oled.setCursor(90, 4);
      oled.print(DEFAULT_ACCELERATION);
      oled.print(F(" G"));

      oled.setCursor(0, 6);
      oled.println(F("Hold L to setup      "));
      oled.println(F("Hold L+R 2s to start "));

      clearPointer();
    }

    else if(mode == 2){
      oled.setCursor(48, 0);
      oled.print(F("SETTINGS     "));

      oled.setCursor(90, 2);
      oled.print(safetyGuardTimeout);
      if(DEMO_MODE) oled.print(F(" s"));
      else oled.print(F(" m"));

      oled.setCursor(90, 3);
      oled.print(selfDestructTimeout);
      if(DEMO_MODE) oled.print(F(" s"));
      else oled.print(F(" m"));

      oled.setCursor(90, 4);
      oled.print(DEFAULT_ACCELERATION);
      oled.print(F(" G"));

      oled.setCursor(0, 6);
      oled.println(F("L-move, Hold R-change"));
      oled.println(F("Hold L to save & exit"));
    }

    else if(mode == 3){
      oled.setCursor(48, 0);
      oled.print(F("ON, DISARMED "));

      oled.setCursor(90, 4);
      oled.print(DEFAULT_ACCELERATION);
      oled.print(F(" G"));

      oled.setCursor(0, 6);
      oled.println(F("                     "));
      oled.println(F("Hold L+R 2s to stop  "));
    }

    else if(mode == 4){
      oled.setCursor(48, 0);
      oled.print(F("ON, ARMED!   "));

      oled.setCursor(90, 2);
      oled.print(F("Off   "));

      oled.setCursor(90, 4);
      oled.print(DEFAULT_ACCELERATION);
      oled.print(F(" G"));

      oled.setCursor(0, 6);
      oled.println(F("                     "));
      oled.println(F("Hold L+R 2s to stop  "));

      clearPointer();
    }

    else if(mode == 5){
      oled.setCursor(48, 0);
      oled.print(F("COMPLETED    "));

      oled.setCursor(90, 2);
      oled.print(F("Off   "));

      oled.setCursor(90, 3);
      oled.print(F("Boom  "));

      oled.setCursor(90, 4);
      oled.print(DEFAULT_ACCELERATION);
      oled.print(F(" G"));

      oled.setCursor(0, 6);
      oled.println(F("                     "));
      oled.println(F("Hold L+R 2s to reset "));     
    }

    
    oldMode = mode;
  }
}

void clearPointer(){
    for(byte i=2; i<5; i++){
      oled.setCursor(0, i);
      oled.print(F(" "));
  }
}

void updateScreen(){
  if(updateScreenTimer.isReady()){
    blinkFlag = !blinkFlag;

    if(mode == 2){
      for(byte i=2; i<5; i++){
        oled.setCursor(0, i);
        if(i == pointer) oled.print(F(">"));
        else oled.print(F(" "));
      }
      
      oled.setCursor(90, 2);
      oled.print(safetyGuardTimeout);
      if(DEMO_MODE) oled.print(F(" s"));
      else oled.print(F(" m"));

      oled.setCursor(90, 3);
      oled.print(selfDestructTimeout);
      if(DEMO_MODE) oled.print(F(" s"));
      else oled.print(F(" m"));

      oled.setCursor(90, 4);
      oled.print(DEFAULT_ACCELERATION);
      oled.print(F(" G"));
    }

    else if(mode == 3){
      
      oled.setCursor(90, 2);
      oled.print(safetyGuardTimeoutCounter / 60);
      if(blinkFlag){
        oled.print(F(":"));
        if(safetyGuardTimeoutCounter % 60 < 10) oled.print(F("0"));
        oled.print(safetyGuardTimeoutCounter % 60);
        oled.print(F("    "));
      }
      else oled.print(F(" "));
      

      oled.setCursor(90, 3);
      oled.print(selfDestructTimeoutCounter / 60);
      if(blinkFlag){
        oled.print(F(":"));
        if(selfDestructTimeoutCounter % 60 < 10) oled.print(F("0"));
        oled.print(selfDestructTimeoutCounter % 60);
        oled.print(F("    "));
      }
      else oled.print(F(" "));

    }

    else if(mode == 4){
      oled.setCursor(90, 3);
      oled.print(selfDestructTimeoutCounter / 60);
      if(blinkFlag){
        oled.print(F(":"));
        if(selfDestructTimeoutCounter % 60 < 10) oled.print(F("0"));
        oled.print(selfDestructTimeoutCounter % 60);
        oled.print(F("    "));
      }
      else oled.print(F(" "));
    }
    
  }
}


void drawDefaultScreen(){
  oled.clear();
  oled.home();
  oled.setScale(1);

  oled.print(F("Status: "));
  oled.line(0, 12, 127, 12);

  oled.setCursor(0, 2);
  oled.println(F(" Safety guard:       "));
  oled.println(F(" Self-destroy:       "));
  oled.println(F(" Acceleration:       "));

  oled.line(0, 44, 127, 44);
  oled.update();
}


void drawIntroScreen(){
  oled.clear();
  oled.setScale(1);
  oled.setCursor(40, 1);
  oled.println("MAY THE");

  oled.setScale(2);
  oled.setCursor(10, 3);
  oled.println("SCHWARTZ");

  oled.setScale(1);
  oled.setCursor(30, 6);
  oled.println("BE WITH YOU");
  
  oled.update();
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
