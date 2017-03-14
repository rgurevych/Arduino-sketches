#include <Servo.h>
#include <MsTimer2.h>

int servo_val_1;
int servo_val_2;
int servo_val_3;

Servo Servomotor1;
Servo Servomotor2;
Servo Servomotor3;

void setup() {
  // put your setup code here, to run once:
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  Servomotor1.attach(11);
  Servomotor2.attach(12);
  Servomotor3.attach(13);
  Serial.begin(9600);
  MsTimer2::set(500, timerInterupt);
  MsTimer2::start();    
}

void loop() {
  // put your main code here, to run repeatedly:
  int val_1 = analogRead(A1);
  int val_2 = analogRead(A2);
  int val_3 = analogRead(A3);
  
  servo_val_1 = map(val_1, 0, 1023, 0, 179);
  servo_val_2 = map(val_2, 0, 1023, 0, 179);
  servo_val_3 = map(val_3, 0, 1023, 0, 179);
  
  Servomotor1.write(servo_val_1);
  Servomotor2.write(servo_val_2);
  Servomotor3.write(servo_val_3);

}

void  timerInterupt() {
  Serial.print("Servo1_angle= ");
  Serial.print(servo_val_1);
  Serial.print(";  Servo2_angle= ");
  Serial.print(servo_val_2);
  Serial.print(";  Servo3_angle= ");
  Serial.println(servo_val_3);
}
