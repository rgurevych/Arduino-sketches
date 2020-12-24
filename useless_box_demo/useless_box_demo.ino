/*Dawn clock
 * 
 */

// Includes
#include <GyverEncoder.h>
#include <GyverTM1637.h>
#include <Servo.h>
//#include <ServoSmooth.h>

#include <CyberLib.h>
#include <GyverButton.h>



// Settings
#define ENCODER_TYPE 1    // 1 or 2


// Pins
#define CLKe 9        // encoder S1
#define DTe 8         // encoder S2
#define SWe 10        // encoder Key

#define CLK 12        // display
#define DIO 11        // display

#define BUZZ_PIN 7    // пищалка (по желанию)
#define LED_PIN 6     // светодиод индикатор
#define SWITCH_PIN 2  // Switch

#define SERVO_HAND_PIN 5


// Timers


// Objects
Encoder enc(CLKe, DTe, SWe);
GyverTM1637 disp(CLK, DIO);

//ServoSmooth servoHand;
Servo servoHand;

// Variables
int motor = 0;
boolean led_flag; 
boolean operate_flag = false;
byte lcd_brightness = 7;
byte led_brightness = 25;

uint32_t myTimer;


void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  led_flag = false;

  enc.setType(ENCODER_TYPE);
  disp.clear();
  disp.brightness(lcd_brightness);

  servoHand.attach(SERVO_HAND_PIN);
  //servoHand.smoothStart();
  //servoHand.setTargetDeg(180);
  //servoHand.setAccel(0);
  //servoHand.setSpeed(300);
  servoHand.write(180);
}


void loop() {

  enc.tick();
  update_motor();
  switch_led();
  check_switch();
 // operate();
}


void update_motor(){
  if (enc.isRight()) motor++;
  if (enc.isRightH()) motor += 5;

  if (enc.isLeft()) motor--;
  if (enc.isLeftH()) motor -= 5;

  motor = constrain(motor, 0, 180);
  disp.displayInt(motor);
}


void switch_led(){
  if (enc.isClick()) led_flag =! led_flag;
  
  if (led_flag){
  

    
    analogWrite(LED_PIN, led_brightness);
    
    if (millis() - myTimer >= 40) {
    myTimer = millis();
   // servoHand.setTargetDeg(motor);
    }
  }
  else digitalWrite(LED_PIN, LOW);
}

void check_switch(){
  if (digitalRead(SWITCH_PIN) == HIGH && !operate_flag){
    operate_flag = true;
    operate();
  }
}

void operate(){
  if (operate_flag){
    servoHand.write(15);
    delay(1000);
    servoHand.write(180);
    operate_flag = false;
  }
}
