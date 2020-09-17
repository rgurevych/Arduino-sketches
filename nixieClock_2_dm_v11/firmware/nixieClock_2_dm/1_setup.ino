void setup() {
  //Serial.begin(9600);

  // случайное зерно для генератора случайных чисел
  randomSeed(analogRead(6) + analogRead(7));

  // настройка пинов на выход
  pinMode(DECODER0, OUTPUT);
  pinMode(DECODER1, OUTPUT);
  pinMode(DECODER2, OUTPUT);
  pinMode(DECODER3, OUTPUT);
  pinMode(KEY0, OUTPUT);
  pinMode(KEY1, OUTPUT);
  pinMode(KEY2, OUTPUT);
  pinMode(KEY3, OUTPUT);
  pinMode(GEN, OUTPUT);
  pinMode(DOT, OUTPUT);
  pinMode(BACKL, OUTPUT);

#ifdef ALM_DFPLAYER
  mySoftwareSerial.begin(9600);
  myDFPlayer.begin(mySoftwareSerial);
  myDFPlayer.volume(0);
#else
  pinMode(PIEZO, OUTPUT);
#endif

  // задаем частоту ШИМ на 9 и 10 выводах 31 кГц
  TCCR1B = TCCR1B & 0b11111000 | 1;    // ставим делитель 1

  // включаем ШИМ
  setPWM(9, DUTY);

  // перенастраиваем частоту ШИМ на пинах 3 и 11 на 7.8 кГц и разрешаем прерывания COMPA
  TCCR2B = (TCCR2B & B11111000) | 2;    // делитель 8
  TCCR2A |= (1 << WGM21);   // включить CTC режим для COMPA
  TIMSK2 |= (1 << OCIE2A);  // включить прерывания по совпадению COMPA

// ---------- RTC -----------
rtc.begin();
if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

  // EEPROM
if (EEPROM.read(1023) != 100) {   // первый запуск
    EEPROM.put(1023, 100);
    EEPROM.put(EEPROM_CELL_FLIP_MODE, 1);
    EEPROM.put(EEPROM_CELL_BACKLIGHT_MODE, 0);
    EEPROM.put(EEPROM_CELL_GLITCH_MODE, 0);
    EEPROM.put(EEPROM_CELL_NIGHT_START, 0);
    EEPROM.put(EEPROM_CELL_NIGHT_END, 0);
    EEPROM.put(EEPROM_CELL_ALARM_HOURS, 0);
    EEPROM.put(EEPROM_CELL_ALARM_MINUTES, 0);
    EEPROM.put(EEPROM_CELL_ALARM_MODE, 0);
#ifdef ALM_DFPLAYER
    EEPROM.put(EEPROM_CELL_ALARM_TRACK, 1);
    EEPROM.put(EEPROM_CELL_ALARM_VOLUME, alarmVolume);
#endif
  }

syncRTC();
sendTime(hrs, mins);  // отправить время на индикаторы
// установить яркость на индикаторы
for (byte i = 0; i < 4; i++) indiDimm[i] = indiMaxBright;

EEPROM.get(EEPROM_CELL_NIGHT_START, nightHrStart);
EEPROM.get(EEPROM_CELL_NIGHT_END, nightHrEnd);
EEPROM.get(EEPROM_CELL_ALARM_HOURS, alarmHrs);
EEPROM.get(EEPROM_CELL_ALARM_MINUTES, alarmMins);
EEPROM.get(EEPROM_CELL_ALARM_MODE, alarmMode);
#ifdef ALM_DFPLAYER
EEPROM.get(EEPROM_CELL_ALARM_TRACK, alarmTrack);
EEPROM.get(EEPROM_CELL_ALARM_VOLUME, alarmVolume);
#endif
}
