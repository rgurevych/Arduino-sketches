BacklightEvent::BacklightEvent(uint32_t timerInterval) : Event() {
    EEPROM.get(EEPROM_CELL_BACKLIGHT_MODE, currentMode);
    timer.setInterval(timerInterval);
}

void BacklightEvent::setMode(byte mode) {
    currentMode = mode;
    if (currentMode == 0) setPin(BACKL, 0);
    EEPROM.put(EEPROM_CELL_BACKLIGHT_MODE, currentMode);
}

void BacklightEvent::switchMode() {
    byte mode = currentMode + 1;
    if (mode > 2) mode = 0;
    setMode(mode);
}

void BacklightEvent::loop()
{
    if (!timer.isReady() || suspended) return;
    if (currentMode == 1) setPWM(BACKL, backlMaxBright);
    if (currentMode == 2) {
        if (backlMaxBright > 0) {
            if (_directionToBright) {
                if (!effectExecution) {
                    effectExecution = true;
                    timer.setInterval(backlMaxBright / 2);
                }
                _backlBrightCounter += backlStep;
                if (_backlBrightCounter >= backlMaxBright) {
                    _directionToBright = false;
                    _backlBrightCounter = backlMaxBright;
                }
            }
            else {
                _backlBrightCounter -= backlStep;
                if (_backlBrightCounter <= BACKL_MIN_BRIGHT) {
                    _directionToBright = true;
                    _backlBrightCounter = BACKL_MIN_BRIGHT;
                    timer.setInterval(BACKL_PAUSE);
                    effectExecution = false;
                }
            }
            setPWM(BACKL, getPWM_CRT(_backlBrightCounter));
        }
        else {
            setPin(BACKL, 0);
        }
    }
}
