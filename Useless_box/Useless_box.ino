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
GTimer switchDelayTimer(MS, 500);    //Timer used between enabling switch and starting operation


// Objects
ServoSmooth handServo;
ServoSmooth boxServo;


// Variables
boolean led_flag, hand_servo_state, box_servo_state;
boolean operate_flag = false;
byte operation_step = 0;
byte mode = 8;

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
    else if (mode == 2) mode_2();
    else if (mode == 3) mode_3();
    else if (mode == 4) mode_4();
    else if (mode == 5) mode_5();
    else if (mode == 6) mode_6();
    else if (mode == 7) mode_7();
    else if (mode == 8) mode_8();
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
      delayTimer.setTimeout(1200);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(1500);
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }

    if (operation_step == 2 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(1300);
      handServo.setTargetDeg(180);
      handServo.tick();
    }

    if (operation_step == 3 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(2500);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 4 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(3000);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 5 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(800);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 6 && delayTimer.isReady()){
      operation_step = 0;
      operate_flag = false;
    }
}


void mode_2(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      operation_step ++;
      boxServo.setAccel(0.3);
      delayTimer.setTimeout(1100);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      handServo.setAccel(0.3);
      delayTimer.setTimeout(1500);
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
      delayTimer.setTimeout(1000);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 4 && delayTimer.isReady()){
      operation_step = 0;
      operate_flag = false;
      boxServo.setAccel(0);
      handServo.setAccel(0);
    }
}


void mode_3(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      operation_step ++;
      boxServo.setSpeed(8);
      delayTimer.setTimeout(3500);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      handServo.setSpeed(200);
      delayTimer.setTimeout(900);
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }
    
    if (operation_step == 2 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(900);
      handServo.setTargetDeg(180);
      handServo.tick();
    }

    if (operation_step == 3 && delayTimer.isReady()){
      operation_step ++;
      boxServo.setSpeed(10);
      delayTimer.setTimeout(3500);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 4 && delayTimer.isReady()){
      boxServo.setSpeed(DEFAULT_BOX_SERVO_SPEED);
      handServo.setSpeed(DEFAULT_HAND_SERVO_SPEED);
      operation_step = 0;
      operate_flag = false;
    }
}


void mode_4(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(500);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      handServo.setSpeed(40);
      delayTimer.setTimeout(4000);
      handServo.setTargetDeg(MAX_HAND_SERVO + 15);
      handServo.tick();
    }

    if (operation_step == 2 && delayTimer.isReady()){
      operation_step ++;
      handServo.setSpeed(200);
      delayTimer.setTimeout(500);
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }
    
    if (operation_step == 3 && delayTimer.isReady()){
      operation_step ++;
      handServo.setSpeed(50);
      delayTimer.setTimeout(2600);
      handServo.setTargetDeg(180);
      handServo.tick();
    }

    if (operation_step == 4 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(500);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 5 && delayTimer.isReady()){
      handServo.setSpeed(DEFAULT_HAND_SERVO_SPEED);
      boxServo.setSpeed(DEFAULT_BOX_SERVO_SPEED);
      operation_step = 0;
      operate_flag = false;
    }
}


void mode_5(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      operation_step ++;
      boxServo.setSpeed(250);
      delayTimer.setTimeout(300);
      boxServo.setTargetDeg(MAX_BOX_SERVO + 10);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }

    if (operation_step == 2 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      boxServo.setTargetDeg(MAX_BOX_SERVO + 10);
      boxServo.tick();
    }

    if (operation_step == 3 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }

    if (operation_step == 4 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      boxServo.setTargetDeg(MAX_BOX_SERVO + 10);
      boxServo.tick();
    }

    if (operation_step == 5 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }

    if (operation_step == 6 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }
    
    if (operation_step == 7 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(1200);
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }

    if (operation_step == 8 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(300);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }

    if (operation_step == 9 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }
    
    if (operation_step == 10 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(1200);
      handServo.setTargetDeg(180);
      handServo.tick();
    }

    if (operation_step == 11 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 12 && delayTimer.isReady()){
      operation_step = 0;
      operate_flag = false;
      boxServo.setSpeed(DEFAULT_BOX_SERVO_SPEED);
    }
}


void mode_6(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(500);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      handServo.setAccel(0.2);
      delayTimer.setTimeout(1300);
      handServo.setTargetDeg(150);
      handServo.tick();
    }

    if (operation_step == 2 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      handServo.setTargetDeg(120);
      handServo.tick();
    }

    if (operation_step == 3 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      handServo.setTargetDeg(90);
      handServo.tick();
    }

    if (operation_step == 4 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      handServo.setTargetDeg(60);
      handServo.tick();
    }

    if (operation_step == 5 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      handServo.setTargetDeg(MAX_HAND_SERVO + 15);
      handServo.tick();
    }

    if (operation_step == 6 && delayTimer.isReady()){
      operation_step ++;
      handServo.setAccel(0);
      delayTimer.start();
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }

    if (operation_step == 7 && delayTimer.isReady()){
      operation_step ++;
      handServo.setAccel(0.2);
      delayTimer.setTimeout(2300);
      handServo.setTargetDeg(180);
      handServo.tick();
    }

    if (operation_step == 8 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(500);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 9 && delayTimer.isReady()){
      operation_step = 0;
      operate_flag = false;
      handServo.setAccel(0);
    }
}


void mode_7(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      operation_step ++;
      boxServo.setAccel(0.3);
      delayTimer.setTimeout(1100);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      handServo.setAccel(0.3);
      delayTimer.setTimeout(1500);
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
      delayTimer.setTimeout(1600);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }

    if (operation_step == 4 && delayTimer.isReady()){
      operation_step ++;
      boxServo.setAccel(0);
      delayTimer.setTimeout(2000);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 5 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(500);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 6 && delayTimer.isReady()){
      operation_step = 0;
      operate_flag = false;
      handServo.setAccel(0);
    }
}


void mode_8(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(500);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      handServo.setAccel(0.5);
      delayTimer.setTimeout(2000);
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }

    if (operation_step == 2 && delayTimer.isReady()){
      operation_step ++;
      handServo.setAccel(0);
      delayTimer.setTimeout(500);
      handServo.setTargetDeg(MAX_HAND_SERVO + 30);
      handServo.tick();
    }

    if (operation_step == 3 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }

    if (operation_step == 4 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      handServo.setTargetDeg(MAX_HAND_SERVO + 30);
      handServo.tick();
    }

    if (operation_step == 5 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }

    if (operation_step == 6 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      handServo.setTargetDeg(MAX_HAND_SERVO + 30);
      handServo.tick();
    }
    
    if (operation_step == 7 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.start();
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }

    if (operation_step == 8 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(800);
      handServo.setTargetDeg(180);
      handServo.tick();
    }

    if (operation_step == 9 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(500);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 10 && delayTimer.isReady()){
      operation_step = 0;
      operate_flag = false;
    }
}


void mode_100(){
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
