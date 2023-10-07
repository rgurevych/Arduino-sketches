//Initiator by R. Gurevych

//---------- Define pins and settings
#define VERSION 0.9                            //Firmware version
#define INIT_ADDR 1023                         //Number of EEPROM first launch check cell
#define INIT_KEY 10                            //First launch key
#define DEBUG_MODE 1                           //Enable debug mode
#define ACCEL_OFFSETS_BYTE 900                 //Nubmer of EEPROM cell where accel offsets are stored
#define BUTTON_PIN 3                           //Button pin
#define DETONATION_PIN 5                       //MOSFET pin
#define ACC_COEF 2048                          //Divider to be used with 16G accelerometer
#define CALIBRATION_BUFFER_SIZE 100            //Buffer size needed for calibration function
#define CALIBRATION_TOLERANCE 500              //What is the calibration tolerance (units)
#define ACCEL_REQUEST_TIMEOUT 50               //Delay between accelerometer request
#define ACCELERATION_LIMIT 10                  //Acceleration limit to detonate

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
uint8_t max_acc;
int16_t ax, ay, az;
int32_t acc_x, acc_y, acc_z;
long offsets[6] = {0,0,0,0,0,0};
bool accelCheckFlag = true;

//---------- Declare timers
TimerMs accelTimer(ACCEL_REQUEST_TIMEOUT, 1);


void setup() {
  Wire.begin();
  Serial.begin(9600);
  detonateDisable();
  pinMode(DETONATION_PIN, OUTPUT);

  // EEPROM
  if (EEPROM.read(INIT_ADDR) != INIT_KEY){
    EEPROM.write(INIT_ADDR, INIT_KEY);
    // EEPROM.put(10, DEFAULT_GUARD_TIMER_VALUE);
    // EEPROM.put(20, DEFAULT_SELF_DESTRUCT_TIMER_VALUE);
    // EEPROM.put(30, DEFAULT_ACCELERATION);
    // EEPROM.put(40, DEMO_MODE);
    EEPROM.put(50, 1);
    EEPROM.put(ACCEL_OFFSETS_BYTE, offsets);
  }
  // EEPROM.get(10, safetyGuardTimeout);
  // EEPROM.get(20, selfDestructTimeout);
  // EEPROM.get(30, accelerationLimit);
  // EEPROM.get(40, demoMode);
  
  
  //Button and accelerometer
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
}


void loop() {
  checkAccel();
  operationTick();
}


void(* resetFunc) (void) = 0;


void operationTick(){
  if(accelCheckFlag){
    if(max_acc >= ACCELERATION_LIMIT){
      if(DEBUG_MODE){
        Serial.print(F("Acceleration limit ")); Serial.print(ACCELERATION_LIMIT);
        Serial.print(F(" is reached, current acc = ")); Serial.print(max_acc);
        Serial.println(F(", detonating!!!"));
      }

      detonateEnable();
      // mode = 7;
      accelCheckFlag = false;
      // selfDestructActiveFlag = false;
      // bothBtn.setHoldTimeout(2000);
    }
  }
  
  // if(safetyGuardActiveFlag){
  //   if(safetyGuardTimeoutCounter == 0){
  //     if(debugMode) Serial.println(F("Deactivating Safety guard, device armed"));
  //     safetyGuardDisable();
  //     safetyGuardActiveFlag = false;
  //     accelCheckFlag = true;
  //     mode = 5;
  //     bothBtn.setHoldTimeout(4000);
  //   }
  // }

  // if(selfDestructActiveFlag){
  //   if(selfDestructTimeoutCounter == 0){
  //     if(debugMode) Serial.print(F("Self-destruct timeout is reached! "));
  //     if(safetyGuardActiveFlag){
  //       if(debugMode) Serial.println(F("Safety guard is still on, detonation blocked!"));
  //     }
  //     else{
  //       if(debugMode) Serial.println(F("Detonating!!!"));
  //       detonateEnable();
  //       mode = 6;
  //       selfDestructActiveFlag = false;
  //       accelCheckFlag = false;
  //       bothBtn.setHoldTimeout(2000);
  //     }
  //   }
  // }
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

