int temperaturePin = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int value = analogRead(temperaturePin);
  int voltage = map(value, 0, 1023, 0, 5000);
  int c = round((voltage - 500.0)/10.2);
  int f = round(c*9.0/5.0 + 32.0);
  Serial.print(c);
  Serial.print("C,");
  Serial.print(f);
  Serial.print("F .");
  delay(500);
}
