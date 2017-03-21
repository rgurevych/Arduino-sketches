
//Defining pins
#define rotateMotorPin 11
#define carriageMotorPin 12
#define pusherMotorPin 13
#define scannerMotorPin 10
#define leftButtonPin 2
#define rightButtonPin 3

//Including libraries
#include <Servo.h>
#include <MsTimer2.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


//Defining variables

//Temporary variables
byte motor_1_angle;
byte motor_4_angle;

//Default motors positions
byte motor_1_Position = 90;
byte motor_2_Position = 104;
byte motor_3_Position = 89;
byte motor_4_Position = 34;

//Default motors movements limits
byte minAngleMotor1 = 0;   // minimum limit fur turning
byte maxAngleMotor1 = 179; // maximum limit for turning
byte minAngleMotor2 = 95;  // minimum limit fur turning
byte maxAngleMotor2 = 135; // maximum limit for turning
byte minAngleMotor3 = 45;  // minimum limit fur turning
byte maxAngleMotor3 = 95;  // maximum limit for turning
byte minAngleMotor4 = 35;  // minimum limit fur turning
byte maxAngleMotor4 = 98;  // maximum limit for turning

//Flags
volatile boolean leftButtonIsPressed = false;
volatile boolean rightButtonIsPressed = false;
volatile boolean operateFlag = false;


//Data arrays and matrixes
//Arrays for color scanning
byte Motor_1_ScanAngles [] = {44, 85, 122, 4, 4, 168, 134, 96, 58};
byte Motor_4_ScanAngles [] = {71, 75, 71, 75, 83, 74, 96, 91, 95};

//Initializing motors and LCD
Servo Motor1;
Servo Motor2;
Servo Motor3;
Servo Motor4;
LiquidCrystal_I2C lcd(0x3f,16,2);


//Setup method
void setup() {
  // put your setup code here, to run once:
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  //pinMode(leftButtonPin, INPUT_PULLUP);
  //pinMode(rightButtonPin, INPUT_PULLUP);
  Motor1.attach(rotateMotorPin);
  Motor2.attach(carriageMotorPin);
  Motor3.attach(pusherMotorPin);
  Motor4.attach(scannerMotorPin);
  Serial.begin(9600);
  
  turnMotor1(89, 3);
  delay(200);
  turnMotor3(90, 2);
  delay(200);
  turnMotor2(105, 5);
  delay(200);
  turnMotor4(35, 7);
  delay(200);

  // Initialise interruption pin
  attachInterrupt (0, leftButtonPressed, RISING);
  attachInterrupt (1, rightButtonPressed, RISING);

  //Initialize LCD
  lcd.init();
  lcd.backlight();
  
  //MsTimer2::set(500, timerInterupt);
  //MsTimer2::start();    
}

//Main operation method
void loop() {
  // put your main code here, to run repeatedly:
  operateOrNot();
  
  //String movementString = "RLFBRLFBRLFB";
  //assembleCube(movementString);

  scanTopSide();
  
  /*byte motor_1_angle = map(analogRead(A0), 0, 1023, 0, 179);
  byte motor_4_angle = map(analogRead(A1), 0, 1023, 0, 179);
    
  turnMotor1(motor_1_angle, 3);
  turnMotor4(motor_4_angle, 3);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Motor_1  Motor_4");
  lcd.setCursor(0, 1);
  lcd.print(motor_1_angle);
  lcd.setCursor(8, 1);
  lcd.print(motor_4_angle);*/
  
  delay(5000);

 

}


/////////////////////////////////////////////////METHODS FOR MANAGING INTERRUPTIONS AND OPERATION/////////////////////////////////////////

//Method for dealing with timer interruption
void timerInterupt() {
  //Serial.print("Servo1_angle= ");
  //Serial.print(servo_val_1);
  //Serial.print(";  Servo2_angle= ");
  //Serial.print(servo_val_2);
  //Serial.print(";  Servo3_angle= ");
  //Serial.println(servo_val_3);
  
  Serial.print("Left_button= ");
  Serial.print(leftButtonIsPressed);
  Serial.print("  Right_button= ");
  Serial.println(rightButtonIsPressed);
  
}


//Interruption methods for detecting button press
void leftButtonPressed() {
  leftButtonIsPressed = true;
}

void rightButtonPressed() {
  rightButtonIsPressed = true;
}


//Methods for checking if the buttons are pressed
void waitForLeftButtonIsPressed() {
  leftButtonIsPressed = false;
  while (leftButtonIsPressed == false) {
      delay(10);
    }
  leftButtonIsPressed = false;
}

void waitForRightButtonIsPressed() {
  rightButtonIsPressed = false;
  while (rightButtonIsPressed == false) {
      delay(10);
    }
  rightButtonIsPressed = false;
}

//Method for starting or stopping operation
void operateOrNot() {
  if (leftButtonIsPressed == true) {
    operateFlag = false;
  }
  if (operateFlag == false) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Operation paused");
    lcd.setCursor(0, 1);
    lcd.print("Press button...");
    leftButtonIsPressed = false;
    operateFlag = true;
    while (leftButtonIsPressed == false) {
      delay(10);
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Resuming...");
    delay(200);
    leftButtonIsPressed = false;
    operateFlag = true;
  }
  
}

/////////////////////////////////////////////////METHODS FOR SCANNING AND SCANNER CALIBRATION////////////////////////
void scanTopSide() {
  turnMotor2(95, 5);
  delay(200);
  turnMotor4(35, 7);
  delay(200);
  for (int i = 0; i<9; i++) {
    turnMotor1(Motor_1_ScanAngles[i], 4);
    turnMotor4(Motor_4_ScanAngles[i], 7);
    delay(2000);
  }
  turnMotor4(35, 7);
  turnCubeStraight();
}


