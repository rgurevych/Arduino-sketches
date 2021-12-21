void getPowerData(){
  if (measureTimer.isReady()){
    if (DEMO_MODE){
      generatePowerData();
    }
    else {
      measurePower();
    }
  }
}


void measurePower(){  
  if (pzem.readAddress() == 0){
    if (DEBUG_MODE) {
      Serial.println(F("The meter is not powered!"));
    }
    if (mode == 0 && screen == 1) {
      screen = 0;
      screenReadyFlag = false;
    }    
    mom_voltage = 0;
    mom_current = 0;
    mom_power = 0;
    mom_frequency = 0;
    mom_pf = 0;
  }
  else {
    readPowerData();
    if (mode == 0 && screen == 0) {
      screen = 1;
      screenReadyFlag = false;
    }
  }
}


void readPowerData() {
  mom_voltage = pzem.voltage() * 10;
  mom_current = round(pzem.current() * 100);
  mom_power = round(pzem.power());
  mom_energy = round(pzem.energy() * 10);
  mom_frequency = pzem.frequency() * 10;
  mom_pf = pzem.pf() * 100;
  if (isnan(mom_voltage)) {
    mom_voltage = 0;
  }
}


void generatePowerData(){
  mom_voltage = 2100 + random(200);
  mom_current = random(1500);
  mom_power = round((mom_voltage / 10.0) * (mom_current / 100.0));
  mom_energy += mom_power / 1800;
  mom_frequency = random(498, 502);
  mom_pf = random(90, 101);
}


void getTime() {
  now = rtc.now();
  second = now.second();
  min = now.minute();
  hour = now.hour();
  day = now.day();
  month = now.month();
  year = now.year();
}


void backgroundTime(){
  if (timeTimer.isReady()){
    getTime();
  }
}


void setTime(){
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(2, 0);  lcd.print(F("Set time&date"));
    lcd.setCursor(0, 1);  lcd.print(F("Time:")); lcd.setCursor(7, 1); lcd.print(F(":")); lcd.setCursor(10,1); lcd.print(F(":"));
    lcd.setCursor(0, 2);  lcd.print(F("Date:")); lcd.setCursor(7, 2); lcd.print(F("/")); lcd.setCursor(10,2); lcd.print(F("/"));
    lcd.setCursor(2, 3);  lcd.print(F("Back"));
    lcd.setCursor(12, 3);  lcd.print(F("Save"));
    
    new_hour = hour;
    new_min = min;
    new_second = second;
    new_day = day;
    new_month = month;
    new_year = year - 2000;
    screenReadyFlag = true;
    }

  if(printTimer.isReady()) {
    lcd.setCursor(5,1); if (new_hour < 10){lcd.print(F("0"));} lcd.print(new_hour); 
    lcd.setCursor(8,1); if (new_min < 10){lcd.print(F("0"));} lcd.print(new_min); 
    lcd.setCursor(11,1); if (new_second < 10){lcd.print(F("0"));} lcd.print(new_second);
    lcd.setCursor(5,2); if (new_day < 10){lcd.print(F("0"));} lcd.print(new_day); 
    lcd.setCursor(8,2); if (new_month < 10){lcd.print(F("0"));} lcd.print(new_month); 
    lcd.setCursor(11,2); lcd.print(new_year);
    blinkFlag = !blinkFlag;
    if(blinkFlag) 
      timeSetMenu(menu);
  }
  
  if(enc.right()) {
    menu += 1;
    if (menu > 8) menu = 1;
    timeSetMenu(menu);
    menuExitTimer.start();
  }

  if(enc.left()) {
    menu -= 1;
    if (menu < 1) menu = 8;
    timeSetMenu(menu);
    menuExitTimer.start();
  }

  if(enc.rightH()) {
    adjustTimeDate(1, menu);
    timeSetMenu(menu);
  }

  if(enc.leftH()) {
    adjustTimeDate(-1, menu);
    timeSetMenu(menu);
  }
  
  if(enc.click()){
    menuExitTimer.start();
    if(menu == 7){
      mode = 5;
      menu = 2;
      screenReadyFlag = false;
      screen = 1;
      enc.resetState();
    }

    if(menu == 8){
      rtc.adjust(DateTime(2000+new_year, new_month, new_day, new_hour, new_min, new_second));
      mode = 5;
      menu = 2;
      screen = 1;
      screenReadyFlag = false;
    }
  }
}


