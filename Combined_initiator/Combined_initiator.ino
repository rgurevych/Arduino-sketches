/*Combined initator by Rostyslav Gurevych
Mode description:
1 - Idle
2 - Settings
3 - Change value
4 - Disarmed
5 - Safe waiting for PWM
6 - Safe, active
7 - Armed
8 - Detonate
*/

//---------- Define pins and settings
#define VERSION 3.30                           //Firmware version
#define REMOTE_CONTROL 1                       //Arming is done via Remote control
#define INIT_ADDR 1023                         //Number of EEPROM first launch check cell
#define INIT_KEY 10                            //First launch key
#define INIT_CALIBRATION_ADDR 1022             //Number of EEPROM initial calibration check cell
#define INIT_CALIBRATION_KEY 20                //Initial calibration key
#define ACCEL_OFFSETS_BYTE 900                 //Nubmer of EEPROM cell where accel offsets are stored
#define BUTTON_1_PIN 17                        //Button 1 pin
#define BUTTON_2_PIN 16                        //Button 2 pin
#define RELAY_1_PIN 6                          //Safety guard relay pin (relay 1)
#define RELAY_2_PIN 7                          //Detonation relay pin (relay 2)
#define SAFETY_LED_PIN 8                       //Safety guard LED pin
#define RELAY_TEST_PIN 9                       //Relay test pin (for self-test)
#define MIN_GUARD_TIMER_VALUE 1                //Minimum safety guard timer value (in minutes)
#define MAX_GUARD_TIMER_VALUE 60               //Maximum safety guard timer value (in minutes)
#define DEFAULT_GUARD_TIMER_VALUE 20           //Default safety guard timer value on startup (in minutes)
#define MIN_SELF_DESTRUCT_TIMER_VALUE 60       //Minimum self-destruction timer value (in minutes)
#define MAX_SELF_DESTRUCT_TIMER_VALUE 600      //Maximum self-destruction timer value (in minutes)
#define DEFAULT_SELF_DESTRUCT_TIMER_VALUE 90   //Default self-destruction timer value on startup (in minutes)
#define MIN_ACCELERATION 5                     //Minimum acceleration limit
#define MAX_ACCELERATION 15                    //Maximum acceleration limit
#define DEFAULT_ACCELERATION 12                //Default acceleration limit value
#define BUTTON_TIMEOUT 20000                   //Timeout after which device will return to idle mode from settings (without saving)
#define DEMO_MODE 1                            //Initially Demo mode enabled (all times are reduced to seconds)
#define DEBUG_MODE 1                           //Initially Debug mode enabled (Serial is activated and used for debugging)
#define ACC_COEF 2048                          //Divider to be used with 16G accelerometer
#define CALIBRATION_BUFFER_SIZE 100            //Buffer size needed for calibration function
#define CALIBRATION_TOLERANCE 500              //What is the calibration tolerance (units)
#define ACCEL_REQUEST_TIMEOUT 5                //Delay between accelerometer request
#define RELEASE_AFTER_DETONATION 3000          //Timeout after which the detonation relay is released (after detonation)
#define LED_BLINK_DURATION 200                 //Duration of LED blinks
#define LED_BLINK_INTERVAL 800                 //Interval between LED blinks

#define PWM_REQUEST_TIMEOUT 100                //Delay between PWM checks
#define PWM_PIN 2                              //PWM remote pin
#define SAFETY_PWM 2000                        //PWM value which enables safety mode
#define DISARMED_TIMEOUT 10                    //Timeout after which the disarmed PWM mode is switched off

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
TimerMs PWMCheckTimer(PWM_REQUEST_TIMEOUT, 1);


//---------- Variables
bool disarmedActiveFlag = false, safetyGuardActiveFlag = false, selfDestructActiveFlag = false, accelCheckFlag = false;
bool blinkFlag = true, ledFlag = true, ledBlinkFlag = true, detonateByTimerFlag = false, detonateByAccelFlag = false;
bool demoMode, debugMode, PWMremote;
uint8_t max_acc, accelerationLimit, debugMaxAccel = 0;
int16_t safetyGuardTimeout, safetyGuardTimeoutCounter, selfDestructTimeout, selfDestructTimeoutCounter, disarmedTimeoutCounter;
uint8_t mode, oldMode = 0, pointer = 2;
int32_t acc_x, acc_y, acc_z;
int16_t ax, ay, az;
long offsets[6] = {0,0,0,0,0,0};
int PWMvalue;

