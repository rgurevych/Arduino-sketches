/*Dawn clock
 * 
 */

// Includes
#include <GyverEncoder.h>
#include <GyverTM1637.h>
#include <Servo.h> 

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
#define BUTTON_PIN 4  // кнопка

#define SERVO_HAND_PIN 5


// Timers


// Objects
Encoder enc(CLKe, DTe, SWe);
GyverTM1637 disp(CLK, DIO);
GButton button(BUTTON_PIN);
Servo servoHand;


// Variables
int motor = 0;
boolean led_flag;
byte lcd_brightness = 7;
byte led_brightness = 25;


void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  led_flag = false;

  enc.setType(ENCODER_TYPE);
  disp.clear();
  disp.brightness(lcd_brightness);

  servoHand.attach(SERVO_HAND_PIN);
  servoHand.write(0);
}


void loop() {
  enc.tick();
  button.tick();
  update_motor();
  switch_led();
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
    servoHand.write(motor);
  }
  else digitalWrite(LED_PIN, LOW);
}