void adjustTimeDate(float delta, byte menu){
  switch(menu){
    case 1:
      new_hour += delta;
      if(new_hour > 23) new_hour = 0;
      if(new_hour < 0) new_hour = 23;
      break;

    case 2:
      new_min += delta;
      if(new_min > 59) new_min = 0;
      if(new_min < 0) new_min = 59;
      break;

    case 3:
      new_second += delta;
      if(new_second > 59) new_second = 0;
      if(new_second < 0) new_second = 59;
      break;

    case 4:
      new_day += delta;
      if(new_day > 31) new_day = 0;
      if(new_day < 0) new_day = 31;
      break;

    case 5:
      new_month += delta;
      if(new_month > 31) new_month = 0;
      if(new_month < 0) new_month = 31;
      break;

    case 6:
      new_year += delta;
      if(new_year > 99) new_year = 20;
      if(new_year < 20) new_year = 99;
      break;

    case 11:
      day_energy += delta;
      if(day_energy < 0) day_energy = 0;
      break;

    case 12:
      night_energy += delta;
      if(night_energy < 0) night_energy = 0;
      break;

    case 21:
      lcd_bright += delta;
      lcd_bright = constrain(lcd_bright, 1, 9);
      analogWrite(BACKLIGHT, lcd_bright*15);
      break;
      
  }
  menuExitTimer.start();
}


void setMeter(){
  byte s;
  if (DEMO_MODE) s = 100; 
  else s = 0;
  
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(2, 0);  lcd.print(F("Adjust meter"));
    lcd.setCursor(0, 1);  lcd.print(F("Day:")); lcd.setCursor(12, 1); lcd.print(F("kWh"));
    lcd.setCursor(0, 2);  lcd.print(F("Ngt:")); lcd.setCursor(12, 2); lcd.print(F("kWh"));
    lcd.setCursor(2, 3);  lcd.print(F("Back"));
    lcd.setCursor(12, 3);  lcd.print(F("Save"));
    EEPROM.get(4+s, day_energy);
    EEPROM.get(8+s, night_energy);
    screenReadyFlag = true;
    }

  if(printTimer.isReady()) {
    lcd.setCursor(4,1); printEnergy(day_energy, true);
    lcd.setCursor(4,2); printEnergy(night_energy, true);
    meterSetMenu(menu);
    blinkFlag = !blinkFlag;
  }
  
  if(enc.right()) {
    menu += 1;
    if (menu > 4) menu = 1;
    meterSetMenu(menu);
    menuExitTimer.start();
  }

  if(enc.left()) {
    menu -= 1;
    if (menu < 1) menu = 4;
    meterSetMenu(menu);
    menuExitTimer.start();
  }

  if(enc.rightH()) {
    if (enc.fast()) adjustTimeDate(5, menu + 10);
    else adjustTimeDate(0.1, menu + 10);
    meterSetMenu(menu);
  }

  if(enc.leftH()) {
    if (enc.fast()) adjustTimeDate(-5, menu + 10);
    else adjustTimeDate(-0.1, menu + 10);
    meterSetMenu(menu);
  }
  
  if(enc.click()){
    menuExitTimer.start();
    if(menu == 3){
      mode = 5;
      menu = 3;
      screen = 1;
      screenReadyFlag = false;
    }

    if(menu == 4){
      EEPROM.put(4+s, day_energy);
      EEPROM.put(8+s, night_energy);
      total_energy = day_energy + night_energy;
      EEPROM.put(12+s, total_energy);
      mode = 5;
      menu = 3;
      screen = 1;
      screenReadyFlag = false;
    }
    enc.resetState();
  }
}

