int ledPin = 13;

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (Serial.available() == 0);
  
  int val = Serial.read() - '0';

  if (val == 1){
    Serial.println("The LED is ON");
    digitalWrite(ledPin, HIGH);
  }
  else if (val == 0){
    Serial.println("The LED is OFF");
    digitalWrite(ledPin, LOW);
  }
  else{
    Serial.println("Invalid command");
  }
  delay(50);
  while(Serial.available()) Serial.read(); //this will delete the rest of trash in the serial port (instead of Serial.flush())
  
}
