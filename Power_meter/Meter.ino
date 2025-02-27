void getPowerData() {
  if (measureTimer.isReady()) {
    if (DEMO_MODE) {
      generatePowerData();
    }
    else {
      measurePower();
    }

    updateAverageData();
  }
}


void measurePower() {
  if (pzem.readAddress() == 0) {
    meterPowered = false;
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
    meterPowered = true;
    readPowerData();
    if (mode == 0 && screen == 0) {
      screen = 1;
      screenReadyFlag = false;
    }
  }
}


void readPowerData() {
  float cur_voltage = pzem.voltage();
  float cur_current = pzem.current();
  float cur_power = pzem.power();
  float cur_energy = pzem.energy();
  float cur_frequency = pzem.frequency();
  float cur_pf = pzem.pf();
  
  if (!isnan(cur_voltage)) {
    mom_voltage = cur_voltage * 10;
  }
  mom_voltage = constrain(mom_voltage, 0, 3800);
  
  if (!isnan(cur_current)) {
    mom_current = round(cur_current * 100);
  }

  if (!isnan(cur_power)) {
    mom_power = round(cur_power);
  }
  
  if (!isnan(cur_energy)) {
    mom_energy = round(cur_energy * 10);
  }
  
  if (!isnan(cur_frequency)) {
    mom_frequency = cur_frequency * 10;
  }

  if (!isnan(cur_pf)) {
    mom_pf = cur_pf * 100;
  }
}


void generatePowerData() {
  mom_voltage = 2100 + random(200);
  mom_current = random(1000);
  mom_power = round((mom_voltage / 10.0) * (mom_current / 100.0));
  mom_energy += mom_power / 1800;
  mom_frequency = random(498, 502);
  mom_pf = random(90, 101);
}

void updateAverageData(){
  if (resetAverageDataFlag) {
    av_voltage = mom_voltage;
    av_current = mom_current;
    av_power = mom_power;
    av_counter = 1;
    resetAverageDataFlag = false;
  }
  else {
    if (mom_voltage > 0) {
      av_voltage = (av_voltage * av_counter + mom_voltage) / (av_counter + 1);
    }
  
    if (mom_current > 0) {
      av_current = (av_current * av_counter + mom_current) / (av_counter + 1);
    }
  
    if (mom_power > 0) {
      av_power = (av_power * av_counter + mom_power) / (av_counter + 1);
    }
  av_counter ++;
  }
}


void recordMeter() {
  if (minute == 0 && !recordMeterDoneFlag) {
    if (meterPowered || DEMO_MODE) {
      updateMeter();
      recordMeterDoneFlag = true;
    }
  }
  else if (minute != 0) {
    recordMeterDoneFlag = false;
  }
}


void updateMeter() {
  byte s;
  float recordEnergy = mom_energy / 10.0;
  if (DEMO_MODE) {
    s = 100;
  }
  else {
    s = 0;
    if (recordEnergy > 99000) {
      pzem.resetEnergy();
    }
  }

  EEPROMr.get(0 + s, latest_energy);
  getMeterData(s);

  energyDelta = recordEnergy - latest_energy;                 //Считаем разницу между текущими и сохраненными ранее показаниями
  if (energyDelta < 0) {                                      //Если разница меньше 0, значит счетчик сбрасывали
    energyDelta = recordEnergy;                               //поэтому в таком случае учитываем полное значение счетчика (разница между текущими показаниями и 0)
  }
  
  if (!isnan(recordEnergy)) {
    EEPROMr.put(0 + s, recordEnergy);

    if (hour > DAY_TARIFF_START && hour <= NIGHT_TARIFF_START) {
      day_energy += energyDelta;
      EEPROMr.put(4 + s, day_energy);
    }
    else {
      night_energy += energyDelta;
      EEPROMr.put(8 + s, night_energy);
    }
    total_energy = day_energy + night_energy;
    EEPROMr.put(12 + s, total_energy);
  }

  if (hour == 0) {
    updateDailyMeter(s, day_energy, night_energy);
    if (day == 1) {
      updateMonthlyMeter(s, day_energy, night_energy);
    }
  }

  updatePlotArray(energyDelta);

  publishHourlyEnergyFlag = true;

  EEPROMr.put(300 + s, plot_array);
  EEPROMr.commit();
}


