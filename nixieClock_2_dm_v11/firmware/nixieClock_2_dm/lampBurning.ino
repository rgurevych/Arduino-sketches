LampBurningEvent::LampBurningEvent(uint32_t timerInterval) : Event() {
    timer.setInterval(timerInterval);
}

void LampBurningEvent::loop() {
    if (!timer.isReady() || suspended) return;
    for (byte k = 0; k < BURN_LOOPS; k++) {
        for (byte d = 0; d < 10; d++) {
            for (byte i = 0; i < 4; i++) {
                indiDigits[i]--;
                if (indiDigits[i] < 0) indiDigits[i] = 9;
            }
            delay(BURN_TIME);
        }
    }
}
