//Unified device by Rostyslav Gurevych

//---------- Define pins and settings
#define INIT_ADDR 1023                         //Number of EEPROM first launch check cell
#define INIT_KEY 10                            //First launch key
#define ACCEL_OFFSETS_BYTE 900                 //Nubmer of EEPROM cell where accel offsets are stored
#define BUTTON_1_PIN 2                         //Button 1 pin
#define BUTTON_2_PIN 3                         //Button 2 pin
#define RELAY_1_PIN 6                          //Safety guard relay pin (relay 1)
#define RELAY_2_PIN 7                          //Detonation relay pin (relay 2)
#define RELAY_1_TEST_PIN 8                     //Safety guard relay test pin (relay 1)
#define RELAY_2_TEST_PIN 9                     //Detonation relay test pin (relay 2)
#define MIN_GUARD_TIMER_VALUE 10               //Minimum safety guard timer value (in minutes)
#define MAX_GUARD_TIMER_VALUE 60               //Maximum safety guard timer value (in minutes)
#define DEFAULT_GUARD_TIMER_VALUE 40           //Default safety guard timer value on startup (in minutes)
#define MIN_SELF_DESTRUCT_TIMER_VALUE 60       //Minimum self-destruction timer value (in minutes)
#define MAX_SELF_DESTRUCT_TIMER_VALUE 600      //Maximum self-destruction timer value (in minutes)
#define DEFAULT_SELF_DESTRUCT_TIMER_VALUE 90   //Default self-destruction timer value on startup (in minutes)
#define MIN_ACCELERATION 4                     //Minimum acceleration limit
#define MAX_ACCELERATION 16                    //Maximum acceleration limit
#define DEFAULT_ACCELERATION 6                 //Default acceleration limit value
#define BUTTON_TIMEOUT 20000                   //Timeout after which device will return to idle mode from settings (without saving)
#define DEMO_MODE 1                            //Demo mode enabled (all times are reduced to seconds)
#define DEBUG_MODE 1                           //Debug mode enabled (Serial is activated and used for debugging)
#define ACC_COEF 2048                          //Divider to be used with 16G accelerometer
#define CALIBRATION_BUFFER_SIZE 100            //Buffer size needed for calibration function
#define CALIBRATION_TOLERANCE 500              //What is the calibration tolerance (units)
#define ACCEL_REQUEST_TIMEOUT 50               //Delay between accelerometer request


//---------- Include libraries
#include <GyverTimer.h>
#include <EncButton.h>
#include <Wire.h>
#include <GyverOLED.h>
#include <EEPROM.h>
#include "MPU6050.h"


//---------- Initialize devices
Button leftBtn(BUTTON_1_PIN, INPUT_PULLUP);
Button rightBtn(BUTTON_2_PIN, INPUT_PULLUP);
VirtButton bothBtn;
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
MPU6050 mpu;


//---------- Timers
//GTimer blinkTimer(MS, 500);
GTimer updateScreenTimer(MS, 500);
GTimer oneSecondTimer(MS, 1000);
GTimer explosionTimer(MS);
GTimer menuExitTimer(MS);
GTimer accelTimer(MS, ACCEL_REQUEST_TIMEOUT);


//---------- Variables
boolean safetyGuardActiveFlag = false, selfDestructActiveFlag = false, accelCheckFlag = false;
boolean blinkFlag = true, ledFlag = true;
byte max_acc, accelerationLimit;
unsigned int safetyGuardTimeout, safetyGuardTimeoutCounter, selfDestructTimeout, selfDestructTimeoutCounter;
byte mode = 0, oldMode = 0;
byte pointer = 2;
int32_t acc_x, acc_y, acc_z;
int16_t ax, ay, az;
long offsets[6] = {0,0,0,0,0,0};

