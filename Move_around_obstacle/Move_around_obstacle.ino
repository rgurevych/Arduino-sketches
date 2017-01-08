// Pin assignments
#define AIN1 3
#define AIN2 4
#define APWM 5
#define BIN1 12
#define BIN2 13
#define BPWM 11
#define STBY 6

// Constants for motor control functions
#define STEPTIME 600 
#define STRAIGHTSPEED 75
#define TURNSPEED 100
#define TURNTIME 500

#include <NewPing.h>

bool obstacleDetected = false;

int ledPin = 13;

#define TRIGGER_PIN  10  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     7   // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 400 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

// Array to track current PWM values for each channel (A and B)
int pwms[] = {APWM, BPWM};

// Offsets to be used to compensate for one motor being more powerful
byte leftOffset = 0;
byte rightOffset = 0;

// Variable to track remaining time
unsigned long pwmTimeout = 0;

// Function to write out pwm values
void writePwms ( int left, int right) {
    analogWrite (pwms[0], left);
    analogWrite (pwms[1], right);
}

// Move the robot forward for STEPTIME
void goForward(){
  digitalWrite(STBY, HIGH);
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  writePwms (STRAIGHTSPEED-leftOffset,STRAIGHTSPEED-rightOffset);
  pwmTimeout = millis() + STEPTIME;
}

// Move the robot backward for STEPTIME
void goBack() {
    digitalWrite(STBY, HIGH);
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    writePwms (STRAIGHTSPEED-leftOffset,STRAIGHTSPEED-rightOffset);
    pwmTimeout = millis() + STEPTIME;
}

// Turn the robot left for TURNTIME
void goLeft () {
    digitalWrite(STBY, HIGH);
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);

    writePwms (TURNSPEED,TURNSPEED);
    pwmTimeout = millis() + TURNTIME;
}

// Turn the robot right for TURNTIME
void goRight () {
    digitalWrite(STBY, HIGH);
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);

    writePwms (TURNSPEED,TURNSPEED);
    pwmTimeout = millis() + TURNTIME;
}

// Stop the robot (using standby)
void stop(){
    digitalWrite(STBY, LOW); 
}

// Arduino setup function
void setup() {
    // Initialize pins as outputs
    pinMode (STBY, OUTPUT);
    pinMode (AIN1, OUTPUT);
    pinMode (AIN2, OUTPUT);
    pinMode (APWM, OUTPUT);
    pinMode (BIN1, OUTPUT);
    pinMode (BIN2, OUTPUT);
    pinMode (BPWM, OUTPUT);
    pinMode (TRIGGER_PIN, OUTPUT);
    pinMode (ECHO_PIN, INPUT);
    Serial.begin(9600);
}

// Loop (code betwen {}'s repeats over and over again)
void loop() {
    // Trigger ultrasonic ping
    int uS = sonar.ping();
    // Calculate distance in cm
    int distance = uS / US_ROUNDTRIP_CM;
    Serial.print("Ping: ");
    Serial.print(distance);
    Serial.println("cm");

    // If there is an object within 10cm of the sensor
    // NOTE: The NewPing library returns 0 if no object is detected.
    if(distance <= 15 && distance != 0){
        // Set the obstacleDetected flag to disable the motors
        obstacleDetected = true;
    }

    // Make the robot go Forward.
    if(obstacleDetected){
      stop();
      delay(200);
      goBack();
      delay(300);
      goRight();
      delay(500);
      obstacleDetected = false;
    }
    else {
      goForward();
      delay(250);
    }
    // Wait for one second
    
    /* // Make the robot stop
    stop();
    delay(200);
    // Make the robot turn right
    goRight();
    // Wait for one second
    delay(250);
    // Make the robot stop
    stop();
    delay(200);
    goForward();
    // Wait for one second
    delay(1000);
    // Make the robot stop
    stop();
    delay(200);
    // Make the robot turn right
    goRight();
    // Wait for one second
    delay(250);
    // Make the robot stop
    stop();
    delay(200);
    goForward();
    // Wait for one second
    delay(1000);
    // Make the robot stop
    stop();
    delay(200);
    // Make the robot turn right
    goRight();
    // Wait for one second
    delay(250);
    // Make the robot stop
    stop();
    delay(200);
    goForward();
    // Wait for one second
    delay(1000);
    // Make the robot stop
    stop();
    delay(200);
    // Make the robot turn right
    goRight();
    // Wait for one second
    delay(250);
    // Make the robot stop
    stop();
    delay(200);
    delay(5000);*/
}