void setup() {
  Wire.begin();

  // Set both executive outputs to 0
  safetyGuardDisable();
  detonateDisable();
  
  // Pin modes  
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SAFETY_LED_PIN, OUTPUT);
  pinMode(RELAY_TEST_PIN, INPUT_PULLUP);
  pinMode(PWM_PIN, INPUT_PULLUP);

  // OLED
  oled.init();
  oled.clear();
  oled.setContrast(200);

  // EEPROM
  if (EEPROM.read(INIT_ADDR) != INIT_KEY){
    EEPROM.put(INIT_ADDR, INIT_KEY);
    EEPROM.put(10, DEFAULT_GUARD_TIMER_VALUE);
    EEPROM.put(20, DEFAULT_SELF_DESTRUCT_TIMER_VALUE);
    EEPROM.put(30, DEFAULT_ACCELERATION);
    EEPROM.put(40, DEMO_MODE);
    EEPROM.put(50, DEBUG_MODE);
    EEPROM.put(60, REMOTE_CONTROL);
  }
  EEPROM.get(10, safetyGuardTimeout);
  EEPROM.get(20, selfDestructTimeout);
  EEPROM.get(30, accelerationLimit);
  EEPROM.get(40, demoMode);
  EEPROM.get(50, debugMode);
  EEPROM.get(60, PWMremote);

  // Open Serial output in Debug mode
  if(debugMode) Serial.begin(9600);

  //Initialize accelerometer, show error screen if fail
  mpu.initialize();
  if(mpu.testConnection()){
    if(debugMode) Serial.println(F("MPU6050 check - SUCCESS"));
  }
  else{
    if(debugMode) Serial.println(F("MPU6050 check - FAILED"));
    drawErrorIntroScreen();
    while(1) {delay(1000);}
  }

  if(PWMremote){
    if(debugMode) Serial.println(F("PWM remote enabled"));
  }

  
  // Switch to calibration when first launch happens
  if (EEPROM.read(INIT_CALIBRATION_ADDR) != INIT_CALIBRATION_KEY){
    calibrateAccel();
  }
  else{
    drawIntroScreen();
    EEPROM.get(ACCEL_OFFSETS_BYTE, offsets);
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
    mpu.setXAccelOffset(offsets[0]);
    mpu.setYAccelOffset(offsets[1]);
    mpu.setZAccelOffset(offsets[2]);
    mpu.setXGyroOffset(offsets[3]);
    mpu.setYGyroOffset(offsets[4]);
    mpu.setZGyroOffset(offsets[5]);

    delay(1500);
    switchToIdleMode();
  }  
}


void loop() {
  buttonTick();
  timersCountdown();
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
      pointer = 2;
      switchToSettingsMode();
    }
    
    if(bothBtn.hold()){
      if(PWMremote) {
        switchToDisarmedMode();
      }
      else {
        switchToSafetyMode();
      }
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

    if(leftBtn.hasClicks(10)){
      changePWMMode();
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
      switchToIdleMode();
    }

    if(rightBtn.hold()){
      switchToChangeValueMode();
    }

    exitMenu();
    return;
  }

  if(mode == 3){
    if(leftBtn.click()){
      if(pointer == 2) safetyGuardTimeout -= 5;
      
      else if(pointer == 3) {
        selfDestructTimeout -= 10;
        if(selfDestructTimeout < MIN_SELF_DESTRUCT_TIMER_VALUE) selfDestructTimeout = 0;
      }
  
      else if(pointer == 4) accelerationLimit --;
    }

    if(rightBtn.click()){
      if(pointer == 2){
        if(safetyGuardTimeout <= 1) safetyGuardTimeout = 5;
        else safetyGuardTimeout += 5;
      }
      
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
      switchToSettingsMode();
    }

    exitMenu();
    return;
  }

  if(mode >= 4 && mode <= 8){
    if(bothBtn.hold()){
      switchToIdleMode();
    }
  }
}

void switchToIdleMode() {
  if(debugMode) {
    Serial.println(F("Switching to Idle mode"));
  }
  safetyGuardActiveFlag = false;
  selfDestructActiveFlag = false;
  accelCheckFlag = false;
  detonateDisable();
  safetyGuardDisable();
  bothBtn.setHoldTimeout(2000);
  drawDefaultScreen();
  mode = 1;
  changeMode();
}


