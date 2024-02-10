//Initiator by R. Gurevych
/*
Modes description:
0 - Initial start, to be switched to Idle after defined timeout;
1 - Idle (power on): pin should be in place;
2 - Disarmed mode: safety and self-destroy timers are off;
3 - Safety mode: safety timer is running, self-destroy timer is running (for FPV mode);
4 - Armed mode: reading accelerometer, ready to detonate, self-destroy timer is running (for FPV mode);
5 - Detonate mode: triggered by accelerometer or by self-destroy timer (for FPV mode)
*/

//---------- Define pins and settings
#define VERSION 3.0                            //Firmware version
#define INIT_ADDR 1023                         //Number of EEPROM first launch check cell
#define INIT_KEY 10                            //First launch key
#define DEBUG_MODE 1                           //Enable debug mode
#define ACCEL_OFFSETS_BYTE 900                 //Nubmer of EEPROM cell where accel offsets are stored
#define DETONATION_PIN 17                      //MOSFET pin
#define SAFETY_PIN 16                          //Safety switch pin
#define PWM_PIN 15

#define LED_PIN 14                             //External LED pin
#define ACC_COEF 2048                          //Divider to be used with 16G accelerometer
#define CALIBRATION_BUFFER_SIZE 100            //Buffer size needed for calibration function
#define CALIBRATION_TOLERANCE 500              //What is the calibration tolerance (units)
#define ACCEL_REQUEST_TIMEOUT 20               //Delay between accelerometer request
#define STARTUP_LED_SERIES_INTERVAL 4500          //Delay between LED blinks in Idle mode
#define STARTUP_LED_BLINK_INTERVAL 500            //Duration of LED blink in Idle mode
#define IDLE_LED_SERIES_INTERVAL 2750          //Delay between LED blinks in Idle mode
#define IDLE_LED_BLINK_INTERVAL 250            //Duration of LED blink in Idle mode
#define DISARMED_LED_SERIES_INTERVAL 1250      //Delay between LED blinks in Disarmed mode
#define DISARMED_LED_BLINK_INTERVAL 250        //Duration of LED blink in Disarmed mode
#define SAFETY_LED_SERIES_INTERVAL 350         //Delay between LED blinks in Safety mode
#define SAFETY_LED_BLINK_INTERVAL 150          //Duration of LED blink in Safety mode
#define ARMED_LED_SERIES_INTERVAL 50           //Delay between LED blinks in Armed mode
#define ARMED_LED_BLINK_INTERVAL 50            //Duration of LED blink in Armed mode
#define MODE_CHANGE_INDICATION 100             //How long the LED will be on when mode is changed
#define RELEASE_AFTER_DETONATION 3000          //Timeout after which the detonation relay is released (after detonation)

#define WORK_MODE 0                            //Define the work mode for the device. 0 - FPV, 1 - Bomber

#if WORK_MODE == 0
#define INITIAL_START_TIMEOUT 0                //Initial start timeout before switching to Disarmed mode in minutes
#define SAFETY_TIMEOUT 45                      //Safety timeout in seconds
#define SELF_DESTROY_TIMEOUT 18                //Self-destroy timeout in minutes
#define ACCELERATION_LIMIT 6                   //Acceleration limit to detonate
#else
#define INITIAL_START_TIMEOUT 10               //Initial start timeout before switching to Disarmed mode in minutes
#define SAFETY_TIMEOUT 2                       //Safety timeout in seconds
#define SELF_DESTROY_TIMEOUT 0                 //Self-destroy timeout in minutes
#define ACCELERATION_LIMIT 6                   //Acceleration limit to detonate
#endif


//---------- Include libraries
#include <MPU6050.h>
#include <Wire.h>
#include <EEPROM.h>
#include <EncButton.h>
#include <TimerMs.h>

//---------- Initialize devices
MPU6050 mpu;

//---------- Declare variables
uint8_t max_acc, mode = 0;
int16_t ax, ay, az;
int32_t acc_x, acc_y, acc_z;
int safetyGuardTimeout, safetyGuardTimeoutCounter, selfDestructTimeout, selfDestructTimeoutCounter;
long offsets[6] = {0,0,0,0,0,0};
bool safetyGuardActiveFlag = false, selfDestructActiveFlag = false, accelCheckFlag = false;
bool ledFlag = true, ledBlinkFlag = true, modeChangeFlag = false;

//---------- Declare timers
TimerMs accelTimer(ACCEL_REQUEST_TIMEOUT, 1);
TimerMs oneSecondTimer(1000, 1);
TimerMs blinkTimer(STARTUP_LED_BLINK_INTERVAL, 1, 1);
TimerMs blinkSeriesTimer(STARTUP_LED_SERIES_INTERVAL, 1, 1);
TimerMs modeChangeTimer(MODE_CHANGE_INDICATION, 1);
TimerMs releaseDetonationTimer(RELEASE_AFTER_DETONATION, 0, 1);
TimerMs initialStartTimer(INITIAL_START_TIMEOUT*60000L, 0, 1);