void setup() {
  if(DEBUG_MODE) Serial.begin(9600);
  Wire.begin();

  //Pin modes
  safetyGuardDisable();
  detonateDisable();
    
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_1_TEST_PIN, INPUT_PULLUP);
  pinMode(RELAY_2_TEST_PIN, INPUT_PULLUP);
  bothBtn.setHoldTimeout(2000);

  //OLED
  oled.init();
  oled.clear();
  oled.setContrast(200);

  // EEPROM
  if (EEPROM.read(INIT_ADDR) != INIT_KEY) {     // First launch
    EEPROM.write(INIT_ADDR, INIT_KEY);
    EEPROM.put(10, DEFAULT_GUARD_TIMER_VALUE);
    EEPROM.put(20, DEFAULT_SELF_DESTRUCT_TIMER_VALUE);
    EEPROM.put(30, DEFAULT_ACCELERATION);
    EEPROM.put(ACCEL_OFFSETS_BYTE, offsets);
  }
  EEPROM.get(10, safetyGuardTimeout);
  EEPROM.get(20, selfDestructTimeout);
  EEPROM.get(30, accelerationLimit);
  EEPROM.get(ACCEL_OFFSETS_BYTE, offsets);

  //Accelerometer
  mpu.initialize();
  if(mpu.testConnection()){
    if(DEBUG_MODE) Serial.println(F("MPU6050 check - SUCCESS"));
    drawIntroScreen();
  }
  else{
    if(DEBUG_MODE) Serial.println(F("MPU6050 check - FAILED"));
    drawErrorIntroScreen();
    while(1) {delay(1000);}
  }
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  mpu.setXAccelOffset(offsets[0]);
  mpu.setYAccelOffset(offsets[1]);
  mpu.setZAccelOffset(offsets[2]);
  mpu.setXGyroOffset(offsets[3]);
  mpu.setYGyroOffset(offsets[4]);
  mpu.setZGyroOffset(offsets[5]);
  
  //Startup preparation and check
  safetyGuardDisable();
  detonateDisable();
  delay(1000);

  //Variables
  mode = 1;
  menuExitTimer.setTimeout(BUTTON_TIMEOUT);

  //Draw default screen after setup
  drawDefaultScreen();
}


void loop() {
  buttonTick();
  timersCountdown();
  changeMode();
  updateScreen();
  checkAccel();
  operationTick();
  ledCheck();
}


void(* resetFunc) (void) = 0;


void buttonTick(){
  if(leftBtn.tick() || rightBtn.tick()) menuExitTimer.start();
  bothBtn.tick(leftBtn, rightBtn);
  
  if(mode == 1){
    if(rightBtn.hold()){
      mode = 2;
      pointer = 2;
    }
    
    if(bothBtn.hold()){
      safetyGuardCountdownStart();
      selfDestructCountdownStart();
      pinMode(RELAY_1_TEST_PIN, OUTPUT);
      digitalWrite(RELAY_1_TEST_PIN, HIGH);
      mode = 4;
    }

    if(rightBtn.hasClicks(5)){
      calibrateAccel();
    }

    if(leftBtn.hasClicks(5)){
      selfTest();
    }
  }

  else if(mode == 2){
    if(leftBtn.click()){
      pointer++;
      if (pointer > 4) pointer = 2;
    }

    else if(leftBtn.hold()){
      EEPROM.put(10, safetyGuardTimeout);
      EEPROM.put(20, selfDestructTimeout);
      EEPROM.put(30, accelerationLimit);
      mode = 1;
    }

    else if(rightBtn.hold()){
      mode = 3;
    }

    if(menuExitTimer.isReady()){
      EEPROM.get(10, safetyGuardTimeout);
      EEPROM.get(20, selfDestructTimeout);
      EEPROM.get(30, accelerationLimit);
      mode = 1;
    }
  }

  else if(mode == 3){
    
    if(leftBtn.click()){
      if(pointer == 2) safetyGuardTimeout -= 10;
      
      else if(pointer == 3) selfDestructTimeout -= 10;
  
      else if(pointer == 4) accelerationLimit --;
    }

    if(rightBtn.click()){
      if(pointer == 2) safetyGuardTimeout += 10;
      
      else if(pointer == 3) selfDestructTimeout += 10;
  
      else if(pointer == 4) accelerationLimit ++;
    }

    safetyGuardTimeout = constrain(safetyGuardTimeout, MIN_GUARD_TIMER_VALUE, MAX_GUARD_TIMER_VALUE);
    selfDestructTimeout = constrain(selfDestructTimeout, MIN_SELF_DESTRUCT_TIMER_VALUE, MAX_SELF_DESTRUCT_TIMER_VALUE);
    accelerationLimit = constrain(accelerationLimit, MIN_ACCELERATION, MAX_ACCELERATION);

    if(leftBtn.hold()){
      mode = 2;
    }

    if(menuExitTimer.isReady()){
      EEPROM.get(10, safetyGuardTimeout);
      EEPROM.get(20, selfDestructTimeout);
      EEPROM.get(30, accelerationLimit);
      mode = 1;
    }
  }

  else if(mode >= 4 && mode <= 7){
    if(bothBtn.hold()){
      safetyGuardActiveFlag = false;
      selfDestructActiveFlag = false;
      accelCheckFlag = false;
      detonateDisable();
      safetyGuardDisable();
      mode = 1;
    }
  }
}


