// ---------------------------------------------------------------------------
// Example NewPing library sketch that does a ping about 20 times per second.
// ---------------------------------------------------------------------------

#include <NewPing.h>

int ledPin = 13;

#define TRIGGER_PIN  10  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     7   // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 400 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

int blink_delay = 1000;
int finalizer = 0;

void blink(int distance) {
  if (distance <= 50 && distance > 0){
    finalizer = 0;
    while (finalizer < 1000) {
      blink_delay = distance * 10;
      digitalWrite(ledPin, HIGH);
      delay(blink_delay);
      digitalWrite(ledPin, LOW);
      delay(blink_delay);
      finalizer = finalizer + (blink_delay * 2);
    }
  }
  else {
    digitalWrite(ledPin, LOW);
    delay(500);
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  blink(sonar.ping_cm()); 
}


