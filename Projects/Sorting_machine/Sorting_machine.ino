
// Setup the constants with pin numbers
const int buttonIntPin = 2;
const int buttonPin = 3;
const int loadServoPin = 4;
const int selectServoPin = 5;
const int S0 = 6;
const int S1 = 7;
const int S2 = 8;
const int S3 = 9;
const int scanSensor = 10;
const int greenLedPin = 11;
const int yellowLedPin = 12;
const int redLedPin = 13;
const int photoPin = 0;

//Setup default positions
int loadServoPosition = 100;
int selectServoPosition = 100;
int loadPosition = 121;
int scanPosition = 72;
int dropPosition = 30;

//Initialize the variable for color frequences
int frequency = 0;
int f_RED = 0;
int f_GREEN = 0;
int f_BLUE = 0;

//Initialize flags
boolean startCalibration = false;
boolean startOperation = true;
volatile boolean buttonIsPressed = false;

//Calibration matrix
int f[][6] = { {26, 28, 26, 29, 19, 23},
               {20, 23, 30, 33, 25, 28},
               {21, 25, 23, 26, 23, 25},
               {15, 18, 19, 22, 22, 25},
               {16, 20, 25, 29, 23, 27},
               {25, 28, 30, 33, 25, 28}
               };

// Initialize servo library
#include <Servo.h>

Servo LoadServo;
Servo SelectServo;


// Function for turning the bottom motor
void turn_SelectServo(int degree) {
  int limited_degree = constrain(degree, 40, 175);
  if(selectServoPosition < limited_degree) {
    for(int i = selectServoPosition; i <=limited_degree ; i++) {
    SelectServo.write(i);
    delay(4);
    }
  }
  else if (selectServoPosition > limited_degree) {
    for(int i = selectServoPosition; i >= limited_degree; i--) {
    SelectServo.write(i);
    delay(4);
    }
  }
  selectServoPosition = limited_degree;
}


// Function for turning the upper motor
void turn_LoadServo(int degree) {
  int limited_degree = constrain(degree, 25, 135);
  if(loadServoPosition < limited_degree) {
    for(int i = loadServoPosition; i <=limited_degree ; i++) {
    LoadServo.write(i);
    delay(4);
    }
  }
  else if (loadServoPosition > limited_degree) {
    for(int i = loadServoPosition; i >= limited_degree; i--) {
    LoadServo.write(i);
    delay(4);
    }
  }
  loadServoPosition = limited_degree;
}


//Function for scanning the item once
void scan() {
  delay(100);
  // Setting red filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  // Reading the output frequency
  f_RED = pulseIn(scanSensor, LOW);
  delay(100);
  // Setting Green filtered photodiodes to be read
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  // Reading the output frequency
  f_GREEN = pulseIn(scanSensor, LOW);
  delay(100);
  // Setting Blue filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  // Reading the output frequency
  f_BLUE = pulseIn(scanSensor, LOW);
  delay(100);
}


//Function for detecting color (All devices and Arduino connected to power supply)
String defineColor(int r, int g, int b) {
  if(r>=f[0][0] && r<=f[0][1] && g>=f[0][2] && g<=f[0][3] && b>=f[0][4] && b<=f[0][5]){
    return "blue";
  }
  else if(r>=f[1][0] && r<=f[1][1] && g>=f[1][2] && g<=f[1][3] && b>=f[1][4] && b<=f[1][5]){
    return "red";
  }
  else if(r>=f[2][0] && r<=f[2][1] && g>=f[2][2] && g<=f[2][3] && b>=f[2][4] && b<=f[2][5]){
    return "green";
  }
  else if(r>=f[3][0] && r<=f[3][1] && g>=f[3][2] && g<=f[3][3] && b>=f[3][4] && b<=f[3][5]){
    return "yellow";
  }
  else if(r>=f[4][0] && r<=f[4][1] && g>=f[4][2] && g<=f[4][3] && b>=f[4][4] && b<=f[4][5]){
    return "orange";
  }
  else if(r>=f[5][0] && r<=f[5][1] && g>=f[5][2] && g<=f[5][3] && b>=f[5][4] && b<=f[5][5]){
    return "brown";
  }
  else {
    return "undefined";
  }
}


//Function that repeats scanning up to 3 times if the color was not defined
String scanResult(){
  String color = " ";
  int wrongScan = 0;
  do{
    scan();
    color = defineColor(f_RED, f_GREEN, f_BLUE);
    wrongScan = wrongScan + 1;
    if (color != "undefined") { 
      break;
      }
  } while (wrongScan < 2);
  return color;
}


// Function that turns LoadMotor to the selected contaiter or stops until the button is pressed
void selectContainer(String color){
  if(color == "blue") {
    turn_SelectServo(40);
  }
  else if(color == "red") {
    turn_SelectServo(66);
  }
  else if(color == "green") {
    turn_SelectServo(87);
  }
  else if(color == "yellow") {
    turn_SelectServo(115);
  }
  else if(color == "orange") {
    turn_SelectServo(144);
  }
  else if(color == "brown") {
    turn_SelectServo(175);
  }
  else {
    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, HIGH);
    waitForButtonPressed();
  }
  delay(100); 
}


//Function for determining if the intem is present in the loader
void loadTrayEmpty() {
  delay(100);
  int light = analogRead(photoPin);
  if (light > 900) {
     digitalWrite(greenLedPin, LOW);
     buttonIsPressed = false;
     while (analogRead(photoPin) > 900 || buttonIsPressed == false) {
       buttonIsPressed = false;
       digitalWrite(yellowLedPin, HIGH);
       delay(500);
       digitalWrite(yellowLedPin, LOW);
       delay(500);
     }
   buttonIsPressed = false;
   digitalWrite(greenLedPin, HIGH);
  } 
}


