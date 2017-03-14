int rotateServoPosition = 90;
int pushServoPosition = 89;

#include <Servo.h>


Servo RotateServo;
Servo PushServo;

// Function for turning the bottom motor
void turn_RotateServo(int degree) {
  int limited_degree = constrain(degree, 0, 180);
  if(rotateServoPosition < limited_degree) {
    for(int i = rotateServoPosition; i <=limited_degree ; i++) {
    RotateServo.write(i);
    delay(2);
    }
  }
  else if (rotateServoPosition > limited_degree) {
    for(int i = rotateServoPosition; i >= limited_degree; i--) {
    RotateServo.write(i);
    delay(2);
    }
  }
  rotateServoPosition = limited_degree;
}

void turn_PushServo(int degree) {
  int limited_degree = constrain(degree, 0, 180);
  if(pushServoPosition < limited_degree) {
    for(int i = pushServoPosition; i <=limited_degree ; i++) {
    PushServo.write(i);
    delay(2);
    }
  }
  else if (rotateServoPosition > limited_degree) {
    for(int i = pushServoPosition; i >= limited_degree; i--) {
    PushServo.write(i);
    delay(2);
    }
  }
  pushServoPosition = limited_degree;
}


void setup() {
// Initialize servo motors
  RotateServo.attach(8);
  PushServo.attach(9);
}

void loop() {
  //turn_RotateServo(90);
  //delay(2000);
  //turn_RotateServo(0);
  //delay(2000);
  //turn_RotateServo(180);
  //delay(2000);
  turn_PushServo(90);
  delay(3000);
  turn_PushServo(30);
  delay(3000);
  
}

