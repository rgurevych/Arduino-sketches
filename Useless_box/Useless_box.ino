/*Useless box by R. Gurevych
 * 
 */

// Includes
#include <ServoSmooth.h>
#include <GyverTimer.h>


// Settings


// Pins
#define SERVO_HAND_PIN 5    //Hand pin servo
#define SERVO_BOX_PIN 6     //Box lid pin servo
#define SWITCH_PIN 2        //Switch pin

#define BUZZ_PIN 7    // Buzzer
#define LED_PIN 13     // LED


// Timers
GTimer delayTimer(MS);


// Objects
ServoSmooth handServo;
ServoSmooth boxServo;


// Variables
boolean led_flag, hand_servo_state, box_servo_state;
boolean operate_flag = false;
byte operation_step = 0;
byte max_hand_servo = 20;

uint32_t myTimer;


void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  led_flag = false;

  handServo.attach(SERVO_HAND_PIN, 180);
  handServo.smoothStart();
  handServo.setTargetDeg(180);
  handServo.setAccel(0);
  handServo.setSpeed(200);

  boxServo.attach(SERVO_BOX_PIN, 180);
  boxServo.smoothStart();
  boxServo.setTargetDeg(180);
  boxServo.setAccel(0);
  boxServo.setSpeed(200);
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
  }
  if (operate_flag) digitalWrite(LED_PIN, HIGH);
  else digitalWrite(LED_PIN, LOW);
}


void operate(){
  if (operate_flag){
    if (operation_step == 0){
      operation_step ++;
      delayTimer.setTimeout(1500);
      boxServo.setTargetDeg(150);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      handServo.setTargetDeg(max_hand_servo);
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
}
