
void setup() {
  pinMode(RED_BUTTON, INPUT_PULLUP);
  pinMode(BLUE_BUTTON, INPUT_PULLUP);
  pinMode(GREEN_BUTTON, INPUT_PULLUP);
  pinMode(YELLOW_BUTTON, INPUT_PULLUP);

  RED_LED_FLAG=true, GREEN_LED_FLAG=false, BLUE_LED_FLAG=false, YELLOW_LED_FLAG=false;
  switchLeds();
  delay(1500);

  RED_LED_FLAG=false, GREEN_LED_FLAG=true, BLUE_LED_FLAG=false, YELLOW_LED_FLAG=false;
  switchLeds();
  delay(1500);

  RED_LED_FLAG=false, GREEN_LED_FLAG=false, BLUE_LED_FLAG=true, YELLOW_LED_FLAG=false;
  switchLeds();
  delay(1500);

  RED_LED_FLAG=false, GREEN_LED_FLAG=false, BLUE_LED_FLAG=false, YELLOW_LED_FLAG=true;
  switchLeds();
  delay(1500);
  
  
  Serial.begin(250000);
  lcd.begin();
  selectScreen();

  spalshScreen();

  
  lcd.setAddressingMode(LCD_IF_VIRTUAL_WIDTH(lcd.VerticalAddressingMode, lcd.HorizontalAddressingMode));
  srand((randByte()<<8) | randByte());
#ifdef RESET_HI_SCORE
  EEPROM.put(EEPROM_HI_SCORE, hiScore);
#endif
  EEPROM.get(EEPROM_HI_SCORE, hiScore);
  if(hiScore == 0xFFFF) hiScore = 0;
}

void loop() {
  switchLeds();
  if(firstStart || isPressedJump()) {
    RED_LED_FLAG=false, GREEN_LED_FLAG=true, BLUE_LED_FLAG=true, YELLOW_LED_FLAG=false;
    switchLeds();
    firstStart = false;
    gameLoop(hiScore);
    EEPROM.put(EEPROM_HI_SCORE, hiScore);
    //wait until the jump button is released
    while(isPressedJump()) delay(100);
    delay(500);
  }
}

void switchLeds() {
  digitalWrite(RED_LED, RED_LED_FLAG);
  digitalWrite(GREEN_LED, GREEN_LED_FLAG);
  digitalWrite(BLUE_LED, BLUE_LED_FLAG);
  digitalWrite(YELLOW_LED, YELLOW_LED_FLAG);
}
