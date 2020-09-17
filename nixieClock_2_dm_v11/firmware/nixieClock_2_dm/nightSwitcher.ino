NightSwitcherEvent::NightSwitcherEvent(uint32_t timerInterval) : Event() {
    timer.setInterval(timerInterval);
}

void NightSwitcherEvent::loop() {
    if (!timer.isReady() || suspended) return;
    if (
        (nightHrStart > nightHrEnd && (hrs >= nightHrStart || hrs < nightHrEnd)) ||
        (nightHrStart < nightHrEnd && hrs >= nightHrStart && hrs < nightHrEnd)
        ) {
        indiMaxBright = INDI_BRIGHT_N;
        backlMaxBright = BACKL_BRIGHT_N;
        dotMaxBright = DOT_BRIGHT_N;
        backlStep = BACKL_STEP_N;
    }
    else {
        indiMaxBright = INDI_BRIGHT;
        backlMaxBright = BACKL_BRIGHT;
        dotMaxBright = DOT_BRIGHT;
        backlStep = BACKL_STEP;
    }
    for (byte i = 0; i < 4; i++) {
        indiDimm[i] = indiMaxBright;
    }
}
