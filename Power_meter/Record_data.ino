void recordMeter(){
  if(min == 0 && !recordMeterDoneFlag){
    if (pzem.readAddress() != 0 || DEMO_MODE) {
      updateMeter();
    }
    recordMeterDoneFlag = true;
  }
  else if(min != 0) {
    recordMeterDoneFlag = false;
  }
}


void updateMeter(){
  byte s;
  float recordEnergy;
  if (DEMO_MODE) {
    s = 100;
    recordEnergy = mom_energy / 10.0;
  }
  else {
    s = 0;
    recordEnergy = pzem.energy();
  }
  
  EEPROM.get(0+s, latest_energy);
  EEPROM.get(4+s, day_energy);
  EEPROM.get(8+s, night_energy);
  EEPROM.get(12+s, total_energy);
  float energyDelta = recordEnergy - latest_energy;           //Считаем разницу между текущими и сохраненными ранее показаниями
  if (energyDelta < 0){                                       //Если разница меньше 0, значит счетчик сбрасывали
    energyDelta = recordEnergy;                               //поэтому в таком случае учитываем полное значение счетчика (разница между текущими показаниями и 0)
  }
  EEPROM.put(0+s, recordEnergy);

  if(hour > DAY_TARIFF_START && hour <= NIGHT_TARIFF_START) {
    day_energy += energyDelta;
    EEPROM.put(4+s, day_energy);
  }
  else {
    night_energy += energyDelta;
    EEPROM.put(8+s, night_energy);
  }
  total_energy = day_energy + night_energy;
  EEPROM.put(12+s, total_energy);
}
