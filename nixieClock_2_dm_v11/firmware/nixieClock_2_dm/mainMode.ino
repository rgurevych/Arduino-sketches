MainMode::MainMode(uint32_t timeInterval) : Mode() {
    timer.setInterval(timeInterval);
}

void MainMode::init(byte param) {
    _needRefreshTime = true;
    _showMode = false;
}

void MainMode::loop() {
    Mode::loop();
    if (_needRefreshTime) {
        anodeStates[0] = true;
        anodeStates[1] = true;
        anodeStates[2] = true;
        anodeStates[3] = true;
        _needRefreshTime = false;
        syncRTC();
        sendTime(hrs, mins);
    }
    for (byte i = 0; i < EVENTS_COUNT; i++) _events[i]->loop();
    if (timer.isReady()) {
        syncRTC();
        byte dow = rtc.now().dayOfTheWeek();
        if (hrs == alarmHrs && mins == alarmMins && secs == 0 && 
            (
                (alarmMode == 1 || alarmMode == 2) || 
                (alarmMode == 3 && dow != 1 && dow != 7)
            )) 
                modeSelector.setMode(Modes::Alarm, 0);
        if (_showMode) {
            if (_showModeSecCounter > SHOW_EFFECTS_MODE_TIMEOUT*2) {
                _showMode = false;
                _needRefreshTime = true;
                _glitch.resume();
                _flip.resume();
            }
            else {
                anodeStates[0] = false;
                anodeStates[1] = false;
                anodeStates[2] = false;
                anodeStates[3] = true;
                _showModeSecCounter++;
                sendTime(0, _modeValue);
            }
        }
        else if (secs >= 59) {
            setNewTime();
            newTimeFlag = true;
        }
    }
}

void MainMode::buttonsLoop() {
    Event* evnt = NULL;

    if (btnRight.isClick())  evnt = &_flip;

    if (btnLeft.isClick()) evnt = &_backlight;

    if (btnSet.isClick()) evnt = &_glitch;

    if (evnt != NULL) {
        evnt->switchMode();
        _modeValue = evnt->mode();
        _glitch.suspend();
        _flip.suspend();
        _showModeSecCounter = 0;
        _showMode = true;
    }

    if (btnLeft.isHolded()) modeSelector.setMode(Modes::Setup, 1);

    if (btnSet.isHolded()) modeSelector.setMode(Modes::Setup, 0);

    if (btnRight.isHolded()) modeSelector.setMode(Modes::Setup, 2);
}
