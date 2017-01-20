#include <AccelStepper.h>
#define HALFSTEP 8

// Motor pin definitions
#define motorPin1  2     // IN1 on the ULN2003 driver 1
#define motorPin2  3     // IN2 on the ULN2003 driver 1
#define motorPin3  4     // IN3 on the ULN2003 driver 1
#define motorPin4  5     // IN4 on the ULN2003 driver 1

// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper with 28BYJ-48
AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

void setup() {
  stepper1.setMaxSpeed(1800);
  stepper1.setAcceleration(1000);
  stepper1.setSpeed(1800);
  //stepper1.moveTo(2048);

}//--(end setup )---

void loop() {
    stepper1.runToNewPosition(0);
    delay(1000);
    stepper1.runToNewPosition(1024);
    delay(1000);
    stepper1.runToNewPosition(2048);
    delay(1000);
    stepper1.runToNewPosition(0);
    delay(1000);
    stepper1.runToNewPosition(-1024);
    delay(1000);
}
