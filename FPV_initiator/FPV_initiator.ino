//Initiator by R. Gurevych

//---------- Define pins and settings
#define VERSION 0.9                            //Firmware version
#define INIT_ADDR 1023                         //Number of EEPROM first launch check cell
#define INIT_KEY 10                            //First launch key
#define DEBUG_MODE 1                           //Enable debug mode
#define ACCEL_OFFSETS_BYTE 900                 //Nubmer of EEPROM cell where accel offsets are stored
#define BUTTON_PIN 3                           //Button pin
#define DETONATION_PIN 5                       //MOSFET pin
#define SAFETY_PIN 7                           //Safety switch pin
#define ACC_COEF 2048                          //Divider to be used with 16G accelerometer
#define CALIBRATION_BUFFER_SIZE 100            //Buffer size needed for calibration function
#define CALIBRATION_TOLERANCE 500              //What is the calibration tolerance (units)
#define ACCEL_REQUEST_TIMEOUT 25               //Delay between accelerometer request
#define IDLE_LED_SERIES_INTERVAL 2500          //Delay between LED blinks in Idle mode
#define IDLE_LED_BLINK_INTERVAL 500            //Duration of LED blink in Idle mode
#define DISARMED_LED_SERIES_INTERVAL 500       //Delay between LED blinks in Disarmed mode
#define DISARMED_LED_BLINK_INTERVAL 500        //Duration of LED blink in Disarmed mode
#define SAFETY_LED_SERIES_INTERVAL 200         //Delay between LED blinks in Safety mode
#define SAFETY_LED_BLINK_INTERVAL 200          //Duration of LED blink in Safety mode
#define ARMED_LED_SERIES_INTERVAL 100          //Delay between LED blinks in Armed mode
#define ARMED_LED_BLINK_INTERVAL 100           //Duration of LED blink in Armed mode
#define MODE_CHANGE_INDICATION 1000            //How long the LED will be on when mode is changed

#define SAFETY_TIMEOUT 90                      //Safety timeout in seconds
#define SELF_DESTROY_TIMEOUT 13                //Self-destroy timeout in minutes
#define ACCELERATION_LIMIT 5                   //Acceleration limit to detonate

//---------- Include libraries
#include <MPU6050.h>
#include <Wire.h>
#include <EEPROM.h>
#include <EncButton.h>
#include <TimerMs.h>

//---------- Initialize devices
Button btn(BUTTON_PIN, INPUT_PULLUP);
MPU6050 mpu;

//---------- Declare variables
uint8_t max_acc, mode = 1;
int16_t ax, ay, az;
int32_t acc_x, acc_y, acc_z;
int safetyGuardTimeout, safetyGuardTimeoutCounter, selfDestructTimeout, selfDestructTimeoutCounter;
long offsets[6] = {0,0,0,0,0,0};
bool safetyGuardActiveFlag = false, selfDestructActiveFlag = false, accelCheckFlag = false;
bool ledFlag = false, ledBlinkFlag = false, modeChangeFlag = false;

//---------- Declare timers
TimerMs accelTimer(ACCEL_REQUEST_TIMEOUT, 1);
TimerMs oneSecondTimer(1000, 1);
TimerMs blinkTimer(IDLE_LED_BLINK_INTERVAL, 1, 1);
TimerMs blinkSeriesTimer(IDLE_LED_SERIES_INTERVAL, 1, 1);
TimerMs modeChangeTimer(MODE_CHANGE_INDICATION, 1);


