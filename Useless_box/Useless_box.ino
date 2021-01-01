/*Useless box by R. Gurevych
 * 
 */

// Includes
#include <ServoSmooth.h>
#include <GyverTimer.h>


// Settings
#define MAX_HAND_SERVO 19                //Max allowed angle of hand servo
#define MAX_BOX_SERVO 150
#define DEFAULT_HAND_SERVO_SPEED 180
#define DEFAULT_BOX_SERVO_SPEED 100
#define SWITCH_MODES 1                   //Should the modes be switched automatically


// Pins
#define SERVO_HAND_PIN 5    //Hand pin servo
#define SERVO_BOX_PIN 6     //Box lid pin servo
#define SWITCH_PIN 2        //Switch pin

#define BUZZ_PIN 3          // Buzzer
#define SERVICE_LED_PIN 13
#define LED_1_PIN 9         // LED 1
#define LED_2_PIN 10        // LED 2


// Timers
GTimer delayTimer(MS);                //Delay timer used in each particular step
GTimer switchDelayTimer(MS, 500);     //Timer used between enabling switch and starting operation
GTimer blinkTimer(MS, 500);           //Timer used for blink functions
GTimer buzzTimer(MS, 700);            //Timer used for buzz functions


// Objects
ServoSmooth handServo;
ServoSmooth boxServo;


// Variables
boolean led_flag, hand_servo_state, box_servo_state;
boolean operate_flag = false; 
boolean led_simple_blink_flag = false, led_double_blink_flag = false, leds_on_flag = false;
boolean buzz_simple_flag = false, buzz_double_flag = false;
boolean led_1_flag = false, led_2_flag = false;
boolean buzz_flag = false;
byte operation_step = 0;
byte switch_count = 0;
byte mode = 0;


void setup() {
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(SERVICE_LED_PIN, OUTPUT);
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
  leds_function();
  buzz_function();
}


void check_switch(){
  if (digitalRead(SWITCH_PIN) == HIGH && !operate_flag){
    operate_flag = true;
    switchDelayTimer.start();
  }
  if (operate_flag){
    digitalWrite(SERVICE_LED_PIN, HIGH);
  }
  else{
    digitalWrite(SERVICE_LED_PIN, LOW);
  }
}


void update_mode(){
  switch_count ++;
  handServo.setSpeed(DEFAULT_HAND_SERVO_SPEED);
  boxServo.setSpeed(DEFAULT_BOX_SERVO_SPEED);
  
  if (SWITCH_MODES){
    if (switch_count == 0 || switch_count == 1 || switch_count == 13) mode = 0;
    else if (switch_count == 2 || switch_count == 15) mode = 1;
    else if (switch_count == 3 || switch_count == 6 || switch_count == 20) mode = 2;
    else if (switch_count == 4) mode = 3;
    else if (switch_count == 5 || switch_count == 10) mode = 4;
    else if (switch_count == 7 || switch_count == 19) mode = 5;
    else if (switch_count == 8) mode = 6;
    else if (switch_count == 9) mode = 7;
    else if (switch_count == 11) mode = 8;
    else if (switch_count == 12 || switch_count == 17) mode = 9;
    else if (switch_count == 14 || switch_count == 24) mode = 10;
    else if (switch_count == 16 || switch_count == 22) mode = 11;
    else if (switch_count == 18) mode = 12;
    else if (switch_count == 21) mode = 13;
    else if (switch_count == 23) mode = 14;
    else if (switch_count == 25) mode = 15;
    else if (switch_count > 25){
      switch_count = 0;
      mode = 0;
    }
  }
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
    else if (mode == 9) mode_9();
    else if (mode == 10) mode_10();
    else if (mode == 11) mode_11();
    else if (mode == 12) mode_12();
    else if (mode == 13) mode_13();
    else if (mode == 14) mode_14();
    else if (mode == 15) mode_15();
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
      delayTimer.setTimeout(1700);
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
      update_mode();
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
      delayTimer.setTimeout(1800);
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
      update_mode();
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
      update_mode();
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
      update_mode();
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
      update_mode();
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
      update_mode();
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
      update_mode();
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
      update_mode();
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
      update_mode();
    }
}


void mode_9(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      led_simple_blink_flag = true;
      operation_step ++;
      boxServo.setSpeed(8);
      delayTimer.setTimeout(3500);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(2500);
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }

    if (operation_step == 2 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(1500);
      handServo.setTargetDeg(180);
      handServo.tick();
    }

    if (operation_step == 3 && delayTimer.isReady()){
      operation_step ++;
      boxServo.setSpeed(10);
      delayTimer.setTimeout(2000);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 4 && delayTimer.isReady()){
      led_simple_blink_flag = false;
      operation_step = 0;
      operate_flag = false;
      boxServo.setSpeed(DEFAULT_BOX_SERVO_SPEED);
      update_mode();
    }
}


void mode_10(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      led_double_blink_flag = true;
      operation_step ++;
      delayTimer.setTimeout(1500);
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
      led_double_blink_flag = false;
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
      update_mode();
    }
}


void mode_11(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      buzz_simple_flag = true;
      operation_step ++;
      delayTimer.setTimeout(3000);
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      boxServo.setSpeed(8);
      delayTimer.setTimeout(3000);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 2 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(1500);
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }

    if (operation_step == 3 && delayTimer.isReady()){
      operation_step ++;
      buzz_simple_flag = false;
      delayTimer.setTimeout(1500);
      handServo.setTargetDeg(180);
      handServo.tick();
    }

    if (operation_step == 4 && delayTimer.isReady()){
      operation_step ++;
      boxServo.setSpeed(15);
      delayTimer.setTimeout(2000);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 5 && delayTimer.isReady()){
      operation_step = 0;
      operate_flag = false;
      boxServo.setSpeed(DEFAULT_BOX_SERVO_SPEED);
      update_mode();
    }
}