void operationTick(){
  if(accelCheckFlag){
    if(max_acc >= accelerationLimit){
      if(DEBUG_MODE){
        Serial.print(F("Acceleration limit ")); Serial.print(accelerationLimit);
        Serial.print(F(" is reached, current acc = ")); Serial.print(max_acc);
        Serial.println(F(", detonating!!!"));
      }
      detonateEnable();
      mode = 7;
      accelCheckFlag = false;
    }
  }
  
  if(safetyGuardActiveFlag){
    if(safetyGuardTimeoutCounter == 0){
      if(DEBUG_MODE) Serial.println(F("Deactivating Safety guard, device armed"));
      safetyGuardDisable();
      safetyGuardActiveFlag = false;
      accelCheckFlag = true;
      mode = 5;
    }
  }

  if(selfDestructActiveFlag){
    if(selfDestructTimeoutCounter == 0){
      if(DEBUG_MODE) Serial.print(F("Self-destruct timeout is reached! "));
      if(safetyGuardActiveFlag){
        if(DEBUG_MODE) Serial.println(F("Safety guard is still on, detonation blocked!"));
      }
      else{
      if(DEBUG_MODE) Serial.println(F("Detonating!!!"));
      detonateEnable();
      }
      mode = 6;
      selfDestructActiveFlag = false;
    }
  }
}


void safetyGuardCountdownStart(){
  if(!safetyGuardActiveFlag){
    safetyGuardTimeoutCounter = safetyGuardTimeout;
    if(!DEMO_MODE) safetyGuardTimeoutCounter *= 60;
    if(DEBUG_MODE){
      Serial.print(F("Activating Safety guard, timeout: "));
      Serial.print(safetyGuardTimeoutCounter);
      Serial.println(F(" s"));
    }
    safetyGuardEnable();
    safetyGuardActiveFlag = true;
    accelCheckFlag = false;
  }
}

void selfDestructCountdownStart(){
  if(!selfDestructActiveFlag){
    selfDestructTimeoutCounter = selfDestructTimeout;
    if(!DEMO_MODE) selfDestructTimeoutCounter *= 60;
    if(DEBUG_MODE){
      Serial.print(F("Activating Self-destruct timer with timeout: "));
      Serial.print(selfDestructTimeoutCounter);
      Serial.println(F(" s"));
    }
    selfDestructActiveFlag = true;
  }
}


void timersCountdown(){
  if(oneSecondTimer.isReady()){
    ledFlag = !ledFlag;
    
    if(safetyGuardActiveFlag){
      if(safetyGuardTimeoutCounter > 0){
        safetyGuardTimeoutCounter --;
      }
//      if(DEBUG_MODE){
//        Serial.print(F("Safety guard remaining time: "));
//        Serial.print(safetyGuardTimeoutCounter);
//        Serial.println(F(" s"));
//      }
    }

    if(selfDestructActiveFlag){
      if(selfDestructTimeoutCounter > 0){
        selfDestructTimeoutCounter --;
      }
//      if(DEBUG_MODE){
//        Serial.print(F("Self-destruct remaining time: "));
//        Serial.print(selfDestructTimeoutCounter);
//        Serial.println(F(" s"));
//      }
    }
  }
}


void checkAccel(){
  if(accelCheckFlag){
    if(accelTimer.isReady()){
      mpu.getAcceleration(&ax, &ay, &az);
  
      acc_x = abs(ax / ACC_COEF);
      acc_y = abs(ay / ACC_COEF);
      acc_z = abs(az / ACC_COEF);
  
      max_acc = defineMaxAccel(acc_x, acc_y, acc_z);
  
      if(DEBUG_MODE){
      Serial.print(acc_x); Serial.print(F("  "));
      Serial.print(acc_y); Serial.print(F("  "));
      Serial.print(acc_z); Serial.print(F("  Max is: "));
      Serial.println(max_acc);
      }
    }
  }
}


byte defineMaxAccel(int16_t acc_x, int16_t acc_y, int16_t acc_z){
  return max(max(acc_x, acc_y), acc_z);
}
