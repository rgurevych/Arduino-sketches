void getPowerData() {
  if (measureTimer.isReady()) {
    if (DEMO_MODE) {
      generatePowerData();
    }
    else {
      measurePower();
    }
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


void generatePowerData() {
  mom_voltage = 2100 + random(200);
  mom_current = random(1000);
  mom_power = round((mom_voltage / 10.0) * (mom_current / 100.0));
  mom_energy += mom_power / 1800;
  mom_frequency = random(498, 502);
  mom_pf = random(90, 101);
}


void recordMeter() {
  if (minute == 0 && !recordMeterDoneFlag) {
    if (meterPowered || DEMO_MODE) {
      updateMeter();
    }
    recordMeterDoneFlag = true;
  }
  else if (minute != 0) {
    recordMeterDoneFlag = false;
  }
}


void updateMeter() {
  byte s;
  float recordEnergy;
  if (DEMO_MODE) {
    s = 100;
    recordEnergy = mom_energy / 10.0;
  }
  else {
    s = 0;
    recordEnergy = pzem.energy();
    if (recordEnergy > 9900) {
      pzem.resetEnergy();
    }
  }

  EEPROM.get(0 + s, latest_energy);
  getMeterData(s);

  float energyDelta = recordEnergy - latest_energy;           //Считаем разницу между текущими и сохраненными ранее показаниями
  if (energyDelta < 0) {                                      //Если разница меньше 0, значит счетчик сбрасывали
    energyDelta = recordEnergy;                               //поэтому в таком случае учитываем полное значение счетчика (разница между текущими показаниями и 0)
  }
  EEPROM.put(0 + s, recordEnergy);

  if (hour > DAY_TARIFF_START && hour <= NIGHT_TARIFF_START) {
    day_energy += energyDelta;
    EEPROM.put(4 + s, day_energy);
  }
  else {
    night_energy += energyDelta;
    EEPROM.put(8 + s, night_energy);
  }
  total_energy = day_energy + night_energy;
  EEPROM.put(12 + s, total_energy);
  EEPROM.commit();
}


void getMeterData(byte s) {
  EEPROM.get(4 + s, day_energy);
  EEPROM.get(8 + s, night_energy);
  EEPROM.get(12 + s, total_energy);
}


void showMeter() {
  byte s;
  if (DEMO_MODE) s = 100;
  else s = 0;

  if (!screenReadyFlag) {
    lcd.clear();
  }

  if (!screenReadyFlag) {
    getMeterData(s);

    lcd.setCursor(4, 0);  lcd.print(F("Energy meter"));
    lcd.setCursor(0, 1);  lcd.print(F("Day:"));  printEnergy(day_energy, true);  lcd.print(F("kWh"));
    lcd.setCursor(0, 2);  lcd.print(F("Ngt:"));  printEnergy(night_energy, true);  lcd.print(F("kWh"));
    lcd.setCursor(0, 3);  lcd.print(F("Tot:"));  printEnergy(total_energy, true);  lcd.print(F("kWh"));
    screenReadyFlag = true;
  }

  if (enc.click()) {
    mode = 1;
    screenReadyFlag = false;

    enc.resetState();
  }
}
