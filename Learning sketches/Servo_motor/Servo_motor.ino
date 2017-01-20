int sensorPin = 0;
int servoPin = 12;

#include <Servo.h>

Servo Servomotor;

void setup() {
  // put your setup code here, to run once:
  pinMode(sensorPin, INPUT);
  Servomotor.attach(servoPin);
}

void loop() {
  // put your main code here, to run repeatedly:
  int val = analogRead(sensorPin);
  
  int servo_val = map(val, 1023, 0, 179, 0);
  
  Servomotor.write(90);

}
