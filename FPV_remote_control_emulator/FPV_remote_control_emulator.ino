//Remote control emulator by R. Gurevych 

#define BUTTON_1_PIN 3                         //Button 1 pin
#define BUTTON_2_PIN 2                         //Button 2 pin
#define OUTPUT_PIN 9                           //Output pin
#define MIN_PWM_OUTPUT 1000                    //Minimum PWM output value
#define MAX_PWM_OUTPUT 2000                    //Maximum PWM output value
#define PWM_STEP 500                           //PWM step value
#define PWM_START 1000                         //PWM starting value
#define DISP1637_CLK_DELAY 100 

//---------- Include libraries
#include <EncButton.h>
#include <Wire.h>
#include <Servo.h>
#include <GyverOLED.h>

//---------- Declare variables
int pwmPeriod = PWM_START;

//---------- Initialize devices
Button leftBtn(BUTTON_1_PIN, INPUT_PULLUP);
Button rightBtn(BUTTON_2_PIN, INPUT_PULLUP);
Servo outputPin;
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;


void setup() {
  Serial.begin(9600);
  Serial.println(F("Started remote control emulator"));
  outputPin.attach(OUTPUT_PIN);
  outputPin.writeMicroseconds(pwmPeriod);

  oled.init();
  oled.clear();
  oled.setContrast(200);
  oled.setScale(2);
  oled.setCursor(0, 1);
  oled.print(F("PWM Value:"));
  oled.setScale(3);
  updateDisplay(pwmPeriod);
}

void loop() {
  buttonTick();

}

void buttonTick() {
  leftBtn.tick();
  rightBtn.tick();

  if (leftBtn.click()) {
    pwmPeriod -= PWM_STEP;
    updatePWM();
  }

  if (rightBtn.click()) {
    pwmPeriod += PWM_STEP;
    updatePWM();
  }
}

void updatePWM() {
  pwmPeriod = constrain(pwmPeriod, MIN_PWM_OUTPUT, MAX_PWM_OUTPUT);
  outputPin.writeMicroseconds(pwmPeriod);
  Serial.print(F("Setting PWM Period to ")); Serial.print(pwmPeriod); Serial.println(F(" us"));
  updateDisplay(pwmPeriod);
}

void updateDisplay(int value) {
  oled.setCursor(0, 4);
  oled.print(value);
}