void setup() {
  Wire.begin();
  Serial.begin(9600);
  detonateDisable();
  pinMode(DETONATION_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SAFETY_PIN, INPUT_PULLUP);
  pinMode(PWM_PIN, INPUT);

  Serial.print(F("Firmware version: "));
  Serial.println(VERSION, 2);
  Serial.print(F("Work mode: "));
  if(WORK_MODE == 0) Serial.println(F("FPV")); else Serial.println(F("Bomber"));
  Serial.print(F("Inital unconditional safety timeout: ")); Serial.print(INITIAL_START_TIMEOUT); Serial.println(F(" min"));
  Serial.print(F("Pin pull-out safety timeout: ")); Serial.print(SAFETY_TIMEOUT); Serial.println(F(" sec"));
  Serial.print(F("Self-destroy timeout: ")); Serial.print(SELF_DESTROY_TIMEOUT); Serial.println(F(" min"));
  Serial.print(F("Accelerator limit: ")); Serial.print(ACCELERATION_LIMIT); Serial.println(F(" G"));

  // EEPROM
  if (EEPROM.read(INIT_ADDR) != INIT_KEY){
    EEPROM.write(INIT_ADDR, INIT_KEY);
    EEPROM.put(50, 1);
    EEPROM.put(ACCEL_OFFSETS_BYTE, offsets);
  }
  
  //Initial accelerometer check
  mpu.initialize();
  if(mpu.testConnection()){
    Serial.println(F("MPU6050 accel check - SUCCESS"));
    }
  else{
    Serial.println(F("MPU6050 accel check - FAILED"));
    Serial.println(F("Fix this before proceeding"));
    while (1) {}
    }

  //Initial accelerometer calibration
  if(EEPROM.read(50)){
    Serial.println(F("Send any character to start calibration"));
    delay(100);
    while (1) {
      if (Serial.available() > 0) {
        Serial.read();
        break;
      }
    }
    delay(1000);
    doAccelCalibration();
  }
  else{
    EEPROM.get(ACCEL_OFFSETS_BYTE, offsets);
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
    mpu.setXAccelOffset(offsets[0]);
    mpu.setYAccelOffset(offsets[1]);
    mpu.setZAccelOffset(offsets[2]);
    mpu.setXGyroOffset(offsets[3]);
    mpu.setYGyroOffset(offsets[4]);
    mpu.setZGyroOffset(offsets[5]);
  }
  blinkTimer.start();
  if(INITIAL_START_TIMEOUT != 0){
    initialStartTimer.start();
    if(DEBUG_MODE) {
      Serial.print(F("Starting the initial safety timer, timeout: "));
      Serial.print(INITIAL_START_TIMEOUT);
      Serial.println(F(" m"));
    }
  }
}


void loop() {
  checkAccel();
  timersCountdown();
  operationTick();
  ledTick();
  ledSwitch();
}


void(* resetFunc) (void) = 0;


void safetyGuardCountdownStart(){
  if(!safetyGuardActiveFlag){
    safetyGuardTimeoutCounter = SAFETY_TIMEOUT;
    if(DEBUG_MODE){
      Serial.print(F("Activating Safety guard, timeout: "));
      Serial.print(safetyGuardTimeoutCounter);
      Serial.println(F(" s"));
    }
    safetyGuardActiveFlag = true;
    accelCheckFlag = false;
  }
}


