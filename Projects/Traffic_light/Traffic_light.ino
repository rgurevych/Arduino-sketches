
int buttonPin = 2;


int buttonLed = 3;
int pedGreen = 4;
int pedRed = 5;
int mainGreen = 8;
int mainYellow = 9;
int mainRed = 10;
int buzzPin = 7;
boolean lastButton = LOW;
int delayInUse = 10000;
long lastUse = delayInUse;


void switchlight(){
  digitalWrite(buttonLed, LOW);
  for(int i=0; i<=3; i++){
    digitalWrite(mainGreen, LOW);
    delay(500);
    digitalWrite(mainGreen, HIGH);
    delay(500);
  }
  
  digitalWrite(mainGreen, LOW);
  digitalWrite(mainYellow, HIGH);
  delay(2000);

  digitalWrite(mainYellow, LOW);
  digitalWrite(mainRed, HIGH);
  delay(500);
  digitalWrite(pedRed, LOW);
  digitalWrite(pedGreen, HIGH);
    for(int j=0; j<10; j++){
      for(int i=0;i<40;i++)
      {
        digitalWrite(buzzPin,HIGH);
        delay(1);
        digitalWrite(buzzPin,LOW);
        delay(1);
      }
      delay(450);
    }
  //delay(5000);

  for(int i=0; i<=3; i++){
    digitalWrite(pedGreen, LOW);
    delay(500);
    digitalWrite(pedGreen, HIGH);
    for(int i=0;i<40;i++)
      {
        digitalWrite(buzzPin,HIGH);
        delay(1);
        digitalWrite(buzzPin,LOW);
        delay(1);
      }
    delay(450);
  }

  digitalWrite(pedGreen, LOW);
  digitalWrite(pedRed, HIGH);
  digitalWrite(mainYellow, HIGH);
  delay(2000);
  digitalWrite(mainRed, LOW);
  digitalWrite(mainYellow, LOW);
  digitalWrite(mainGreen, HIGH);

  lastUse = millis();
}

void setup() {
  // initiate pins:
  pinMode(buttonPin, INPUT);
  pinMode(buttonLed, OUTPUT);
  pinMode(pedGreen, OUTPUT);
  pinMode(pedRed, OUTPUT);
  pinMode(mainGreen, OUTPUT);
  pinMode(mainYellow, OUTPUT);
  pinMode(mainRed, OUTPUT);
  pinMode(buzzPin, OUTPUT);

  // switch on the traffic light in safe mode (both red first)
  digitalWrite(mainRed, HIGH);
  digitalWrite(pedRed, HIGH);
  delay(2000);
  digitalWrite(mainYellow, HIGH);
  delay(2000);
  digitalWrite(mainRed, LOW);
  digitalWrite(mainYellow, LOW);
  digitalWrite(mainGreen, HIGH);
}

void loop() {
  //check if the button is pressed
  if (digitalRead(buttonPin) == HIGH && lastButton == LOW)
  {
    analogWrite(buttonLed, 100);
    if(millis() - lastUse > delayInUse){
      delay(2000);
    }
    else{
      delay(delayInUse - millis() + lastUse);
    }
    lastButton = HIGH;
    switchlight();
  }
  else
  {
    lastButton = LOW;
  }
}
