/*Dawn clock
 * 
 */

// Includes
#include <GyverEncoder.h>
#include <GyverTM1637.h>
//#include <Servo.h>
#include <ServoSmooth.h>
#include <GyverTimer.h>
#include <CyberLib.h>
#include <GyverButton.h>



// Settings
#define ENCODER_TYPE 1    // 1 or 2


// Pins
#define CLKe 9        // encoder S1
#define DTe 8         // encoder S2
#define SWe 10        // encoder Key

#define BUZZ_PIN 7    // пищалка (по желанию)
#define LED_PIN 13     // светодиод индикатор
#define SWITCH_PIN 2  // Switch

#define SERVO_HAND_PIN 5
#define SERVO_BOX_PIN 6


// Timers
GTimer delayTimer(MS, 2000);


// Objects
Encoder enc(CLKe, DTe, SWe);

ServoSmooth handServo;
ServoSmooth boxServo;
//Servo handServo;

// Variables
int motor = 0;
boolean led_flag, hand_servo_state, box_servo_state;
boolean operate_flag = false;
byte led_brightness = 25;
byte operation_step = 0;

uint32_t myTimer;


void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  led_flag = false;

  enc.setType(ENCODER_TYPE);

  handServo.attach(SERVO_HAND_PIN, 180);
  handServo.smoothStart();
  handServo.setTargetDeg(180);
  handServo.setAccel(0);
  handServo.setSpeed(300);

  boxServo.attach(SERVO_BOX_PIN, 180);
  boxServo.smoothStart();
  boxServo.setTargetDeg(180);
  boxServo.setAccel(0);
  boxServo.setSpeed(300);
}


void loop() {
  hand_servo_state = handServo.tick();
  box_servo_state = boxServo.tick();
  enc.tick();
  update_motor();
  check_switch();
  operate();
}


void update_motor(){
  if (enc.isRight()) motor++;
  if (enc.isRightH()) motor += 5;

  if (enc.isLeft()) motor--;
  if (enc.isLeftH()) motor -= 5;

  motor = constrain(motor, 0, 180);
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
      delayTimer.setTimeout(2000);
      boxServo.setTargetDeg(150);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      handServo.setTargetDeg(12);
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
