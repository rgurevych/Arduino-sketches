/*Unified device by Rostyslav Gurevych
Mode description:
1 - Idle
2 - Settings
3 - Change value
4 - Working, safety guard enabled
5 - Working, armed (safety guard off)
6 - Detonation by timer
7 - Detonation by accelerometer
*/

//---------- Define pins and settings
#define VERSION 0.92                            //Firmware version
#define INIT_ADDR 1023                         //Number of EEPROM first launch check cell
#define INIT_KEY 10                            //First launch key
#define ACCEL_OFFSETS_BYTE 900                 //Nubmer of EEPROM cell where accel offsets are stored
#define BUTTON_1_PIN 17                        //Button 1 pin
#define BUTTON_2_PIN 16                        //Button 2 pin
#define RELAY_1_PIN 6                          //Safety guard relay pin (relay 1)
#define RELAY_2_PIN 7                          //Detonation relay pin (relay 2)
#define SAFETY_LED_PIN 8                       //Safety guard LED pin
#define RELAY_TEST_PIN 9                       //Relay test pin (for self-test)
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
#define DEMO_MODE 1                            //Initially Demo mode enabled (all times are reduced to seconds)
#define DEBUG_MODE 1                           //Initially Debug mode enabled (Serial is activated and used for debugging)
#define ACC_COEF 2048                          //Divider to be used with 16G accelerometer
#define CALIBRATION_BUFFER_SIZE 100            //Buffer size needed for calibration function
#define CALIBRATION_TOLERANCE 500              //What is the calibration tolerance (units)
#define ACCEL_REQUEST_TIMEOUT 20               //Delay between accelerometer request
#define RELEASE_AFTER_DETONATION 5000          //Timeout after which the detonation relay is released (after detonation)
#define LED_BLINK_DURATION 100                 //Duration of LED blinks
#define LED_BLINK_INTERVAL 1400                //Interval between LED blinks


//---------- Include libraries
#include <TimerMs.h>
#include <EncButton.h>
#include <Wire.h>
#include <GyverOLED.h>
#include <EEPROM.h>
#include <MPU6050.h>


//---------- Initialize devices
Button leftBtn(BUTTON_1_PIN, INPUT_PULLUP);
Button rightBtn(BUTTON_2_PIN, INPUT_PULLUP);
VirtButton bothBtn;
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
MPU6050 mpu;


//---------- Timers
TimerMs updateScreenTimer(500, 1);
TimerMs oneSecondTimer(1000, 1);
TimerMs blinkTimer(LED_BLINK_DURATION, 1, 1);
TimerMs blinkIntervalTimer(LED_BLINK_INTERVAL, 1, 1);
TimerMs menuExitTimer(BUTTON_TIMEOUT, 0, 1);
TimerMs accelTimer(ACCEL_REQUEST_TIMEOUT, 1);
TimerMs releaseDetonationTimer(RELEASE_AFTER_DETONATION, 0, 1);


//---------- Variables
bool safetyGuardActiveFlag = false, selfDestructActiveFlag = false, accelCheckFlag = false;
bool blinkFlag = true, ledFlag = true, ledBlinkFlag = true;
bool demoMode, debugMode;
uint8_t max_acc, accelerationLimit, debugMaxAccel = 0;
uint16_t safetyGuardTimeout, safetyGuardTimeoutCounter, selfDestructTimeout, selfDestructTimeoutCounter;
uint8_t mode = 1, oldMode = 0;
uint8_t pointer = 2;
int32_t acc_x, acc_y, acc_z;
int16_t ax, ay, az;
long offsets[6] = {0,0,0,0,0,0};

