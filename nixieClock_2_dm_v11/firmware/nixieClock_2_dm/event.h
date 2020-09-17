#pragma once

#define GLITCH_START_SECOND 7
#define GLITCH_END_SECOND 55

class Event {
protected:
	boolean effectExecution = false, suspended = false; 
	TimerMinim timer = TimerMinim(); 
	byte currentMode;
public:
	Event() { }
	virtual void loop() {}
	virtual void setMode(byte mode) {}
	virtual void switchMode() {}
	virtual void suspend() { suspended = true; }
	virtual void resume() { suspended = false; }
	byte mode() { return currentMode; }
};


class BacklightEvent : public Event {
private:
	boolean _directionToBright = true;
	float _backlBrightCounter = 0.0;
public:
	BacklightEvent(uint32_t timerInterval);
	void setMode(byte mode);
	void switchMode();
	void loop();
};


class DotBlinkingEvent : public Event {
private:
	byte _dotBrightStep, _dotBrightCounter = 0;
	uint32_t _resetCounter, _timerInterval;
	boolean _directionToBright = true;
public:
	DotBlinkingEvent(uint32_t timerInterval);
	void loop();
};


class GlitchEvent : public Event {
private:
	boolean _indiState; 
	byte _glitchCounter, _glitchMax, _glitchIndic, _timerInterval;
	uint32_t _glitchMinSeconds = GLITCH_MIN, _glitchMaxSeconds = GLITCH_MAX;

public:
	GlitchEvent(uint32_t timerInterval);
	void setMode(byte mode);
	void switchMode();
	void loop();
};


class FlipEvent : public Event {
private:
	byte _currentLamp, _flipEffectStages = 0, _indiBrightCounter;
	byte _startCathode[4], _endCathode[4];
	boolean _trainLeaving;
	boolean _directionToBright = true;
public:
	FlipEvent();
	void setMode(byte mode);
	void switchMode();
	void loop();
};

class NightSwitcherEvent : public Event {
private:
public:
	NightSwitcherEvent(uint32_t timerInterval);
	void loop();
};

class LampBurningEvent : public Event {
private:
public:
	LampBurningEvent(uint32_t timerInterval);
	void loop();
};