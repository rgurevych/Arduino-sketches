#pragma once
#include <GyverButton.h>
#include "timerMinim.h"
#include "event.h"
#define EVENTS_COUNT 6

enum Modes : byte {
	Main,
	Setup,
	Alarm
};


class Mode {
protected:
	virtual void buttonsLoop() {}
	TimerMinim timer = TimerMinim();
public:
	virtual void loop();
	virtual void init(byte param) = 0;
};

class MainMode : public Mode {
private:
	// Add new effect derived from 'Event' class here
	GlitchEvent _glitch = GlitchEvent(1000L);
	FlipEvent _flip = FlipEvent();
	BacklightEvent _backlight = BacklightEvent(10L);
	DotBlinkingEvent _blink = DotBlinkingEvent(DOT_TIMER);
	NightSwitcherEvent _night = NightSwitcherEvent(60000L); // every minute
	LampBurningEvent _burn = LampBurningEvent(BURN_PERIOD*60000L);
	// Add pointer and increase EVENTS_COUNT to enable effect
	Event* _events[EVENTS_COUNT] = { &_glitch, &_flip, &_backlight, &_blink, &_night, &_burn };
	boolean _needRefreshTime = false, _showMode = false;
	byte _showModeSecCounter, _modeValue;
protected:
	void buttonsLoop();
public:
	MainMode(uint32_t timeInterval);
	void loop();
	void init(byte param);
};

class SetupMode : public Mode {
private:
	int8_t _changeHrs, _changeMins;
	byte _changeDay, _changeMonth;
	uint16_t _changeYear;
	byte _param; // 0 = time setup, 1 = alarm setup, 2 = night mode setup
	boolean _isHoursSelected, _lampState;
	byte _setupStage = 0;
	GButton* _set;
	GButton* _left;
	GButton* _right;
protected:
	void buttonsLoop();
public:
	SetupMode(uint32_t timeInterval);
	void loop();
	void init(byte param);
};

class AlarmMode : public Mode {
private:
	boolean _lampState;
	uint32_t _alarmTimeCounter, _timeInterval;
	void stopAlarm();
#ifdef ALM_DFPLAYER
	byte _soundVolume;
#endif
protected:
	void buttonsLoop();
public:
	AlarmMode(uint32_t timeInterval);
	void loop();
	void init(byte param);
};