//Supportive function for device calibration
void calibrate() {
  // Setting red filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  // Reading the output frequency
  frequency = pulseIn(scanSensor, LOW);
  // Printing the value on the serial monitor
  Serial.print("R= ");//printing name
  Serial.print(frequency);//printing RED color frequency
  Serial.print("  ");
  delay(100);
  // Setting Green filtered photodiodes to be read
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  // Reading the output frequency
  frequency = pulseIn(scanSensor, LOW);
  // Printing the value on the serial monitor
  Serial.print("G= ");//printing name
  Serial.print(frequency);//printing GREEN color frequency
  Serial.print("  ");
  delay(100);
  // Setting Blue filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  // Reading the output frequency
  frequency = pulseIn(scanSensor, LOW);
  // Printing the value on the serial monitor
  Serial.print("B= ");//printing name
  Serial.print(frequency);//printing BLUE color frequency
  Serial.println("  ");
  delay(100);
}


void rainbowBlink() {
  digitalWrite(greenLedPin, HIGH);
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(redLedPin, LOW);
  delay(500);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(yellowLedPin, HIGH);
  digitalWrite(redLedPin, LOW);
  delay(500);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(redLedPin, HIGH);
  delay(500);
  digitalWrite(greenLedPin, HIGH);
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(redLedPin, LOW);
}


//Function for checking if the button is pressed
void waitForButtonPressed() {
  buttonIsPressed = false;
  while (buttonIsPressed == false) {
      delay(10);
    }
  buttonIsPressed = false;
}


//Interruption function for detecting pressed button
void buttonPressed() {
  buttonIsPressed = true;
}


//Function for sortning arrays
void sort(int a[15]) {
    for(int i=0; i<(14); i++) {
        for(int o=0; o<(15-(i+1)); o++) {
                if(a[o] > a[o+1]) {
                    int t = a[o];
                    a[o] = a[o+1];
                    a[o+1] = t;
                }
        }
    }
}


//Function that describes normal operation
void operate() {
  digitalWrite(greenLedPin, HIGH);
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(redLedPin, LOW);
  turn_LoadServo(130);
  delay(50);
  turn_LoadServo(loadPosition);
  delay(100);
  loadTrayEmpty();
  turn_LoadServo(scanPosition);
  delay(100);
  String color = scanResult();
  selectContainer(color);
  turn_LoadServo(dropPosition);
  delay(100);
}


void autoCalibrate() {
  rainbowBlink();
  String colorSequence[6]= {"blue", "red", "green", "yellow", "orange", "brown"};
  int redArray[15];
  int greenArray[15];
  int blueArray[15];
  for(int colorCount=0; colorCount<6; colorCount++) {
     int arrayElement = 0;
     for(int item=0; item<3; item++){
        turn_LoadServo(130);
        delay(50);
        turn_LoadServo(loadPosition);
        delay(100);
        loadTrayEmpty();
        turn_LoadServo(scanPosition);
        delay(100);
        for(int scanCount=1; scanCount<6; scanCount++) {
            scan();
            delay(50);
            redArray[arrayElement] = f_RED;
            greenArray[arrayElement] = f_GREEN;
            blueArray[arrayElement] = f_BLUE;
            arrayElement ++;
          }
        selectContainer(colorSequence[colorCount]);
        delay(100);
        turn_LoadServo(dropPosition);
        delay(100);
      }
     sort(redArray);
     sort(greenArray);
     sort(blueArray);
     f[colorCount][0] = redArray[2]-1;
     f[colorCount][1] = redArray[12]+1;
     f[colorCount][2] = greenArray[2]-1;
     f[colorCount][3] = greenArray[12]+1;
     f[colorCount][4] = blueArray[2]-1;
     f[colorCount][5] = blueArray[12]+1;
  }
  turn_LoadServo(scanPosition);
  delay(100);
  rainbowBlink();
  digitalWrite(greenLedPin, LOW);
  digitalWrite(yellowLedPin, HIGH);
  digitalWrite(redLedPin, LOW);
  
  for (int i=0; i<6; i++) {
    Serial.print(i);
    Serial.print(": ");
      for (int j=0; j<6; j++) {
        Serial.print(f[i][j]);
        Serial.print(" ");
      }
    Serial.println(" ");
  }
}

void setup() {
  // Initialize servo motors
  LoadServo.attach(loadServoPin);
  SelectServo.attach(selectServoPin);

  // Initialize pins for color sensor
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(scanSensor, INPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

  // Setting frequency-scaling to 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);

  // Initialize the serial connection
  Serial.begin(9600);

  // Initialise interruption pin
  attachInterrupt (0, buttonPressed, RISING);

  // Start initial checkings
  digitalWrite(greenLedPin, LOW);
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(redLedPin, HIGH);
  if (digitalRead(buttonPin) == HIGH) startCalibration = true;
  
  // Initial check of servomotors movements
  delay(200);
  turn_LoadServo(scanPosition);
  delay(200);
  turn_SelectServo(100);
  delay(500);
  digitalWrite(yellowLedPin, HIGH);
  digitalWrite(redLedPin, LOW);
}

void loop() {
  if (startCalibration == true) {
    autoCalibrate();
    startCalibration = false;
  }
  
  if (startOperation == true) {
    waitForButtonPressed();
    startOperation = false;
  }
  operate();
}
