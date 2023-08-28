//Krona battery tester by Rostyslav Gurevych

//---------- Define pins and constants
#define BUTTON_1_PIN 3                     //Button pin
#define SERVO_PIN 5                        //Servo pin
#define CLOSED_ANGLE 160                //Servo angle in closed position
#define OPEN_ANGLE 5                 //Servo angle in open position
#define TEST_INTERVAL 5000        //Period of time equal to one unit of delay counter (seconds)
#define INIT_ADDR 100
#define INIT_KEY 25


//---------- Include libraries
#include <GyverTimer.h>
#include <VirtualButton.h>
#include <ServoSmooth.h>
#include <EEPROM.h>

//---------- Initialize devices
VButton btn1;
ServoSmooth mainServo;

//---------- Timers
GTimer operationTimer(MS, TEST_INTERVAL);

//---------- Variables
boolean ledFlag = false;
boolean operationFlag = false;
boolean openFlag = false;
int max_cycle_counter = 0;
int test_cycle_counter = 0;


void setup() {
  //Pin modes
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  mainServo.attach(SERVO_PIN);
  Serial.begin(9600);

  //Startup
  ledFlag = false;
  if (EEPROM.read(INIT_ADDR) != INIT_KEY) {
    EEPROM.write(INIT_ADDR, INIT_KEY);
    EEPROM.put(0, max_cycle_counter);
  }
  else {
    EEPROM.get(0, max_cycle_counter);
    Serial.print("Last saved run cycle number: ");
    Serial.println(max_cycle_counter);
  }
    

  //Start timers
  operationTimer.start();
}


void loop() {
  buttonTick();
  operationTick();
  ledSwitch();
}


void buttonTick(){
  btn1.poll(!digitalRead(BUTTON_1_PIN));
  
  if(btn1.click()){
    ledFlag = true;
    operationFlag = true;
  }
}

void operationTick(){
  if(operationFlag){
    if(operationTimer.isReady()){
      openFlag = !openFlag;
      if(openFlag){
        openServo();
      }
      else{
        closeServo();
        test_cycle_counter ++;
        EEPROM.put(0, test_cycle_counter);
        Serial.print("Currently completed run cycles: ");
        Serial.println(test_cycle_counter);
      }
    }
  }
}


void openServo(){
  mainServo.write(OPEN_ANGLE);
}


void closeServo(){
  mainServo.write(CLOSED_ANGLE);
}


void ledSwitch() {
  digitalWrite(LED_BUILTIN, ledFlag);
}
