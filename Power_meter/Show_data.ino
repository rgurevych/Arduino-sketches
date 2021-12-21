void printPowerData() {

  if(enc.click()){
    mode = 1;
    screenReadyFlag = false;
    screen = 0;
    menu = 1;
    enc.resetState();
    menuExitTimer.start();
  }
  
  if (printTimer.isReady()){

    getTime();
    
    if (DEBUG_MODE) {
      Serial.print(hour); Serial.print(F(":")); Serial.print(min); Serial.print(F(":")); Serial.print(second);
      Serial.print(F("  ")); Serial.print(day); Serial.print(F("/")); Serial.print(month); Serial.print(F("/")); Serial.println(year);
      Serial.print(F("U: "));      Serial.print(mom_voltage / 10.0);      Serial.println(F("V"));
      Serial.print(F("I: "));      Serial.print(mom_current / 100.0);     Serial.println(F("A"));
      Serial.print(F("P: "));      Serial.print(mom_power);               Serial.println(F("W"));
      Serial.print(F("E: "));      Serial.print(mom_energy / 10.0);       Serial.println(F("kWh"));
      Serial.print(F("F: "));      Serial.print(mom_frequency / 10.0);    Serial.println(F("Hz"));
      Serial.print(F("PF: "));     Serial.println(mom_pf / 100.0);        Serial.println();
    }

    if (!screenReadyFlag) {
      lcd.clear();
    }
    
    if (screen == 0) {
      if (!screenReadyFlag){
        lcd.setCursor(5, 0);
        lcd.print(F("The meter"));
        lcd.setCursor(3, 1);
        lcd.print(F("is not powered"));
        screenReadyFlag = true;
      }
    }
  
    if (screen == 1) {
      if (!screenReadyFlag){
        if (DEMO_MODE) {
          lcd.setCursor(18, 0); lcd.print(F("DM"));
        }
        else {
          lcd.setCursor(18, 0); lcd.print(F("NW"));
        }
        lcd.setCursor(2, 0); lcd.print(F(":")); lcd.setCursor(5,0); lcd.print(F(":"));
        lcd.setCursor(11, 0); lcd.print(F("/")); lcd.setCursor(14,0); lcd.print(F("/"));
        lcd.setCursor(0, 1); lcd.print(F("U=")); lcd.setCursor(7,1); lcd.print(F("V"));
        lcd.setCursor(12, 1); lcd.print(F("I=")); lcd.setCursor(19,1); lcd.print(F("A"));
        lcd.setCursor(0, 2); lcd.print(F("P=")); lcd.setCursor(7,2); lcd.print(F("W"));
        lcd.setCursor(12, 2); lcd.print(F("F=")); lcd.setCursor(18,2); lcd.print(F("Hz"));
        lcd.setCursor(0, 3); lcd.print(F("E=")); lcd.setCursor(8,3); lcd.print(F("kWh"));
        lcd.setCursor(13, 3); lcd.print(F("PF=")); 
        screenReadyFlag = true;
      }
  
      lcd.setCursor(0,0); if (hour < 10){lcd.print(F("0"));} lcd.print(hour); 
      lcd.setCursor(3,0); if (min < 10){lcd.print(F("0"));} lcd.print(min); 
      lcd.setCursor(6,0); if (second < 10){lcd.print(F("0"));} lcd.print(second);
      lcd.setCursor(9,0); if (day < 10){lcd.print(F("0"));} lcd.print(day); 
      lcd.setCursor(12,0); if (month < 10){lcd.print(F("0"));} lcd.print(month); 
      lcd.setCursor(15,0); lcd.print(year-2000);
      lcd.setCursor(2,1); lcd.print(mom_voltage / 10.0, 1);
      lcd.setCursor(14,1); if(mom_current < 1000){lcd.print(F(" "));} lcd.print(mom_current / 100.0, 2);
      lcd.setCursor(2,2); if(mom_power < 10000){lcd.print(F(" "));} if(mom_power < 1000){lcd.print(F(" "));} 
      if(mom_power < 100){lcd.print(F(" "));} if(mom_power < 10){lcd.print(F(" "));} lcd.print(mom_power);
      lcd.setCursor(14,2); lcd.print(mom_frequency / 10.0, 1);
      lcd.setCursor(2,3); printEnergy(mom_energy, false);
      lcd.setCursor(16,3); lcd.print(mom_pf / 100.0);
    }
  }
}

void printEnergy(float energy, bool meter_energy){
  if(meter_energy && energy < 100000) lcd.print(F(" "));
  if(energy < 10000) lcd.print(F(" ")); 
  if(energy < 1000) lcd.print(F(" ")); 
  if(energy < 100) lcd.print(F(" "));
  if(meter_energy) lcd.print(energy, 1);
  else lcd.print(energy / 10.0, 1);
}


void showMeter(){
  byte s;
  if (DEMO_MODE) s = 100; 
  else s = 0;
  
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    EEPROM.get(4+s, day_energy);
    EEPROM.get(8+s, night_energy);
    EEPROM.get(12+s, total_energy);
    
    lcd.setCursor(4, 0);  lcd.print(F("Energy meter"));
    lcd.setCursor(0, 1);  lcd.print(F("Day:"));  printEnergy(day_energy, true);  lcd.print(F("kWh"));
    lcd.setCursor(0, 2);  lcd.print(F("Ngt:"));  printEnergy(night_energy, true);  lcd.print(F("kWh"));
    lcd.setCursor(0, 3);  lcd.print(F("Tot:"));  printEnergy(total_energy, true);  lcd.print(F("kWh"));
    screenReadyFlag = true;
    }
  
  if(enc.click()){
    mode = 1;
    screenReadyFlag = false;

    enc.resetState();
  }
}