void mode_12(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(500);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      led_double_blink_flag = true;
      delayTimer.setTimeout(3000);
    }

    if (operation_step == 2 && delayTimer.isReady()){
      operation_step ++;
      led_double_blink_flag = false;
      delayTimer.setTimeout(1000);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }

    if (operation_step == 3 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(500);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 4 && delayTimer.isReady()){
      operation_step ++;
      buzz_simple_flag = true;
      delayTimer.setTimeout(3000);
    }
    
    if (operation_step == 5 && delayTimer.isReady()){
      operation_step ++;
      buzz_simple_flag = false;
      delayTimer.setTimeout(1000);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }

    if (operation_step == 6 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(500);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }
    
    if (operation_step == 7 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(1000);
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }
    
    if (operation_step == 8 && delayTimer.isReady()){
      operation_step ++;
      handServo.setSpeed(50);
      delayTimer.setTimeout(2500);
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
      handServo.setSpeed(DEFAULT_HAND_SERVO_SPEED);
      update_mode();
    }
}


void mode_13(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      buzz_double_flag = true;
      operation_step ++;
      delayTimer.setTimeout(3000);
    }
    
    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(500);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 2 && delayTimer.isReady()){
      operation_step ++;
      handServo.setSpeed(40);
      delayTimer.setTimeout(4000);
      handServo.setTargetDeg(MAX_HAND_SERVO + 15);
      handServo.tick();
    }

    if (operation_step == 3 && delayTimer.isReady()){
      operation_step ++;
      handServo.setSpeed(200);
      delayTimer.setTimeout(500);
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }
    
    if (operation_step == 4 && delayTimer.isReady()){
      buzz_double_flag = false;
      operation_step ++;
      handServo.setSpeed(80);
      delayTimer.setTimeout(2000);
      handServo.setTargetDeg(180);
      handServo.tick();
    }

    if (operation_step == 5 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(500);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }
    
    if (operation_step == 6 && delayTimer.isReady()){
      handServo.setSpeed(DEFAULT_HAND_SERVO_SPEED);
      operation_step = 0;
      operate_flag = false;
      update_mode();
    }
}


void mode_14(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(1800);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }

    if (operation_step == 1 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(1000);
      boxServo.setTargetDeg(180);
      boxServo.tick();
    }

    if (operation_step == 2 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(500);
      boxServo.setTargetDeg(MAX_BOX_SERVO);
      boxServo.tick();
    }
    
    if (operation_step == 3 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(1000);
      handServo.setTargetDeg(MAX_HAND_SERVO);
      handServo.tick();
    }
    
    if (operation_step == 4 && delayTimer.isReady()){
      operation_step ++;
      delayTimer.setTimeout(1000);
      handServo.setTargetDeg(180);
      handServo.tick();
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
      update_mode();
    }
}


void mode_15(){
    if (operation_step == 0 && switchDelayTimer.isReady()){
      melody_1();
      leds_on_flag = true;
      operation_step ++;
      boxServo.setSpeed(8);
      delayTimer.setTimeout(2500);
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
      melody_2();
      leds_on_flag = false;
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
      update_mode();
    }
}


void leds_function(){
  if (led_simple_blink_flag){
    if (blinkTimer.isReady()){
      blinkTimer.start();
      led_1_flag = !led_1_flag;
      led_2_flag = !led_2_flag;
    }
  }

  else if (led_double_blink_flag){
    if (blinkTimer.isReady()){
      blinkTimer.start();
      led_1_flag = led_2_flag;
      led_2_flag = !led_1_flag;
    }
  }
  
  else if (leds_on_flag){
    led_1_flag = true;
    led_2_flag = true;
  }
  
  else{
    led_1_flag = false;
    led_2_flag = false;
  }

  digitalWrite(LED_1_PIN, led_1_flag);
  digitalWrite(LED_2_PIN, led_2_flag);
}


void buzz_function(){
  if (buzz_simple_flag){
    if (buzzTimer.isReady()){
      buzzTimer.start();
      buzz_flag = !buzz_flag;
    }

    if (buzz_flag){
      tone(BUZZ_PIN, 800);
    }
    else{
      noTone(BUZZ_PIN);
    }
  }

  else if (buzz_double_flag){
    if (buzzTimer.isReady()){
      buzzTimer.start();
      buzz_flag = !buzz_flag;

      if (buzz_flag) tone(BUZZ_PIN, 349, 650);
      else tone(BUZZ_PIN, 494, 650);
    }
  }
  
  else{
    noTone(BUZZ_PIN);
  }
}


void melody_1(){
  int notes_1[] = {392, 392, 392, 311, 466, 392, 311, 466, 392};
  int times_1[] = {350, 350, 350, 250, 100, 350, 250, 100, 700};
  
  for (int i = 0; i < 9; i++){
    tone(BUZZ_PIN, notes_1[i], times_1[i]*2);
    delay(times_1[i]*2 + 25);
    noTone(BUZZ_PIN);
  }
}


void melody_2(){
  int notes_2[] = {587, 587, 587, 622, 466, 369, 311, 466, 392};
  int times_2[] = {350, 350, 350, 250, 100, 350, 250, 100, 700};
  
  for (int i = 0; i < 9; i++){
    tone(BUZZ_PIN, notes_2[i], times_2[i]*2);
    delay(times_2[i]*2 + 25);
    noTone(BUZZ_PIN);
  }
}
