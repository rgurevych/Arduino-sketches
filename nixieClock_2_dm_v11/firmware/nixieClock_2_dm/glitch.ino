GlitchEvent::GlitchEvent(uint32_t timerInterval) : Event() {
    EEPROM.get(EEPROM_CELL_GLITCH_MODE, currentMode);
    _timerInterval = timerInterval;
    timer.setInterval(random(_glitchMinSeconds * _timerInterval, _glitchMaxSeconds * _timerInterval));
}

void GlitchEvent::setMode(byte mode) {
    currentMode = mode;
    EEPROM.put(EEPROM_CELL_GLITCH_MODE, currentMode);
}

void GlitchEvent::switchMode() {
    setMode(!currentMode);
}

void GlitchEvent::loop() {
    if (!timer.isReady() || suspended) return;
    if (currentMode == 0) return;
    if (!effectExecution && secs > GLITCH_START_SECOND && secs < GLITCH_END_SECOND) {
        effectExecution = true;
        _indiState = 0;
        _glitchCounter = 0;
        _glitchMax = random(2, 6);
        _glitchIndic = random(0, 4);
        timer.setInterval(random(2, 6) * 30);
    }
    else if (effectExecution) {
        indiDimm[_glitchIndic] = _indiState * indiMaxBright;
        _indiState = !_indiState;
        timer.setInterval(random(2, 6) * 30);
        _glitchCounter++;
        if (_glitchCounter > _glitchMax) {
            timer.setInterval(random(_glitchMinSeconds * _timerInterval, _glitchMaxSeconds * _timerInterval));
            effectExecution = false;
            indiDimm[_glitchIndic] = indiMaxBright;
        }
    }
}