void setup() {
  Wire.begin();
  Serial.begin(9600);
  detonateDisable();
  pinMode(DETONATION_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SAFETY_PIN, INPUT_PULLUP);

  // EEPROM
  if (EEPROM.read(INIT_ADDR) != INIT_KEY){
    EEPROM.write(INIT_ADDR, INIT_KEY);
    EEPROM.put(50, 1);
    EEPROM.put(ACCEL_OFFSETS_BYTE, offsets);
  }
  
  //Button timeout
  btn.setHoldTimeout(2000);

  //Initial accelerometer check
  mpu.initialize();
  if(mpu.testConnection()){
    Serial.println(F("MPU6050 check - SUCCESS"));
    }
  else{
    Serial.println(F("MPU6050 check - FAILED"));
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
  blinkSeriesTimer.start();
}


void loop() {
  buttonTick();
  checkAccel();
  timersCountdown();
  operationTick();
  ledTick();
  ledSwitch();
}


void(* resetFunc) (void) = 0;


void buttonTick(){
  btn.tick();

  if(mode == 1){
    if(btn.hold()){
      if(digitalRead(SAFETY_PIN)){
        if(DEBUG_MODE) Serial.println(F("Safety pin missing, disarmed mode not possible"));
        return;
      }
      if(DEBUG_MODE) Serial.println(F("Disarmed mode enabled"));
      blinkTimer.setTime(DISARMED_LED_BLINK_INTERVAL);
      blinkSeriesTimer.setTime(DISARMED_LED_SERIES_INTERVAL);
      btn.setHoldTimeout(4000);
      mode = 2;
      modeChangeIndication();
    }
    return;
  }

  if(mode >= 2){
    if(btn.hold()){
      safetyGuardActiveFlag = false;
      selfDestructActiveFlag = false;
      accelCheckFlag = false;
      detonateDisable();
      mode = 1;
      if(DEBUG_MODE) Serial.println(F("Idle mode enabled"));
      btn.setHoldTimeout(2000);
      blinkTimer.setTime(IDLE_LED_BLINK_INTERVAL);
      blinkSeriesTimer.setTime(IDLE_LED_SERIES_INTERVAL);
      modeChangeIndication();
    }
  }
}


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
  }
}


void operationTick(){
  if(mode == 2){
    if(digitalRead(SAFETY_PIN)){
      safetyGuardCountdownStart();
      selfDestructCountdownStart();
      if(DEBUG_MODE) Serial.println(F("Safety mode enabled"));
      blinkTimer.setTime(SAFETY_LED_BLINK_INTERVAL);
      blinkSeriesTimer.setTime(SAFETY_LED_SERIES_INTERVAL);
      mode = 3;
      modeChangeIndication();
    }
  }

  if(accelCheckFlag){
    if(max_acc >= ACCELERATION_LIMIT){
      if(DEBUG_MODE){
        Serial.print(F("Acceleration limit ")); Serial.print(ACCELERATION_LIMIT);
        Serial.print(F(" is reached, current acc = ")); Serial.print(max_acc);
        Serial.println(F(", detonating!!!"));
      }

      detonateEnable();
      mode = 5;
      selfDestructActiveFlag = false;
      accelCheckFlag = false;
      blinkTimer.setTime(IDLE_LED_BLINK_INTERVAL);
      blinkSeriesTimer.setTime(IDLE_LED_SERIES_INTERVAL);
    }
  }
  
  if(safetyGuardActiveFlag){
    if(safetyGuardTimeoutCounter == 0){
      if(DEBUG_MODE) Serial.println(F("Safety timeout reached, armed mode enabled"));
      safetyGuardActiveFlag = false;
      accelCheckFlag = true;
      mode = 4;
      blinkTimer.setTime(ARMED_LED_BLINK_INTERVAL);
      blinkSeriesTimer.setTime(ARMED_LED_SERIES_INTERVAL);
     }
  }

  if(selfDestructActiveFlag){
    if(selfDestructTimeoutCounter == 0){
      if(DEBUG_MODE) Serial.print(F("Self-destruct timeout is reached! Detonating!"));
      detonateEnable();
      mode = 5;
      selfDestructActiveFlag = false;
      accelCheckFlag = false;
      blinkTimer.setTime(IDLE_LED_BLINK_INTERVAL);
      blinkSeriesTimer.setTime(IDLE_LED_SERIES_INTERVAL);
    }
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
  digitalWrite(LED_BUILTIN, ledFlag);
}
