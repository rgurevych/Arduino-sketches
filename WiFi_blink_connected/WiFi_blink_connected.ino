#include <WiFiUdp.h>

#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>

#include <NewPing.h>

#define TRIGGER_PIN  10
#define ECHO_PIN     7
#define MAX_DISTANCE 200

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// Pin assignments
#define AIN1 3
#define AIN2 4
#define APWM 5
#define BIN1 12
#define BIN2 13
#define BPWM 11
#define STBY 6

int x_speed = 0;
int y_speed = 0;
int distance = 0;

enum motor_state_t {
    STOP,
    FORWARD,
    BACKWARD
};

void setLeftMotor(motor_state_t direction, uint8_t speed){
    if(direction == STOP){
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, LOW);
    }else if(direction == BACKWARD){
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
    }else if(direction == FORWARD){
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
    }
    analogWrite (APWM, speed);
}

void setRightMotor(int8_t direction, uint8_t speed){
    if(direction == STOP){
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, LOW);
    }else if(direction == BACKWARD){
        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
    }else if(direction == FORWARD){
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
    }
    analogWrite (BPWM, speed);
}

void stopRobot(){
    digitalWrite(STBY, LOW);
}

void goRobot(){
    digitalWrite(STBY, HIGH);
}


// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "74d179e9421340ca9146557bd0604c34";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "FRITZ!Box_Gurevych";
char pass[] = "04426925845952733255";

// Hardware Serial on Mega, Leonardo, Micro...
//#define EspSerial Serial1

// or Software Serial on Uno, Nano...
#include <SoftwareSerial.h>
SoftwareSerial EspSerial(8, 9); // RX, TX

// Your ESP8266 baud rate:
#define ESP8266_BAUD 9600

ESP8266 wifi(&EspSerial);

void setup()
{
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
    // Set console baud rate
    Serial.begin(9600);
    delay(10);
    // Set ESP8266 baud rate
    EspSerial.begin(ESP8266_BAUD);
    delay(10);

    Blynk.begin(auth, wifi, ssid, pass);
}

BLYNK_READ(V5) {
    int uS = sonar.ping();
    distance = uS / US_ROUNDTRIP_CM;
    Serial.print("Ping: ");
    Serial.print(distance);
    Serial.println("cm");
    // Write to Blynk Virtual Pin 5
    Blynk.virtualWrite(V5, distance);
}

BLYNK_READ(V0){
    Blynk.virtualWrite(V0, 100);
}

BLYNK_WRITE(V2) {
    x_speed = param.asInt();
    Serial.print("x_speed: ");
    Serial.println(x_speed);
}

BLYNK_WRITE(V3) {
    y_speed = param.asInt();
    Serial.print("y_speed: ");
    Serial.println(y_speed);
}

BLYNK_WRITE(V1) {
    // Joystick range is 0 to 1024. Resting zero position of the joystick is 512
    const int ZERO_POSITION = 512;

    // Use a threshold to account for near zero position
    const int THRESHOLD_OFFSET = 90;

    // speed for a hard clouckwise/counterclockwise turn
    const int TURN_SPEED = 120;

    // Parameters received from Blynk app
    int x = param[0].asInt();
    int y = param[1].asInt();

    // Direction variables indicating direction of joystick movement
    // Possible values: -1, 0, 1
    int x_direction;
    int y_direction;

    // Print x and y values for debugging
    Serial.print("X = ");
    Serial.print(x);
    Serial.print("; Y = ");
    Serial.println(y);

    // Determine direction of the joystick
    x_direction = 0;
    y_direction = 0;

    if(x > (ZERO_POSITION + THRESHOLD_OFFSET)){
        x_direction = 1;
    }else if(x < (ZERO_POSITION - THRESHOLD_OFFSET)){
        x_direction = -1;
    }

    if(y > (ZERO_POSITION + THRESHOLD_OFFSET)){
        y_direction = 1;
    }else if(y < (ZERO_POSITION - THRESHOLD_OFFSET)){
        y_direction = -1;
    }

    // Move the WiFiBot according to the x and y direction:
    // STOP:       x= 0, y= 0
    // FORWARD:    x= 0, y= 1
    // BACKWARD:   x= 0, y=-1
    // RIGHT UP:   x= 1, y= 1
    // RIGHT:      x= 1, y= 0
    // RIGHT DOWN: x= 1, y=-1
    // LEFT UP:    x=-1, y=-1
    // LEFT:       x=-1, y= 0
    // LEFT DOWN:  x=-1, y=-1

    if (x_direction == 0 && y_direction ==0){
        // STOP: x= 0, y= 0
        stopRobot();
        setLeftMotor(STOP, 0);
        setRightMotor(STOP, 0);
        Serial.println("STOP");
    } else {
        // Robot is moving, set STBY pin accordingly using the goRobot function
        goRobot();
        Serial.print("Go ");

        // Set the direction and speed of the motors accordingly
        if(x_direction == 0){

            if(y_direction == 1){
                Serial.println("Forward");
                // FORWARD: x= 0, y= 1
                setLeftMotor(FORWARD, x_speed);
                setRightMotor(FORWARD, y_speed);
            }else if(y_direction == -1){
                // BACKWARD:   x= 0, y=-1
                Serial.println("Backward");
                setLeftMotor(BACKWARD, x_speed);
                setRightMotor(BACKWARD, y_speed);
            }
            //
        }else if(x_direction == 1){
            if(y_direction == 1){
                // RIGHT UP:   x= 1, y= 1
                Serial.println("Right up");
                setLeftMotor(FORWARD, x_speed);
                setRightMotor(STOP, y_speed);
            }else if(y_direction == 0){
                // RIGHT:      x= 1, y= 0
                Serial.println("Right");
                setLeftMotor(FORWARD, TURN_SPEED);
                setRightMotor(BACKWARD, TURN_SPEED);
            }else if(y_direction == -1){
                // RIGHT DOWN: x= 1, y=-1
                Serial.println("Right down");
                setLeftMotor(BACKWARD, x_speed);
                setRightMotor(STOP, y_speed);
            }
        }else if(x_direction == -1){
            if(y_direction == 1){
                // LEFT UP:    x=-1, y=-1
                Serial.println("Left up");
                setLeftMotor(STOP, x_speed);
                setRightMotor(FORWARD, y_speed);
            }else if(y_direction == 0){
                // LEFT:       x=-1, y= 0
                Serial.println("Left");
                setLeftMotor(BACKWARD, TURN_SPEED);
                setRightMotor(FORWARD, TURN_SPEED);
            }else if(y_direction == -1){
                // LEFT DOWN:  x=-1, y=-1
                Serial.println("Left down");
                setLeftMotor(STOP, x_speed);
                setRightMotor(BACKWARD, y_speed);
            }
        }
    }
}

void loop()
{
    Blynk.run();
}
