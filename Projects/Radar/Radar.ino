// Includes the libraries
#include <Servo.h>
#include <NewPing.h>

// Defines Tirg and Echo pins of the Ultrasonic Sensor
const int trigPin = 10;  // Arduino pin tied to trigger pin on the ultrasonic sensor.
const int echoPin = 11;  // Arduino pin tied to echo pin on the ultrasonic sensor.
const int maxDistance = 200; // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

// Variables for the duration and the distance
long duration;
int distance;
const int turnSpeed = 60; // delay in ms for each degree value

Servo myServo; // Creates a servo object for controlling the servo motor

NewPing sonar(trigPin, echoPin, maxDistance); // NewPing setup of pins and maximum distance.

void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.begin(9600);
  myServo.attach(12); // Defines on which pin is the servo motor attached
}

void loop() {
  // rotates the servo motor from 15 to 165 degrees
  for(int i=15; i<=165; i++){  
  myServo.write(i);
  delay(turnSpeed);
  distance = sonar.ping_cm();// Calls a function for calculating the distance measured by the Ultrasonic sensor for each degree
  if(distance == 0) distance = maxDistance;
  Serial.print(i); // Sends the current degree into the Serial Port
  Serial.print(","); // Sends addition character right next to the previous value needed later in the Processing IDE for indexing
  Serial.print(distance); // Sends the distance value into the Serial Port
  Serial.print("."); // Sends addition character right next to the previous value needed later in the Processing IDE for indexing
  }
  // Repeats the previous lines from 165 to 15 degrees
  for(int i=165; i>15; i--){  
  myServo.write(i);
  delay(turnSpeed);
  distance = sonar.ping_cm();
  if(distance == 0) distance = maxDistance;
  Serial.print(i);
  Serial.print(",");
  Serial.print(distance);
  Serial.print(".");
  }
}