/////////////////////////////////////////////////METHODS FOR CUBE ASSEMBLING/////////////////////////////////////////

//Method for assembling cube based on the steps string
void assembleCube(String movementString) {
for (byte i=0; i<=movementString.length(); i++) {
    String stepValue = String(movementString[i]);
    Serial.println(stepValue);

    //Print process on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Assembling cube");
    lcd.setCursor(0, 1);
    lcd.print("Step   ( ) of ");
    lcd.setCursor(5, 1);
    lcd.print(i+1);
    lcd.setCursor(14, 1);
    lcd.print(movementString.length());
    lcd.setCursor(8, 1);
    lcd.print(stepValue);
        
    makeStep(stepValue);
  }
}

//Method for implementing one defined step
void makeStep (String stepCode){
    
  if(stepCode == "D") {
    downCW();
  }
  
  else if(stepCode == "d") {
    downCCW();
  }
  
  else if(stepCode == "U") {
    push();
    push();
    downCW();
    push();
    turnCubeCCW();
    push();
    turnCubeStraight();
    push();
  }

  else if(stepCode == "u") {
    push();
    push();
    downCCW();
    push();
    turnCubeCW();
    push();
    turnCubeStraight();
    push();
  }

  else if(stepCode == "R") {
    turnCubeCCW();
    push();
    downCW();
    turnCubeCW();
    push();
    turnCubeStraight();
    push();
  }


  else if(stepCode == "r") {
    turnCubeCCW();
    push();
    downCCW();
    turnCubeCCW();
    push();
    push();
    push();
    turnCubeStraight();
    push();
    push();
    push();
  }

  else if(stepCode == "L") {
    turnCubeCW();
    push();
    downCW();
    turnCubeCW();
    push();
    push();
    push();
    turnCubeStraight();
    push();
    push();
    push();
  }

  else if(stepCode == "l") {
    turnCubeCW();
    push();
    downCCW();
    turnCubeCCW();
    push();
    turnCubeStraight();
    push();
  }

  else if(stepCode == "F") {
    push();
    push();
    push();
    downCW();
    push();
    turnCubeCCW();
    push();
    turnCubeStraight();
  }

  else if(stepCode == "f") {
    push();
    push();
    push();
    downCCW();
    push();
    turnCubeCW();
    push();
    turnCubeStraight();
  }

  else if(stepCode == "B") {
    push();
    downCW();
    push();
    push();
    push();
    turnCubeCW();
    push();
    turnCubeStraight();
  }

  else if(stepCode == "b") {
    push();
    downCCW();
    push();
    push();
    push();
    turnCubeCCW();
    push();
    turnCubeStraight();
  }
  
  else {
    operateFlag = false;
  }

}

/////////////////////////////////////////////////ELEMENTARY MOTORS MOVEMENTS/////////////////////////////////////////

//Method for turning the Motor1 (the motor which rotates the cube)
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

//Method for turning the Motor2 (the motor which moves the carriage)
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

//Method for turning the Motor3 (the motor that moves the pusher)
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

//Method for turning the Motor4 (the motor that moves the color scanner)
void turnMotor4(byte degree, byte motorSpeed) {
  byte limitedDegree = constrain(degree, minAngleMotor4, maxAngleMotor4);
  if(motor_4_Position < limitedDegree) {
    for(int i = motor_4_Position; i <=limitedDegree ; i++) {
    Motor4.write(i);
    delay(motorSpeed);
    }
  }
  else if (motor_4_Position > limitedDegree) {
    for(int i = motor_4_Position; i >= limitedDegree; i--) {
    Motor4.write(i);
    delay(motorSpeed);
    }
  }
  motor_4_Position = limitedDegree;
}


//Method for pushing the cube
void push() {
  operateOrNot();
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
  delay(200);
}

//Method for turning the cube ClockWise
void turnCubeCW() {
  operateOrNot();
  turnMotor2(95, 5);
  delay(200);
  turnMotor1(5, 3);
  delay(200);
  turnMotor2(105, 5);
  delay(200);
}

//Method for turning the cube CounterClockWise
void turnCubeCCW() {
  operateOrNot();
  turnMotor2(95, 5);
  delay(200);
  turnMotor1(171, 3);
  delay(200);
  turnMotor2(105, 5);
  delay(200);
}

//Method for turning the cube straight
void turnCubeStraight() {
  operateOrNot();
  turnMotor2(95, 5);
  delay(200);
  turnMotor1(88, 3);
  delay(200);
  turnMotor2(105, 5);
  delay(200);
}

//Method for turning the bottom layer ClockWise
void downCW() {
  turnCubeCW();
  turnMotor3(45, 2);
  delay(200);
  turnMotor2(135, 5);
  delay(200);
  turnMotor1(95, 3);
  delay(200);
  turnMotor1(88, 3);
  delay(200);
  turnMotor2(105, 5);
  delay(200);
  turnMotor3(90, 2);
  delay(200);
}

//Method for turning the bottom layer CounterClockWise
void downCCW() {
  turnCubeCCW();
  turnMotor3(45, 2);
  delay(200);
  turnMotor2(135, 5);
  delay(200);
  turnMotor1(80, 3);
  delay(200);
  turnMotor1(88, 3);
  delay(200);
  turnMotor2(105, 5);
  delay(200);
  turnMotor3(90, 2);
  delay(200);
}
