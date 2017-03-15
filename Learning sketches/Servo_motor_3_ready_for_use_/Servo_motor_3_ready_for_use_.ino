
//Defining pins
#define rotateMotorPin 11
#define carriageMotorPin 12
#define pusherMotorPin 13

//Including libraries
#include <Servo.h>
#include <MsTimer2.h>


//Defining variables

int servo_val_1;
int servo_val_2;
int servo_val_3;

byte motor_1_Position = 88;
byte motor_2_Position = 105;
byte motor_3_Position = 90;

byte minAngleMotor1 = 0;   // minimum limit fur turning
byte maxAngleMotor1 = 179; // maximum limit for turning
byte minAngleMotor2 = 95;  // minimum limit fur turning
byte maxAngleMotor2 = 135; // maximum limit for turning
byte minAngleMotor3 = 45;  // minimum limit fur turning
byte maxAngleMotor3 = 95;  // maximum limit for turning

//Initializing motors
Servo Motor1;
Servo Motor2;
Servo Motor3;


void setup() {
  // put your setup code here, to run once:
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  Motor1.attach(rotateMotorPin);
  Motor2.attach(carriageMotorPin);
  Motor3.attach(pusherMotorPin);
  Serial.begin(9600);
  //MsTimer2::set(500, timerInterupt);
  //MsTimer2::start();    
}

void loop() {
  // put your main code here, to run repeatedly:
  int val_1 = analogRead(A1);
  int val_2 = analogRead(A2);
  int val_3 = analogRead(A3);
  
  servo_val_1 = map(val_1, 0, 1023, 0, 179);
  servo_val_2 = map(val_2, 0, 1023, 0, 179);
  servo_val_3 = map(val_3, 0, 1023, 0, 179);
  
  //turnMotor1(servo_val_1, 2);
  //turnMotor2(servo_val_2, 3);
  //turnMotor3(servo_val_3, 0);
  //Motor1.write(servo_val_1);
  //Motor2.write(servo_val_2);
  //Motor3.write(servo_val_3);
  
  turnMotor1(4, 2);
  delay(3000);
  push();

}

void timerInterupt() {
  Serial.print("Servo1_angle= ");
  Serial.print(servo_val_1);
  Serial.print(";  Servo2_angle= ");
  Serial.print(servo_val_2);
  Serial.print(";  Servo3_angle= ");
  Serial.println(servo_val_3);
}

//Method for turning the Motor1
void turnMotor1(byte degree, byte motorSpeed) {
  byte limitedDegree = constrain(degree, minAngleMotor1, maxAngleMotor1);
  if(motor_1_Position < limitedDegree) {
    for(int i = motor_1_Position; i <=limitedDegree ; i++) {
    Motor1.write(i);
    delay(motorSpeed);
    }
  }
  else if (motor_1_Position > limitedDegree) {
    for(int i = motor_1_Position; i >= limitedDegree; i--) {
    Motor1.write(i);
    delay(motorSpeed);
    }
  }
  motor_1_Position = limitedDegree;
}

//Method for turning the Motor2
void turnMotor2(byte degree, byte motorSpeed) {
  byte limitedDegree = constrain(degree, minAngleMotor2, maxAngleMotor2);
  if(motor_2_Position < limitedDegree) {
    for(int i = motor_2_Position; i <=limitedDegree ; i++) {
    Motor2.write(i);
    delay(motorSpeed);
    }
  }
  else if (motor_2_Position > limitedDegree) {
    for(int i = motor_2_Position; i >= limitedDegree; i--) {
    Motor2.write(i);
    delay(motorSpeed);
    }
  }
  motor_2_Position = limitedDegree;
}

//Method for turning the Motor3
void turnMotor3(byte degree, byte motorSpeed) {
  byte limitedDegree = constrain(degree, minAngleMotor3, maxAngleMotor3);
  if(motor_3_Position < limitedDegree) {
    for(int i = motor_3_Position; i <=limitedDegree ; i++) {
    Motor3.write(i);
    delay(motorSpeed);
    }
  }
  else if (motor_3_Position > limitedDegree) {
    for(int i = motor_3_Position; i >= limitedDegree; i--) {
    Motor3.write(i);
    delay(motorSpeed);
    }
  }
  motor_3_Position = limitedDegree;
}

void push() {
  turnMotor2(105, 5);
  delay(200);
  turnMotor3(45, 2);
  delay(200);
  turnMotor2(118, 5);
  delay(200);
  turnMotor3(95, 2);
  delay(200);
  turnMotor2(105, 5);
  delay(200);
  turnMotor3(90, 2);
  
}