void setup() {
  Wire.begin();

  //Pin modes
  safetyGuardDisable();
  detonateDisable();
    
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SAFETY_LED_PIN, OUTPUT);
  pinMode(RELAY_TEST_PIN, INPUT_PULLUP);
  bothBtn.setHoldTimeout(2000);

  //OLED
  oled.init();
  oled.clear();
  oled.setContrast(200);

  // EEPROM
  if (EEPROM.read(INIT_ADDR) != INIT_KEY){
    EEPROM.write(INIT_ADDR, INIT_KEY);
    EEPROM.put(10, DEFAULT_GUARD_TIMER_VALUE);
    EEPROM.put(20, DEFAULT_SELF_DESTRUCT_TIMER_VALUE);
    EEPROM.put(30, DEFAULT_ACCELERATION);
    EEPROM.put(40, DEMO_MODE);
    EEPROM.put(50, DEBUG_MODE);
    EEPROM.put(ACCEL_OFFSETS_BYTE, offsets);
  }
  EEPROM.get(10, safetyGuardTimeout);
  EEPROM.get(20, selfDestructTimeout);
  EEPROM.get(30, accelerationLimit);
  EEPROM.get(40, demoMode);
  EEPROM.get(50, debugMode);
  EEPROM.get(ACCEL_OFFSETS_BYTE, offsets);

  if(debugMode) Serial.begin(9600);

  //Accelerometer
  mpu.initialize();
  if(mpu.testConnection()){
    if(debugMode) Serial.println(F("MPU6050 check - SUCCESS"));
    drawIntroScreen();
  }
  else{
    if(debugMode) Serial.println(F("MPU6050 check - FAILED"));
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
  delay(1500);

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
  ledTick();
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
      mode = 4;
    }

    if(rightBtn.hasClicks(5)){
      changeDebugMode();
    }
    
    if(rightBtn.hasClicks(10)){
      calibrateAccel();
    }

    if(leftBtn.hasClicks(3)){
      selfTest();
    }

    if(leftBtn.hasClicks(7)){
      changeDemoMode();
    }
    return;
  }

  if(mode == 2){
    if(leftBtn.click()){
      pointer++;
      if (pointer > 4) pointer = 2;
    }

    if(leftBtn.hold()){
      EEPROM.put(10, safetyGuardTimeout);
      EEPROM.put(20, selfDestructTimeout);
      EEPROM.put(30, accelerationLimit);
      mode = 1;
    }

    if(rightBtn.hold()){
      mode = 3;
    }

    exitMenu();
    return;
  }

  if(mode == 3){
    if(leftBtn.click()){
      if(pointer == 2) safetyGuardTimeout -= 10;
      
      else if(pointer == 3) {
        selfDestructTimeout -= 10;
        if(selfDestructTimeout < MIN_SELF_DESTRUCT_TIMER_VALUE) selfDestructTimeout = 0;
      }
  
      else if(pointer == 4) accelerationLimit --;
    }

    if(rightBtn.click()){
      if(pointer == 2) safetyGuardTimeout += 10;
      
      else if(pointer == 3){
        selfDestructTimeout += 10;
        if(selfDestructTimeout < MIN_SELF_DESTRUCT_TIMER_VALUE) selfDestructTimeout = MIN_SELF_DESTRUCT_TIMER_VALUE;
      }
  
      else if(pointer == 4) accelerationLimit ++;
    }

    safetyGuardTimeout = constrain(safetyGuardTimeout, MIN_GUARD_TIMER_VALUE, MAX_GUARD_TIMER_VALUE);
    selfDestructTimeout = constrain(selfDestructTimeout, 0, MAX_SELF_DESTRUCT_TIMER_VALUE);
    accelerationLimit = constrain(accelerationLimit, MIN_ACCELERATION, MAX_ACCELERATION);

    if(leftBtn.hold()){
      mode = 2;
    }

    exitMenu();
    return;
  }

  if(mode >= 4 && mode <= 7){
    if(bothBtn.hold()){
      safetyGuardActiveFlag = false;
      selfDestructActiveFlag = false;
      accelCheckFlag = false;
      detonateDisable();
      safetyGuardDisable();
      mode = 1;
      bothBtn.setHoldTimeout(2000);
      if(!demoMode) drawDefaultScreen();
    }
  }
}


void exitMenu(){
  if(menuExitTimer.tick()){
    EEPROM.get(10, safetyGuardTimeout);
    EEPROM.get(20, selfDestructTimeout);
    EEPROM.get(30, accelerationLimit);
    mode = 1;
  }
}


