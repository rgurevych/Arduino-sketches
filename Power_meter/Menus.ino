void mainMenu(){
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(5, 0);  lcd.print(F("Main menu"));
    lcd.setCursor(1, 1);  lcd.print(F("Back"));
    lcd.setCursor(1, 2);  lcd.print(F("Meter"));
    lcd.setCursor(1, 3);  lcd.print(F("Charts"));
    lcd.setCursor(11,1);  lcd.print(F("Min/max"));
    lcd.setCursor(11,2);  lcd.print(F("Settings"));
    setMenuCursor(); lcd.print(F(">"));
    screenReadyFlag = true;
    }

  if(enc.right()) {
    setMenuCursor();
    lcd.print(F(" "));
    menu += 1;
    if (menu > 5) menu = 1;
    setMenuCursor();
    lcd.print(F(">"));
  }

  if(enc.left()) {
    setMenuCursor();
    lcd.print(F(" "));
    menu -= 1;
    if (menu < 1) menu = 5;
    setMenuCursor();
    lcd.print(F(">"));
  }
  
  if(enc.click()){
    if(menu == 1){
      mode = 0;
      screenReadyFlag = false;
      screen = 1;
    }

    if(menu == 2){
      mode = 2;
      screenReadyFlag = false;

    }

    if(menu == 5){
      mode = 5;
      screenReadyFlag = false;
      menu = 1;
    }
    enc.resetState();
    menuExitTimer.start();
  }
}


void setMenuCursor() {
  if (menu < 4) lcd.setCursor(0, menu);
  else lcd.setCursor(10, menu-3);
  menuExitTimer.start();
}


void settingsMenu(){
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(6, 0);  lcd.print(F("Settings"));
    lcd.setCursor(1, 1);  lcd.print(F("Back"));
    lcd.setCursor(1, 2);  lcd.print(F("Time&Date"));
    lcd.setCursor(1, 3);  lcd.print(F("Set meter"));
    lcd.setCursor(11,1);  lcd.print(F("Bright"));
    lcd.setCursor(11,2);  lcd.print(F("Mode"));
    lcd.setCursor(11,3);  lcd.print(F("Reset"));
    setMenuCursor(); lcd.print(F(">"));
    screenReadyFlag = true;
    }

  if(enc.right()) {
    setMenuCursor();
    lcd.print(F(" "));
    menu += 1;
    if (menu > 6) menu = 1;
    setMenuCursor();
    lcd.print(F(">"));
  }

  if(enc.left()) {
    setMenuCursor();
    lcd.print(F(" "));
    menu -= 1;
    if (menu < 1) menu = 6;
    setMenuCursor();
    lcd.print(F(">"));
  }
  
  if(enc.click()){
    if(menu == 1){
      mode = 0;
//      menu = 5;
//      screenReadyFlag = false;
//      screen = 1;
      Serial.println(mode);
      Serial.println(menu);
//      enc.resetState();
    }

    if(menu == 2){
      mode = 6;
      menu = 1;
      screenReadyFlag = false;
    }

    if(menu == 3){
      mode = 7;
      menu = 1;
      screenReadyFlag = false;
    }

    if(menu == 4){
      mode = 8;
      menu = 1;
      screenReadyFlag = false;
    }

    if(menu == 5){
      mode = 9;
      menu = 1;
      screenReadyFlag = false;
    }

//    enc.resetState();
//    menuExitTimer.start();
  }
}


void timeSetMenu(byte menu){
  lcd.setCursor(1, 3);  lcd.print(F(" "));
  lcd.setCursor(11, 3); lcd.print(F(" "));
  
  switch(menu){
    case 1:
      lcd.setCursor(5,1);
      lcd.print(F("  "));
      break;

    case 2:
      lcd.setCursor(8,1);
      lcd.print(F("  "));
      break;

    case 3:
      lcd.setCursor(11,1);
      lcd.print(F("  "));
      break;

    case 4:
      lcd.setCursor(5,2);
      lcd.print(F("  "));
      break;

    case 5:
      lcd.setCursor(8,2);
      lcd.print(F("  "));
      break;

    case 6:
      lcd.setCursor(11,2);
      lcd.print(F("  "));
      break;

    case 7:
      lcd.setCursor(1, 3);
      lcd.print(F(">"));
      break;

    case 8:
      lcd.setCursor(11, 3);
      lcd.print(F(">"));
      break;
  }
}


void meterSetMenu(byte menu){
  lcd.setCursor(1, 3);  lcd.print(F(" "));
  lcd.setCursor(11, 3); lcd.print(F(" "));
  lcd.setCursor(15, 1); lcd.print(F(" "));
  lcd.setCursor(15, 2); lcd.print(F(" "));
  
  switch(menu){
    case 1:
      lcd.setCursor(15,1);
      if(blinkFlag) lcd.print(F("<")); 
      break;

    case 2:
      lcd.setCursor(15,2);
      if(blinkFlag) lcd.print(F("<")); 
      break;

    case 3:
      lcd.setCursor(1, 3);
      lcd.print(F(">"));
      break;

    case 4:
      lcd.setCursor(11, 3);
      lcd.print(F(">"));
      break;
  }
}


void brightSetMenu(byte menu){
  lcd.setCursor(1, 3);  lcd.print(F(" "));
  lcd.setCursor(11, 3); lcd.print(F(" "));
  
  switch(menu){
    case 1:
      lcd.setCursor(12,1); lcd.print(F(" "));
      break;

    case 2:
      lcd.setCursor(1, 3);
      lcd.print(F(">"));
      break;

    case 3:
      lcd.setCursor(11, 3);
      lcd.print(F(">"));
      break;
  }
}

//void modeSetMenu(byte menu){
//  lcd.setCursor(1, 3);  lcd.print(F(" "));
//  lcd.setCursor(11, 3); lcd.print(F(" "));
//  
//  switch(menu){
//    case 1:
//      lcd.setCursor(12,1); lcd.print(F("   "));
//      break;
//
//    case 2:
//      lcd.setCursor(1, 3);
//      lcd.print(F(">"));
//      break;
//
//    case 3:
//      lcd.setCursor(11, 3);
//      lcd.print(F(">"));
//      break;
//  }
//}
