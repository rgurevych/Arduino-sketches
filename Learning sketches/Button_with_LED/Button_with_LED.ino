//This is my first program
//It should use button to switch on the LED

int switchPin = 8;
int ledPin = 11;
boolean lastButton = LOW;
boolean currentButton = LOW;
int ledLevel = 0;

boolean debounce(boolean last)
  {
    boolean current = digitalRead(switchPin);
    if(last != current) 
    {
      delay(5);
      current = digitalRead(switchPin);  
    }
    return current;
  }

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  pinMode(switchPin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  currentButton = debounce(lastButton);
  if (lastButton == LOW && currentButton == HIGH) 
  {
    ledLevel = ledLevel + 51;
   }
 lastButton = currentButton;

 if(ledLevel > 255)
 {
  ledLevel = 0;
 }
 analogWrite(ledPin, ledLevel);
}
