void ModeSelector::setMode(Modes mode, byte param) {
	if (mode == Modes::Main) {
		_currentMode = _mainMode;
	}
	if (mode == Modes::Setup) {
		_currentMode = _setupMode;
	}
	if (mode == Modes::Alarm) {
		_currentMode = _alarmMode;
	}
	_currentMode->init(param);
}

void Mode::loop() {
	btnSet.tick();
	btnLeft.tick();
	btnRight.tick();
	buttonsLoop();
};
