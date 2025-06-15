//Relay tester by Rostyslav Gurevych

#define RELAY_PIN 2
bool current_state, new_state;
long timer;

void setup() {
  pinMode(RELAY_PIN, INPUT_PULLUP);

  //Wire.begin();
  Serial.begin(9600);
  Serial.print("Initial state: ");
  current_state = digitalRead(RELAY_PIN);
  if (current_state) Serial.println("Open");
  else Serial.println("Closed");
  timer = micros();
}

void loop() {
  new_state = digitalRead(RELAY_PIN);
  if (new_state != current_state) {
    long duration = micros()-timer;
    current_state = new_state;
    if (duration > 1000000) Serial.println("-----");
    if (current_state) Serial.print("Closed");
    else Serial.print("Open");
    Serial.print(" for ");
    Serial.print(duration);
    Serial.println(" mcs");
    timer = micros();
  }

}