void setBright(){
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(1, 0);  lcd.print(F("Adjust brightness"));
    lcd.setCursor(0, 1);  lcd.print(F("Brightness:")); 
    lcd.setCursor(2, 3);  lcd.print(F("Back"));
    lcd.setCursor(12, 3);  lcd.print(F("Save"));
    EEPROM.get(16, lcd_bright);
    screenReadyFlag = true;
    }

  if(printTimer.isReady()) {
    lcd.setCursor(12,1);  lcd.print(lcd_bright);
    blinkFlag = !blinkFlag;
    if(blinkFlag) brightSetMenu(menu);
  }
  
  if(enc.right()) {
    menu += 1;
    if (menu > 3) menu = 1;
    menuExitTimer.start();
    brightSetMenu(menu);
  }

  if(enc.left()) {
    menu -= 1;
    if (menu < 1) menu = 3;
    menuExitTimer.start();
    brightSetMenu(menu);
  }

  if(enc.rightH()) {
    adjustTimeDate(1, 21);
    brightSetMenu(menu);
  }

  if(enc.leftH()) {
    adjustTimeDate(-1, 21);
    brightSetMenu(menu);
  }
  
  if(enc.click()){
    menuExitTimer.start();
    if(menu == 2){
      mode = 5;
      menu = 4;
      screen = 1;
      screenReadyFlag = false;
      EEPROM.get(16, lcd_bright);
      analogWrite(BACKLIGHT, lcd_bright*15);
    }

    if(menu == 3){
      EEPROM.put(16, lcd_bright);
      mode = 5;
      menu = 4;
      screen = 1;
      screenReadyFlag = false;
      analogWrite(BACKLIGHT, lcd_bright*15);
    }
    enc.resetState();
  }
}

//
//void setDemoMode(){
//  if (!screenReadyFlag) {
//    lcd.clear();
//    }
//    
//  if (!screenReadyFlag){
//    lcd.setCursor(4, 0);  lcd.print(F("Select mode"));
//    lcd.setCursor(0, 1);  lcd.print(F("Demo mode:")); 
//    lcd.setCursor(2, 3);  lcd.print(F("Back"));
//    lcd.setCursor(12, 3);  lcd.print(F("Save"));
//    EEPROM.get(17, new_demo_mode);
//    screenReadyFlag = true;
//    }
//
//  if(printTimer.isReady()) {
//    lcd.setCursor(12,1);  
//    if (new_demo_mode) lcd.print(F("On "));
//    else lcd.print(F("Off"));
//    blinkFlag = !blinkFlag;
//    if(blinkFlag) modeSetMenu(menu);
//  }
//  
//  if(enc.right()) {
//    menu += 1;
//    if (menu > 3) menu = 1;
//    menuExitTimer.start();
//    modeSetMenu(menu);
//  }
//
//  if(enc.left()) {
//    menu -= 1;
//    if (menu < 1) menu = 3;
//    menuExitTimer.start();
//    modeSetMenu(menu);
//  }
//
//  if(menu == 1 && (enc.leftH() || enc.rightH())) {
//    new_demo_mode = !new_demo_mode;
//    menuExitTimer.start();
//    modeSetMenu(menu);
//  }
//  
//  if(enc.click()){
//    menuExitTimer.start();
//    if(menu == 2){
//      mode = 5;
//      menu = 5;
//      screen = 1;
//      screenReadyFlag = false;
//    }
//
//    if(menu == 3){
//      DEMO_MODE = new_demo_mode;
//      EEPROM.put(17, DEMO_MODE);
//      mode = 5;
//      menu = 5;
//      screen = 1;
//      screenReadyFlag = false;
//    }
//  enc.resetState();
//  }
//}