void operationTick(){
  if(accelCheckFlag){
    if(max_acc >= accelerationLimit){
      if(debugMode){
        Serial.print(F("Acceleration limit ")); Serial.print(accelerationLimit);
        Serial.print(F(" is reached, current acc = ")); Serial.print(max_acc);
        Serial.println(F(", detonating!!!"));
      }
      detonateEnable();
      mode = 7;
      accelCheckFlag = false;
      selfDestructActiveFlag = false;
      bothBtn.setHoldTimeout(2000);
      releaseDetonationTimer.start();
    }

    if(debugMode){
      if(max_acc > debugMaxAccel) debugMaxAccel = max_acc;
    }
  }
  
  if(safetyGuardActiveFlag){
    if(safetyGuardTimeoutCounter == 0){
      if(debugMode) Serial.println(F("Deactivating Safety guard, device armed"));
      safetyGuardDisable();
      safetyGuardActiveFlag = false;
      accelCheckFlag = true;
      mode = 5;
      bothBtn.setHoldTimeout(4000);
    }
  }

  if(selfDestructActiveFlag){
    if(selfDestructTimeoutCounter == 0){
      if(debugMode) Serial.print(F("Self-destruct timeout is reached! "));
      if(safetyGuardActiveFlag){
        if(debugMode) Serial.println(F("Safety guard is still on, detonation blocked!"));
      }
      else{
        if(debugMode) Serial.println(F("Detonating!!!"));
        detonateEnable();
        mode = 6;
        selfDestructActiveFlag = false;
        accelCheckFlag = false;
        bothBtn.setHoldTimeout(2000);
        releaseDetonationTimer.start();
      }
    }
  }

  if(mode >= 6 && mode <= 7){
    if(releaseDetonationTimer.tick()) detonateDisable();
  }
}


void safetyGuardCountdownStart(){
  if(!safetyGuardActiveFlag){
    safetyGuardTimeoutCounter = safetyGuardTimeout;
    if(!demoMode) safetyGuardTimeoutCounter *= 60;
    if(debugMode){
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
  if(selfDestructTimeout == 0){
    if(debugMode){
      Serial.println(F("Self-destruct timer is turned off and not activated"));
      return;
    }
  }
  
  if(!selfDestructActiveFlag){
    selfDestructTimeoutCounter = selfDestructTimeout;
    if(!demoMode) selfDestructTimeoutCounter *= 60;
    if(debugMode){
      Serial.print(F("Activating Self-destruct timer with timeout: "));
      Serial.print(selfDestructTimeoutCounter);
      Serial.println(F(" s"));
    }
    selfDestructActiveFlag = true;
  }
}


void timersCountdown(){
  if(oneSecondTimer.tick()){
    if(safetyGuardActiveFlag){
      if(safetyGuardTimeoutCounter > 0) safetyGuardTimeoutCounter --;
    }

    if(selfDestructActiveFlag){
      if(selfDestructTimeoutCounter > 0) selfDestructTimeoutCounter --;
    }

    if(debugMode) debugMaxAccel = 0; 
  }
}


void checkAccel(){
  if(accelCheckFlag){
    if(accelTimer.tick()){
      mpu.getAcceleration(&ax, &ay, &az);
  
      acc_x = abs(ax / ACC_COEF);
      acc_y = abs(ay / ACC_COEF);
      acc_z = abs(az / ACC_COEF);
  
      max_acc = defineMaxAccel(acc_x, acc_y, acc_z);
  
      if(debugMode){
        Serial.print(acc_x); Serial.print(F("  "));
        Serial.print(acc_y); Serial.print(F("  "));
        Serial.print(acc_z); Serial.print(F("  Max is: "));
        Serial.println(max_acc);
      }
    }
  }
}


int8_t defineMaxAccel(int16_t acc_x, int16_t acc_y, int16_t acc_z){
  return max(max(acc_x, acc_y), acc_z);
}
