/*Useless box by R. Gurevych
 * 
 */

// Includes
#include <ServoSmooth.h>
#include <GyverTimer.h>


// Settings
#define MAX_HAND_SERVO 20                //Max allowed angle of hand servo
#define MAX_BOX_SERVO 150
#define DEFAULT_HAND_SERVO_SPEED 180
#define DEFAULT_BOX_SERVO_SPEED 100


// Pins
#define SERVO_HAND_PIN 5    //Hand pin servo
#define SERVO_BOX_PIN 6     //Box lid pin servo
#define SWITCH_PIN 2        //Switch pin

#define BUZZ_PIN 7    // Buzzer
#define LED_PIN 13     // LED


// Timers
GTimer delayTimer(MS);                //Delay timer used in each particular step
GTimer switchDelayTimer(MS, 1000);    //Timer used between enabling switch and starting operation


// Objects
ServoSmooth handServo;
ServoSmooth boxServo;


// Variables
boolean led_flag, hand_servo_state, box_servo_state;
boolean operate_flag = false;
byte operation_step = 0;
byte mode = 0;

uint32_t myTimer;


void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  led_flag = false;

  handServo.attach(SERVO_HAND_PIN, 180);
  handServo.smoothStart();
  handServo.setTargetDeg(180);
  handServo.setAccel(0);
  handServo.setSpeed(DEFAULT_HAND_SERVO_SPEED);

  boxServo.attach(SERVO_BOX_PIN, 180);
  boxServo.smoothStart();
  boxServo.setTargetDeg(180);
  boxServo.setAccel(0);
  boxServo.setSpeed(DEFAULT_BOX_SERVO_SPEED);
}


void loop() {
  hand_servo_state = handServo.tick();
  box_servo_state = boxServo.tick();
  check_switch();
  operate();
}


void check_switch(){
  if (digitalRead(SWITCH_PIN) == HIGH && !operate_flag){
    operate_flag = true;
    switchDelayTimer.start();
  }
  if (operate_flag) digitalWrite(LED_PIN, HIGH);
  else digitalWrite(LED_PIN, LOW);
}


void operate(){
  if (operate_flag){
    if (mode == 0) mode_0();
    else if (mode == 1) mode_1();
  }
}


void mode_0(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(2500);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      handServo.setSpeed(120);
      delayTimer.setTimeout(2000);
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }

    if (operation_step == 2 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(2000);
      handServo.setTargetDeg(180);
      handServo.tick();
    }

    if (operation_step == 3 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(800);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 4 && delayTimer.isReady()){
      handServo.setSpeed(DEFAULT_HAND_SERVO_SPEED);
      operation_step = 0;
      operate_flag = false;
    }
}


void mode_1(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(1500);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }

    if (operation_step == 2 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      handServo.setTargetDeg(180);
      handServo.tick();
    }

    if (operation_step == 3 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 4 && delayTimer.isReady()){
      operation_step = 0;
      operate_flag = false;
    }
}
