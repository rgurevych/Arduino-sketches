
void setup() {
  pinMode(RED_BUTTON, INPUT_PULLUP);
  pinMode(BLUE_BUTTON, INPUT_PULLUP);
  pinMode(GREEN_BUTTON, INPUT_PULLUP);
  pinMode(YELLOW_BUTTON, INPUT_PULLUP);
 
  Serial.begin(250000);
  lcd.begin();

  srand((randByte()<<8) | randByte());
#ifdef RESET_HI_SCORE
  EEPROM.put(EEPROM_HI_SCORE, hiScore);
#endif
  EEPROM.get(EEPROM_HI_SCORE, hiScore);
  if(hiScore == 0xFFFF) hiScore = 0;
}

void loop() {
  redButton.tick();
  
  if(!trexSelected && !memoSelected){
    selectScreen();
  }
  
  if(trexSelected){

      RED_LED_FLAG=false, GREEN_LED_FLAG=true, BLUE_LED_FLAG=true, YELLOW_LED_FLAG=false;
      switchLeds();
      firstStart = false;
      gameLoop(hiScore);
      EEPROM.put(EEPROM_HI_SCORE, hiScore);
      gameOver();

  }
}

void switchLeds() {
  digitalWrite(RED_LED, RED_LED_FLAG);
  digitalWrite(GREEN_LED, GREEN_LED_FLAG);
  digitalWrite(BLUE_LED, BLUE_LED_FLAG);
  digitalWrite(YELLOW_LED, YELLOW_LED_FLAG);
}


void selectScreen(){
  RED_LED_FLAG=false, GREEN_LED_FLAG=false, BLUE_LED_FLAG=true, YELLOW_LED_FLAG=false;
  switchLeds();
  
  lcd.setAddressingMode(lcd.HorizontalAddressingMode);
  uint8_t buff[32];
  for(uint8_t i = 0; i < LCD_BYTE_SZIE/sizeof(buff); ++i) {
    memcpy_P(buff, select_screen_bitmap + 2 + uint16_t(i) * sizeof(buff), sizeof(buff));
    lcd.fillScreen(buff, sizeof(buff));
  }

  while(1){
    blueButton.tick();
    greenButton.tick();
    
    if(blinkTimer.isReady()){
      blinkTimer.start();
      GREEN_LED_FLAG = !GREEN_LED_FLAG;
      BLUE_LED_FLAG = !BLUE_LED_FLAG;
      switchLeds();
    }
    
    if(blueButton.isPress()){
      trexSelected=true;
      memoSelected=false;
      splashScreen();
      lcd.setAddressingMode(LCD_IF_VIRTUAL_WIDTH(lcd.VerticalAddressingMode, lcd.HorizontalAddressingMode));
      return;
    }
    if(greenButton.isPress()){
      trexSelected=false;
      memoSelected=true;
      splashScreen();
      delay(1000);
      memoSelected=false;
      return;     
    }
  }
}

void gameOver(){
  RED_LED_FLAG=true, GREEN_LED_FLAG=false, BLUE_LED_FLAG=false, YELLOW_LED_FLAG=false;
  switchLeds();
  
  while(1){
    blueButton.tick();
    redButton.tick();
    
    if(blinkTimer.isReady()){
      blinkTimer.start();
      RED_LED_FLAG = !RED_LED_FLAG;
      BLUE_LED_FLAG = !BLUE_LED_FLAG;
      switchLeds();
    }
    
    if(blueButton.isPress()){
      return;
    }
    
    if(redButton.isPress()){
      trexSelected=false;
      return;
    }
  }
}

void splashScreen() {
  RED_LED_FLAG=true, GREEN_LED_FLAG=true, BLUE_LED_FLAG=true, YELLOW_LED_FLAG=true;
  switchLeds();
  
  lcd.setAddressingMode(lcd.HorizontalAddressingMode);
  uint8_t buff[32];
  for(uint8_t i = 0; i < LCD_BYTE_SZIE/sizeof(buff); ++i) {
    if(trexSelected){
      memcpy_P(buff, splash_screen_bitmap + 2 + uint16_t(i) * sizeof(buff), sizeof(buff));
      lcd.fillScreen(buff, sizeof(buff));
    }
    
    else if(memoSelected){
      memcpy_P(buff, memo_splash_screen_bitmap + 2 + uint16_t(i) * sizeof(buff), sizeof(buff));
      lcd.fillScreen(buff, sizeof(buff));
    }
    
  }
  for(uint8_t i = 20; i; --i) delay(100);
}
