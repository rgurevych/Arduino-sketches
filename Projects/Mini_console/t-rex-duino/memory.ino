//Memory game

const int STATE_START_GAME        = 0;  // Initial state
const int STATE_PICK_RND_SEQUENCE = 1;  // Pick a random sequence of LEDs
const int STATE_SHOW_RND_SEQUENCE = 2;  // Show the randomly selected sequence of LED flashes
const int STATE_READ_PLAYER_GUESS = 3;  // Read the player's guess
const int STATE_VERIFY_GUESS      = 4;  // Check the guess against the random sequence
const int STATE_GUESS_CORRECT     = 5;  // Player guessed correctly
const int STATE_GUESS_INCORRECT   = 6;  // Player guessed incorrectly

const int MAX_DIFFICULTY_LEVEL    = 9;  // Maximum difficulty level (LED flash sequence length)

// Array used to store the generated random sequence
int randomSequence[MAX_DIFFICULTY_LEVEL];

// Array used to store the player's guess
int playerGuess[MAX_DIFFICULTY_LEVEL];
// A counter to record the number of button presses made by the player
int numButtonPresses;

// The current state the game is in
int currentState;
int nextState;

// The difficulty level (1..MAX_DIFFICULTY_LEVEL)
// (Do not set to zero!)
int difficultyLevel;

void memoGameLoop() {
  
  
  
  // Set initial difficulty level to 1 random flash && put game in start state
  difficultyLevel = 1;
  currentState = STATE_START_GAME;
  nextState = currentState;
  numButtonPresses = 0;

  //main cycle
  while(1) {
    // Run the state machine controlling the game's states
    switch( currentState ) {
      case STATE_START_GAME: 
        delay(5000); // Give player time to get ready
        nextState = STATE_PICK_RND_SEQUENCE; 
        break;
      case STATE_PICK_RND_SEQUENCE:
        generateRndSequence();
        nextState = STATE_SHOW_RND_SEQUENCE;
        break; 
      case STATE_SHOW_RND_SEQUENCE:
        showRndSequence();  // Display the rnd sequence to the player
        nextState = STATE_READ_PLAYER_GUESS;
        break;
      case STATE_READ_PLAYER_GUESS:
        readPlayerGuess();  // Poll buttons and record each button press
        // If all inputs have been made then verify the guess
        if( numButtonPresses >= difficultyLevel ) {
          numButtonPresses = 0;
          nextState = STATE_VERIFY_GUESS;
        }
        break;
      case STATE_VERIFY_GUESS:
        // Check player's guess against the generated random sequence
        if( verifyGuess() ) {
          nextState = STATE_GUESS_CORRECT;
        } else {
          nextState = STATE_GUESS_INCORRECT;
        }
        break;
      case STATE_GUESS_CORRECT:
        // Player was right. Increase difficulty level and goto start game
        soundCorrectGuess();
        difficultyLevel++;
        if( difficultyLevel > MAX_DIFFICULTY_LEVEL )
          difficultyLevel = MAX_DIFFICULTY_LEVEL;
        nextState = STATE_START_GAME;
        break;
      case STATE_GUESS_INCORRECT:
        // Player was wrong. Sound buzzer, show correct sequence, set difficulty level to 1 and re-start game
        soundBuzzer();
        showRndSequence();
        difficultyLevel = 1;
        nextState = STATE_START_GAME;
        
        break;
    }
    currentState = nextState;
  }
}

// Generate a random sequence of LED pin numbers
void generateRndSequence() {
  for(int i=0; i<difficultyLevel; i++) {
    randomSequence[i] = rndLEDPin();
  }
}

// Show random sequence
void showRndSequence() {
  for(int i=0; i<difficultyLevel; i++) {
    flashLED(randomSequence[i], true);  
  }
}

// Read a button press representing a guess from player
void readPlayerGuess() {
  if( digitalRead(YELLOW_BUTTON) == LOW ) {
    playerGuess[numButtonPresses] = YELLOW_LED;
    numButtonPresses++;
    flashLED(YELLOW_LED, true);
    blockUntilRelease(YELLOW_BUTTON);
  } else if( digitalRead(BLUE_BUTTON) == LOW ) {
    playerGuess[numButtonPresses] = BLUE_LED;
    numButtonPresses++;
    flashLED(BLUE_LED, true);
    blockUntilRelease(BLUE_BUTTON);
  }else if( digitalRead(GREEN_BUTTON) == LOW ) {
    playerGuess[numButtonPresses] = GREEN_LED;
    numButtonPresses++;
    flashLED(GREEN_LED, true);
    blockUntilRelease(GREEN_BUTTON);
  }else if( digitalRead(RED_BUTTON) == LOW ) {
    playerGuess[numButtonPresses] = RED_LED;
    numButtonPresses++;
    flashLED(RED_LED, true);
    blockUntilRelease(RED_BUTTON);

  }
}

void blockUntilRelease(int buttonNumber) {
  while( digitalRead(buttonNumber) == LOW )
    ;
}
// Compare the guess with the random sequence and return true if identical
bool verifyGuess() {
  bool identical = true;
  for(int i=0; i<difficultyLevel; i++) {
    if( playerGuess[i] != randomSequence[i] ) {
      identical = false;
      break;
    }
  }
  return identical;
}

// Sound the buzzer for incorrect guess
void soundBuzzer() {
  tone(BUZZ_PIN, 100, 2000);
  delay(2300);  
}

void soundCorrectGuess() {
  tone(BUZZ_PIN, 700, 100);
  delay(100);
  tone(BUZZ_PIN, 800, 100);
  delay(100);
  tone(BUZZ_PIN, 900, 100);
  delay(100);
  tone(BUZZ_PIN, 1000, 100);
  delay(100);
  tone(BUZZ_PIN, 1100, 100);
  delay(100);
  tone(BUZZ_PIN, 1200, 100);
  delay(100);
}
// Flash the LED on the given pin
void flashLED(int ledPinNum, bool playSound) {
  digitalWrite(ledPinNum, HIGH);
  if( playSound )
    playSoundForLED(ledPinNum);
  delay(1000);
  digitalWrite(ledPinNum, LOW);
  delay(500);
}

void playSoundForLED(int ledPinNum) {
  int pitch = 0;
  switch(ledPinNum) {
    case YELLOW_LED: pitch = 1480; break;
    case BLUE_LED: pitch = 1109; break;
    case GREEN_LED: pitch = 1175; break;
    case RED_LED: pitch = 1319; break;
  }
  tone(BUZZ_PIN, pitch, 800);
}
// Get a random LED pin number
int rndLEDPin() {
  return random(RED_LED, YELLOW_LED + 1);
}