void selfDestructCountdownStart(){
  if(!selfDestructActiveFlag){
    if(SELF_DESTROY_TIMEOUT == 0){
      if(DEBUG_MODE) Serial.println(F("Self-destroy timer won't be activated because it's disabled in the preset"));
      return;
    }
    selfDestructTimeoutCounter = SELF_DESTROY_TIMEOUT * 60;
    if(DEBUG_MODE){
      Serial.print(F("Activating Self-destroy timer with timeout: "));
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

    if(DEBUG_MODE) {
      if(safetyGuardActiveFlag && safetyGuardTimeoutCounter == 0) Serial.println(F("Safety guard timer reached 0"));
      if(selfDestructActiveFlag && selfDestructTimeoutCounter == 0) Serial.println(F("Self-destroy timer reached 0"));
    }
  }
}


void operationTick(){
  if(mode == 0){
    if(INITIAL_START_TIMEOUT == 0 || initialStartTimer.tick()) {
      switchToIdleMode();
      return;
    }
  }
  
  if(mode == 1){
    if(digitalRead(SAFETY_PIN)){
      if(DEBUG_MODE) Serial.println(F("Safety pin missing, switch to disarmed mode not possible"));
      return;
    }
    switchToDisarmedMode();
    return;
  }

  if(mode == 2){
    if(digitalRead(SAFETY_PIN)){
      swtichToSafetyMode();
      return;
    }
  }

  if(mode == 3 && safetyGuardActiveFlag){
    if(safetyGuardTimeoutCounter == 0){
      safetyGuardActiveFlag = false;
      switchToArmedMode();
      return;
     }
  }

  if(mode == 4 && accelCheckFlag){
    if(max_acc >= ACCELERATION_LIMIT){
      if(DEBUG_MODE){
        Serial.print(F("Acceleration limit ")); Serial.print(ACCELERATION_LIMIT);
        Serial.print(F(" is reached, current acc = ")); Serial.println(max_acc);
      }
      switchToDetonateMode();
      return;
    }
  }
  
  if(mode != 1 && selfDestructActiveFlag){
    if(selfDestructTimeoutCounter == 0){
      switchToDetonateMode();
      return;
    }
  }

  if(mode == 5){
    if(releaseDetonationTimer.tick()) {
      switchToReleaseAfterDetonation();
    }
  }
}

void switchToIdleMode() {
  if(DEBUG_MODE) {
    Serial.println(F("Switching to Idle mode"));
  }
  blinkTimer.setTime(IDLE_LED_BLINK_INTERVAL);
  blinkSeriesTimer.setTime(IDLE_LED_SERIES_INTERVAL);
  mode = 1;
  modeChangeIndication();
}

void switchToDisarmedMode() {
  if(DEBUG_MODE) {
    Serial.println(F("Switching to Disarmed mode"));
  }
  blinkTimer.setTime(DISARMED_LED_BLINK_INTERVAL);
  blinkSeriesTimer.setTime(DISARMED_LED_SERIES_INTERVAL);
  mode = 2;
  modeChangeIndication();
}

void swtichToSafetyMode() {
  if(DEBUG_MODE) {
    Serial.println(F("Switching to Safety mode"));
  }
  safetyGuardCountdownStart();
  selfDestructCountdownStart();
  blinkTimer.setTime(SAFETY_LED_BLINK_INTERVAL);
  blinkSeriesTimer.setTime(SAFETY_LED_SERIES_INTERVAL);
  mode = 3;
  modeChangeIndication();
}

void switchToArmedMode() {
  if(DEBUG_MODE) {
    Serial.println(F("Switching to Armed mode"));
  }
  accelCheckFlag = true;
  blinkTimer.setTime(ARMED_LED_BLINK_INTERVAL);
  blinkSeriesTimer.setTime(ARMED_LED_SERIES_INTERVAL);
  mode = 4;
  modeChangeIndication();
}

void switchToDetonateMode() {
  if(DEBUG_MODE) {
    Serial.println(F("Switching to Detonate mode"));
  }
  detonateEnable();
  selfDestructActiveFlag = false;
  accelCheckFlag = false;
  blinkTimer.setTime(STARTUP_LED_BLINK_INTERVAL);
  blinkSeriesTimer.setTime(STARTUP_LED_SERIES_INTERVAL);
  mode = 5;
  releaseDetonationTimer.start();
}

void switchToReleaseAfterDetonation() {
  if(DEBUG_MODE) {
    Serial.println(F("Releasing after detonation and restart"));
  }
  detonateDisable();
  resetFunc();
}

void checkAccel(){
  if(accelCheckFlag){
    if(accelTimer.tick()){
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

int8_t defineMaxAccel(int16_t acc_x, int16_t acc_y, int16_t acc_z){
  return max(max(acc_x, acc_y), acc_z);
}


void detonateEnable(){
  if(!digitalRead(SAFETY_PIN)){
    return;
  }
  digitalWrite(DETONATION_PIN, HIGH);
}


void detonateDisable(){
  digitalWrite(DETONATION_PIN, LOW);
}


void modeChangeIndication(){
  modeChangeTimer.start();
  modeChangeFlag = true;
}


void ledTick(){
  if(mode == 5){
    ledFlag = true;
    return;
  }

  if(modeChangeFlag){
    if(modeChangeTimer.tick()){
      ledFlag = false;
      modeChangeFlag = false;
      blinkSeriesTimer.start();
    }
    else{
      ledFlag = true;
    }
    return;
  }
  
  if(blinkSeriesTimer.tick()){
    ledBlinkFlag = true;
    ledFlag = true;
    blinkTimer.start();
    uint16_t pwm_value = getPWM();
    Serial.print(F("PWM Value = ")); Serial.println(pwm_value);
  }

  if(ledBlinkFlag){
    if(blinkTimer.tick()){
      ledFlag = false;
      blinkSeriesTimer.start();
      ledBlinkFlag = false;
    }
  }
}


void ledSwitch() {
  digitalWrite(LED_PIN, ledFlag);
}


uint16_t getPWM() {
  uint16_t highTime = pulseIn(PWM_PIN, HIGH, 50000UL);  // 50 millisecond timeout
  return highTime;
}
