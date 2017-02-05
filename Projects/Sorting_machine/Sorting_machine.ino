
// Setup the constants with pin numbers
const int loadServoPin = 2;
const int selectServoPin = 3;
const int S0 = 4;
const int S1 = 5;
const int S2 = 6;
const int S3 = 7;
const int scanSensor = 8;
const int greenLedPin = 10;
const int yellowLedPin = 11;
const int redLedPin = 12;

//Setup default positions
int loadServoPosition = 100;
int selectServoPosition = 100;
int loadPosition = 122;
int scanPosition = 72;
int dropPosition = 30;

// Initialize the variable for color frequences
int frequency = 0;
int f_RED = 0;
int f_GREEN = 0;
int f_BLUE = 0;

// Initialize servo library
#include <Servo.h>

Servo LoadServo;
Servo SelectServo;


// Function for turning the bottom motor
void turn_SelectServo(int degree) {
  int limited_degree = constrain(degree, 40, 170);
  if(selectServoPosition < limited_degree) {
    for(int i = selectServoPosition; i <=limited_degree ; i++) {
    SelectServo.write(i);
    delay(3);
    }
  }
  else if (selectServoPosition > limited_degree) {
    for(int i = selectServoPosition; i >= limited_degree; i--) {
    SelectServo.write(i);
    delay(3);
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
  delay(200);
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

//Function for detecting color (All devices connected via Arduino)
String defineColorOld(int r, int g, int b) {
  if(r>=33 && r<=41 && g>=37 && g<=39 && b>=21 && b<=29){
    return "blue";
  }
  else if(r>=25 && r<=32 && g>=42 && g<=45 && b>=34 && b<=37){
    return "red";
  }
  else if(r>=30 && r<=35 && g>=28 && g<=35 && b>=29 && b<=34){
    return "green";
  }
  else if(r>=22 && r<=25 && g>=21 && g<=30 && b>=29 && b<=32){
    return "yellow";
  }
  else if(r>=24 && r<=27 && g>=35 && g<=39 && b>=31 && b<=35){
    return "orange";
  }
  else if(r>=33 && r<=39 && g>=40 && g<=45 && b>=35 && b<=38){
    return "brown";
  }
  else {
    return "undefined";
  }
}

//Function for detecting color (All devices and Arduino connected to power supply)
String defineColor(int r, int g, int b) {
  if(r>=29 && r<=32 && g>=28 && g<=31 && b>=19 && b<=23){
    return "blue";
  }
  else if(r>=23 && r<=26 && g>=32 && g<=35 && b>=26 && b<=29){
    return "red";
  }
  else if(r>=25 && r<=28 && g>=24 && g<=28 && b>=23 && b<=27){
    return "green";
  }
  else if(r>=17 && r<=20 && g>=21 && g<=24 && b>=22 && b<=26){
    return "yellow";
  }
  else if(r>=19 && r<=22 && g>=27 && g<=31 && b>=24 && b<=27){
    return "orange";
  }
  else if(r>=28 && r<=31 && g>=32 && g<=35 && b>=26 && b<=29){
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
    turn_SelectServo(92);
  }
  else if(color == "yellow") {
    turn_SelectServo(118);
  }
  else if(color == "orange") {
    turn_SelectServo(144);
  }
  else if(color == "brown") {
    turn_SelectServo(170);
  }
  else {
    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, HIGH);
    delay(3000);
  }
  delay(1000); 
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
  Serial.print(frequency);//printing RED color frequency
  Serial.print("  ");
  delay(100);
  // Setting Blue filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  // Reading the output frequency
  frequency = pulseIn(scanSensor, LOW);
  // Printing the value on the serial monitor
  Serial.print("B= ");//printing name
  Serial.print(frequency);//printing RED color frequency
  Serial.println("  ");
  delay(100);

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

  // Start initial checkings
  digitalWrite(greenLedPin, LOW);
  digitalWrite(yellowLedPin, HIGH);
  digitalWrite(redLedPin, LOW);
  
  // Initial check of servomotors movements
  
  //turn_LoadServo(loadPosition);
  //delay(1000);
  turn_LoadServo(scanPosition);
  delay(1000);
  turn_SelectServo(100);
  delay(1000);

}

void loop() {
  
  digitalWrite(greenLedPin, HIGH);
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(redLedPin, LOW);
  
  turn_LoadServo(loadPosition);
  delay(6000);
  
  turn_LoadServo(scanPosition);
  calibrate();
  delay(1000);
  String color = scanResult();
  Serial.print("Color= ");//printing name
  Serial.println(color);
  
  selectContainer(color);
  //calibrate();
  //calibrate();
  //calibrate();
  //delay(1000);
  
  turn_LoadServo(dropPosition);
  delay(1000);
}
