
void setup() {
  pinMode(JUMP_BUTTON, INPUT_PULLUP);
  pinMode(DUCK_BUTTON, INPUT_PULLUP);
  Serial.begin(250000);
  lcd.begin();
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
  if(firstStart || isPressedJump()) {
    firstStart = false;
    gameLoop(hiScore);
    EEPROM.put(EEPROM_HI_SCORE, hiScore);
    //wait until the jump button is released
    while(isPressedJump()) delay(100);
    delay(500);
  }
}
