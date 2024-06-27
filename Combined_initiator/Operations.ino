void ledTick(){
  if(blinkIntervalTimer.tick()){
    ledBlinkFlag = true;
    ledFlag = true;
    blinkTimer.start();
  }

  if(ledBlinkFlag){
    if(blinkTimer.tick()){
      ledBlinkFlag = false;
      ledFlag = false;
      blinkIntervalTimer.start();
    }
  }

  ledCheck();
}

void ledCheck(){
  digitalWrite(LED_BUILTIN, ledFlag);
  digitalWrite(SAFETY_LED_PIN, ledFlag);
}

void safetyGuardEnable(){
  digitalWrite(RELAY_1_PIN, HIGH);
}


void safetyGuardDisable(){
  digitalWrite(RELAY_1_PIN, LOW);
  delay(200);
}


void detonateEnable(){
  digitalWrite(RELAY_2_PIN, HIGH);
}


void detonateDisable(){
  digitalWrite(RELAY_2_PIN, LOW);
}

#if REMOTE_CONTROL
void getPWM() {
  if (PWMCheckTimer.tick()) {
    PWMvalue = pulseIn(PWM_PIN, HIGH, 50000UL);  // 50 millisecond timeout
  }
}
#endif
