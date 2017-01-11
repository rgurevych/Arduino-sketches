int sensorPin = 0;
int ledPin = 11;

void setup() {
  // put your setup code here, to run once:
  pinMode(sensorPin, INPUT);
  pinMode(ledPin, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  int val = analogRead(sensorPin);
  //val = constrain(val, 700, 900);
  int ledLevel = map(val, 1023, 0, 255, 0);
  
  analogWrite(ledPin, ledLevel);

}