void switchToSettingsMode() {
  if(debugMode) {
    Serial.println(F("Switching to Settings mode"));
  }
  mode = 2;
  changeMode();
}


void switchToChangeValueMode() {
  if(debugMode) {
    Serial.println(F("Switching to Change Value mode"));
  }
  mode = 3;
  changeMode();
}


void switchToDisarmedMode() {
  if(debugMode) {
    Serial.println(F("Switching to Disarmed mode"));
  }
  disarmedCountdownStart();
  mode = 4;
  changeMode();
}


void switchToPWMSafetyMode() {
  if(debugMode) {
    Serial.println(F("Switching to PWM Safety mode"));
  }
  disarmedActiveFlag = false;
  safetyGuardEnable();
  mode = 5;
  changeMode();
}


void switchToSafetyMode() {
  if(debugMode) {
    Serial.println(F("Switching to Safety mode"));
  }
  safetyGuardCountdownStart();
  selfDestructCountdownStart();
  mode = 6;
  changeMode();
}


void switchToArmedMode() {
  if(debugMode) Serial.println(F("Switching to Armed mode"));
  detonateByTimerFlag = false;
  detonateByAccelFlag = false;
  safetyGuardActiveFlag = false;
  accelCheckFlag = true;
  safetyGuardDisable();
  bothBtn.setHoldTimeout(4000);
  mode = 7;
  changeMode();
}


void switchToDetonateMode() {
  if(debugMode) Serial.println(F("Switching to Detonate mode"));
  detonateEnable();
  accelCheckFlag = false;
  selfDestructActiveFlag = false;
  bothBtn.setHoldTimeout(2000);
  releaseDetonationTimer.start();
  mode = 8;
  changeMode();
}


void exitMenu(){
  if(menuExitTimer.tick()){
    EEPROM.get(10, safetyGuardTimeout);
    EEPROM.get(20, selfDestructTimeout);
    EEPROM.get(30, accelerationLimit);
    switchToIdleMode();
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
      detonateByAccelFlag = true;
      switchToDetonateMode();
    }

    if(debugMode){
      if(max_acc > debugMaxAccel) debugMaxAccel = max_acc;
    }
  }
  
  if(disarmedActiveFlag){
    if(disarmedTimeoutCounter == 0){
      switchToPWMSafetyMode();
    }
  }

  if(safetyGuardActiveFlag){
    if(safetyGuardTimeoutCounter == 0){
      switchToArmedMode();
    }
  }

  if(selfDestructActiveFlag){
    if(selfDestructTimeoutCounter == 0){
      if(debugMode) Serial.print(F("Self-destruct timeout is reached! "));
      if(safetyGuardActiveFlag){
        if(debugMode) Serial.println(F("Safety guard is still on, detonation blocked!"));
      }
      else{
        detonateByTimerFlag = true;
        switchToDetonateMode();
      }
    }
  }

  if(mode == 8){
    if(releaseDetonationTimer.tick()) {
      if(debugMode) Serial.println(F("Releasing fire pin"));
      detonateDisable();
    }
  }
  
  if(mode == 5){
    getPWM();
    if(abs(PWMvalue - SAFETY_PWM) < 50) {
      switchToSafetyMode();
      return;
    }
  }
}

void disarmedCountdownStart(){
  if(!disarmedActiveFlag){
    disarmedTimeoutCounter = DISARMED_TIMEOUT;
    if(!demoMode) disarmedTimeoutCounter *= 60;
    if(debugMode){
      Serial.print(F("Activating Disarmed mode, PWM ignored, timeout: "));
      Serial.print(disarmedTimeoutCounter);
      Serial.println(F(" s"));
    }
    safetyGuardEnable();
    disarmedActiveFlag = true;
    accelCheckFlag = false;
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

    if(disarmedActiveFlag){
      if(disarmedTimeoutCounter > 0) disarmedTimeoutCounter --;
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
  
      // if(debugMode){
      //   Serial.print(acc_x); Serial.print(F("  "));
      //   Serial.print(acc_y); Serial.print(F("  "));
      //   Serial.print(acc_z); Serial.print(F("  Max is: "));
      //   Serial.println(max_acc);
      // }
    }
  }
}


int8_t defineMaxAccel(int16_t acc_x, int16_t acc_y, int16_t acc_z){
  return max(max(acc_x, acc_y), acc_z);
}
