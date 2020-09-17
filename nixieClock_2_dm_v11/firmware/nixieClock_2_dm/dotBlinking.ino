DotBlinkingEvent::DotBlinkingEvent(uint32_t timerInterval) : Event() {
    timer.setInterval(timerInterval);
    _timerInterval = timerInterval;
}

void DotBlinkingEvent::loop() {
    if (!timer.isReady() || suspended) return;
    if (_resetCounter >= 1000) {
        _resetCounter = 0;
        effectExecution = true;
        _dotBrightStep = ceil((float)dotMaxBright * (float)_timerInterval * 2 / (float)DOT_TIME);
        if (_dotBrightStep == 0) _dotBrightStep = 1;
    }
    if (effectExecution) {
        if (_directionToBright) {
            _dotBrightCounter += _dotBrightStep;
            if (_dotBrightCounter >= dotMaxBright) {
                _directionToBright = false;
                _dotBrightCounter = dotMaxBright;
            }
        }
        else {
            _dotBrightCounter -= _dotBrightStep;
            if (_dotBrightCounter <= 0) {
                _directionToBright = true;
                _dotBrightCounter = 0;
                effectExecution = false;
            }
        }
        setPWM(DOT, getPWM_CRT(_dotBrightCounter));
    }
    _resetCounter += _timerInterval;
}