void updateDailyMeter(byte s, float currentDayEnergy, float currentNightEnergy) {
  float lastDayEnergy, lastNightEnergy;
  EEPROMr.get(20+s, lastDayEnergy);
  EEPROMr.get(24+s, lastNightEnergy);

  dailyDayEnergyDelta = currentDayEnergy - lastDayEnergy;
  dailyNightEnergyDelta = currentNightEnergy - lastNightEnergy;

  EEPROMr.put(20+s, currentDayEnergy);
  EEPROMr.put(24+s, currentNightEnergy);

  publishDailyEnergyFlag = true;
  publishDailyTelegramReport = true;
}


void updateMonthlyMeter(byte s, float currentDayEnergy, float currentNightEnergy) {
  float lastDayEnergy, lastNightEnergy;
  EEPROMr.get(30+s, lastDayEnergy);
  EEPROMr.get(34+s, lastNightEnergy);

  monthlyDayEnergyDelta = currentDayEnergy - lastDayEnergy;
  monthlyNightEnergyDelta = currentNightEnergy - lastNightEnergy;
   
  EEPROMr.put(30+s, currentDayEnergy);
  EEPROMr.put(34+s, currentNightEnergy);

  publishMonthlyTelegramReport = true;
}


void getMeterData(byte s) {
  EEPROMr.get(4 + s, day_energy);
  EEPROMr.get(8 + s, night_energy);
  EEPROMr.get(12 + s, total_energy);
}


void showMeter() {
  byte s;
  if (DEMO_MODE) s = 100;
  else s = 0;

  if (!screenReadyFlag) {
    lcd.clear();
  }

  if (!screenReadyFlag) {
    if (screen == 0){
      getMeterData(s);
      lcd.setCursor(1, 0);  lcd.print(F("Current meter    ->"));
      lcd.setCursor(0, 1);  lcd.print(F("Day:"));  printEnergy(day_energy, true);  lcd.print(F(" kWh"));
      lcd.setCursor(0, 2);  lcd.print(F("Ngt:"));  printEnergy(night_energy, true);  lcd.print(F(" kWh"));
      lcd.setCursor(0, 3);  lcd.print(F("Tot:"));  printEnergy(total_energy, true);  lcd.print(F(" kWh"));
      screenReadyFlag = true;
    }
    else if (screen == 1){
      float lastMonthlyDayEnergy, lastMonthlyNightEnergy;
      EEPROMr.get(30+s, lastMonthlyDayEnergy);
      EEPROMr.get(34+s, lastMonthlyNightEnergy);
      lcd.setCursor(0, 0);  lcd.print(F("<- Saved for 01.")); if (month < 10){lcd.print(F("0"));} lcd.print(month);
      lcd.setCursor(0, 1);  lcd.print(F("Day:"));  printEnergy(lastMonthlyDayEnergy, true);  lcd.print(F(" kWh"));
      lcd.setCursor(0, 2);  lcd.print(F("Ngt:"));  printEnergy(lastMonthlyNightEnergy, true);  lcd.print(F(" kWh"));
      lcd.setCursor(0, 3);  lcd.print(F("Tot:"));  printEnergy(lastMonthlyDayEnergy + lastMonthlyNightEnergy, true);  lcd.print(F(" kWh"));
      screenReadyFlag = true;
    }
  }

  if (enc.turn()) {
    if (screen == 0) screen = 1;
    else if (screen == 1) screen = 0;
    screenReadyFlag = false;
    menuExitTimer.start();
    enc.resetState();
  }
  
  if (enc.click()) {
    mode = 1;
    screenReadyFlag = false;
    screen = 1;
    enc.resetState();
    menuExitTimer.start();
  }
}
