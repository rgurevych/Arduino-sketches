AlarmMode::AlarmMode(uint32_t timeInterval) : Mode() {
    timer.setInterval(timeInterval);
    _timeInterval = timeInterval;
}

void AlarmMode::init(byte param) {
    _lampState = true;
    _alarmTimeCounter = 0;
#ifdef ALM_DFPLAYER
    _soundVolume = 0;
#endif
}

void AlarmMode::loop() {
    Mode::loop();
    if (!timer.isReady()) return;
    syncRTC();
    sendTime(hrs, mins);
    _alarmTimeCounter += _timeInterval;
    if (_alarmTimeCounter > ALM_TIMEOUT*60000) stopAlarm();
    _lampState = !_lampState;
    anodeStates[0] = _lampState;
    anodeStates[1] = _lampState;
    anodeStates[2] = _lampState;
    anodeStates[3] = _lampState;
    if (_lampState) return;
#ifndef ALM_DFPLAYER
    setPin(PIEZO, HIGH);
    delay(100);
    setPin(PIEZO, LOW);
    delay(100);
    setPin(PIEZO, HIGH);
    delay(100);
    setPin(PIEZO, LOW);
#else
    if (_soundVolume < alarmVolume) {
        _soundVolume++;
        myDFPlayer.volume(_soundVolume);
    }
    if (_soundVolume == 1) {
        myDFPlayer.loop(alarmTrack);
    }
#endif
}

void AlarmMode::buttonsLoop() {
    if (btnSet.isClick() || btnLeft.isClick() || btnRight.isClick()) stopAlarm();
    if (btnSet.isHolded() || btnLeft.isHolded() || btnRight.isHolded()) {} // reset events
}

void AlarmMode::stopAlarm() {
    if (alarmMode == 1) {
        alarmMode = 0;
        EEPROM.put(EEPROM_CELL_ALARM_MODE, alarmMode);
    }
#ifdef ALM_DFPLAYER
    myDFPlayer.stop();
#endif
    modeSelector.setMode(Modes::Main, 0);
}
