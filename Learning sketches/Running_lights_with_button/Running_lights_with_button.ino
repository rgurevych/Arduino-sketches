/*
 * Running lights. Press the button to start, press the button again to stop.
 */

int timer = 50;                   // The higher the number, the slower the timing.
int pins[] = { 2, 3, 4, 5, 6, 7 }; // an array of pin numbers
int num_pins = 6;                  // the number of pins (i.e. the length of the array)
int buttonPin = 8;
boolean runStart = false;
boolean lastButton = LOW;
boolean currentButton = LOW;

boolean debounce(boolean last)
  {
  boolean current = digitalRead(buttonPin);
  if(current =! last)
    {
      delay(5);
      current = digitalRead(buttonPin);
    }
  return current;
}

void run_leds(boolean flag){
  if(flag == true){
  int i;
  for (i = 0; i < num_pins; i++) { // loop through each pin...
    digitalWrite(pins[i], HIGH);   // turning it on,
    delay(timer);                  // pausing,
    digitalWrite(pins[i], LOW);    // and turning it off.
    currentButton = debounce(lastButton);
    if(currentButton == HIGH && lastButton == LOW) runStart = false;
  }
  for (i = num_pins - 1; i >= 0; i--) { 
    digitalWrite(pins[i], HIGH);
    delay(timer);
    digitalWrite(pins[i], LOW);
    currentButton = debounce(lastButton);
    if(currentButton == HIGH && lastButton == LOW) runStart = false;
  }
  }
}

void setup()
{
  int i;

  for (i = 0; i < num_pins; i++)   // the array elements are numbered from 0 to num_pins - 1
    pinMode(pins[i], OUTPUT);      // set each pin as an output
  pinMode(buttonPin, INPUT);       // set the button pin as input
}

void loop()
{
  currentButton = debounce(lastButton);
  if(currentButton == HIGH && lastButton == LOW) runStart = true;
  lastButton = currentButton;
  run_leds(runStart);
}
