void changeMode(){
  if(mode == oldMode){
    return;
  }

  oldMode = mode;
  
  if(mode == 1){
    oled.setCursor(48, 0);
    oled.print(F("IDLE         "));
    if(PWMremote){
      oled.setCursor(72, 0);
      oled.print(F(", PWM "));
    }

    oled.setCursor(90, 2);
    oled.print(safetyGuardTimeout);
    if(demoMode) oled.print(F(" s   "));
    else oled.print(F(" m   "));

    oled.setCursor(90, 3);
    if(selfDestructTimeout == 0) oled.print(F("Off   "));
    else {
      oled.print(selfDestructTimeout);
      if(demoMode) oled.print(F(" s   "));
      else oled.print(F(" m   "));
    }

    oled.setCursor(90, 4);
    oled.print(accelerationLimit);
    oled.print(F(" G  "));

    oled.setCursor(0, 6);
    oled.println(F("Hold R for settings  "));
    oled.println(F("Hold L+R 2s to start "));

    clearPointer();
    return;
  }

  if(mode == 2){
    oled.setCursor(48, 0);
    oled.print(F("SETTINGS     "));

    oled.setCursor(90, 2);
    oled.print(safetyGuardTimeout);
    if(demoMode) oled.print(F(" s   "));
    else oled.print(F(" m   "));

    oled.setCursor(90, 3);
    if(selfDestructTimeout == 0) oled.print(F("Off   "));
    else {
      oled.print(selfDestructTimeout);
      if(demoMode) oled.print(F(" s   "));
      else oled.print(F(" m   "));
    }

    oled.setCursor(90, 4);
    oled.print(accelerationLimit);
    oled.print(F(" G  "));

    oled.setCursor(0, 6);
    oled.println(F("L-move, Hold R-change"));
    oled.println(F("Hold L to save & exit"));
    return;
  }

  if(mode == 3){
    oled.setCursor(48, 0);
    oled.print(F("CHANGE VALUE "));

    oled.setCursor(90, 2);
    oled.print(safetyGuardTimeout);
    if(demoMode) oled.print(F(" s"));
    else oled.print(F(" m"));

    oled.setCursor(90, 3);
    if(selfDestructTimeout == 0) oled.print(F("Off   "));
    else{
      oled.print(selfDestructTimeout);
      if(demoMode) oled.print(F(" s"));
      else oled.print(F(" m"));
    }

    oled.setCursor(90, 4);
    oled.print(accelerationLimit);
    oled.print(F(" G  "));

    oled.setCursor(0, 6);
    oled.println(F("Click L/R for -/+    "));
    oled.println(F("Hold L to return     "));
    return;
  }

  if(mode == 4){
    updateScreenTimer.start();
    
    oled.setCursor(48, 0);
    oled.print(F("DISARMED, PWM "));

    oled.setCursor(0, 6);
    oled.println(F("Ignoring PWM:        "));
    oled.println(F("Hold L+R 2s to stop  "));
    return;
  }

  if(mode == 5){
    updateScreenTimer.start();
    
    oled.setCursor(48, 0);
    oled.print(F("SAFE, PWM exp."));

    oled.setCursor(0, 6);
    oled.println(F("Waiting for PWM input"));
    oled.println(F("Hold L+R 2s to stop  "));
    return;
  }

  if(mode == 6){
    updateScreenTimer.start();
    
    oled.setCursor(48, 0);
    oled.print(F("ACTIVE, SAFE  "));

    oled.setCursor(0, 6);
    oled.println(F("                     "));
    oled.println(F("Hold L+R 2s to stop  "));
    return;
  }

  if(mode == 7){
    if(!demoMode){
      oled.clear();
    }
    else{
      oled.setCursor(48, 0);
      oled.print(F("ACTIVE, ARMED "));

      oled.setCursor(90, 2);
      oled.print(F("Off   "));

      oled.setCursor(0, 6);
      if(debugMode) oled.println(F(" Debug max accel: "));
      else oled.println(F("                     "));
      oled.println(F("Hold L+R 4s to stop  "));
    }
    return;
  }

  if(mode == 8){
    if(!demoMode) drawDefaultScreen();

    oled.setCursor(48, 0);
    oled.print(F("DETONATE     "));

    oled.setCursor(90, 3);
    if (detonateByTimerFlag) oled.print(F("Boom  ")); else oled.print(F("Off  "));

    oled.setCursor(90, 4);
    if (detonateByAccelFlag) oled.print(F("Boom   ")); else oled.print(F("Off  "));

    oled.setCursor(0, 6);
    oled.println(F("                     "));
    oled.println(F("Hold L+R 2s to reset ")); 
    return;
  }
}


void clearPointer(){
    for(byte i=2; i<5; i++){
      oled.setCursor(0, i);
      oled.print(F(" "));
  }
}


void updateScreen(){
  if(!updateScreenTimer.tick()){
    return;
  }
  blinkFlag = !blinkFlag;

  if(mode == 2){
    for(byte i=2; i<5; i++){
      oled.setCursor(0, i);
      if(i == pointer) oled.print(F(">"));
      else oled.print(F(" "));
    }
    return;
  }

  if(mode == 3){
    oled.setCursor(0, pointer);
    if(blinkFlag) oled.print(F(">"));
    else oled.print(F(" "));

    if(pointer == 2){
      oled.setCursor(90, 2);
      if(blinkFlag) {
        oled.print(safetyGuardTimeout);
        if(demoMode) oled.print(F(" s"));
        else oled.print(F(" m"));
      }
      else oled.print(F("       "));
      return;
    }

    if(pointer == 3){
      oled.setCursor(90, 3);
      if(blinkFlag) {
        if(selfDestructTimeout == 0) oled.print(F("Off   "));
        else{
          oled.print(selfDestructTimeout);
          if(demoMode) oled.print(F(" s"));
          else oled.print(F(" m"));
        }
      }
      else oled.print(F("       "));
      return;
    }

    if(pointer == 4){
      oled.setCursor(90, 4);
      if(blinkFlag) {
        oled.print(accelerationLimit);
        oled.print(F(" G"));
      }
      else oled.print(F("       "));
      return;
    }
  }

  if(mode == 4){
    oled.setCursor(84, 6);
    oled.print(disarmedTimeoutCounter / 60);
    if(blinkFlag) oled.print(F(":"));
    else oled.print(F(" "));
    if(disarmedTimeoutCounter % 60 < 10) oled.print(F("0"));
    oled.print(disarmedTimeoutCounter % 60);
    oled.print(F(" "));
    getPWM();
    if(abs(PWMvalue - SAFETY_PWM) < 50) {
      oled.print(F("*  "));  
    }
    else {
      oled.print(F("-  "));
    }
    return;
  }

  if(mode == 6){
    oled.setCursor(90, 2);
    oled.print(safetyGuardTimeoutCounter / 60);
    if(blinkFlag) oled.print(F(":"));
    else oled.print(F(" "));
    if(safetyGuardTimeoutCounter % 60 < 10) oled.print(F("0"));
    oled.print(safetyGuardTimeoutCounter % 60);
    oled.print(F("   "));

    oled.setCursor(90, 3);
    if(selfDestructTimeout == 0) oled.print(F("Off   "));
    else{
      oled.print(selfDestructTimeoutCounter / 60);
      if(blinkFlag) oled.print(F(":"));
      else oled.print(F(" "));
      if(selfDestructTimeoutCounter % 60 < 10) oled.print(F("0"));
      oled.print(selfDestructTimeoutCounter % 60);
      oled.print(F("   "));
    }
    return;
  }

  if(mode == 7){
    if(demoMode){
      oled.setCursor(90, 3);
      if(selfDestructTimeout == 0) oled.print(F("Off   "));
      else{
        oled.print(selfDestructTimeoutCounter / 60);
        if(blinkFlag) oled.print(F(":"));
        else oled.print(F(" "));
        if(selfDestructTimeoutCounter % 60 < 10) oled.print(F("0"));
        oled.print(selfDestructTimeoutCounter % 60);
        oled.print(F("   "));
      }

      if(debugMode){
        oled.setCursor(108, 6);
        oled.print(debugMaxAccel);
        oled.print(F("  "));
      }
      return;
    }
    else{
      oled.home();
      if(blinkFlag) oled.print(F("*"));
      else oled.print(F(" "));
    }
  }    
}


void drawDefaultScreen(){
  oled.clear();
  oled.home();
  oled.setScale(1);

  if(demoMode) oled.print(F("!DEMO!: "));
  else oled.print(F("Status: "));
  
  oled.line(0, 12, 127, 12);

  oled.setCursor(0, 2);
  oled.println(F(" Safety guard:       "));
  oled.println(F(" Self-destroy:       "));
  oled.println(F(" Acceleration:       "));

  oled.line(0, 44, 127, 44);
}


void drawIntroScreen(){
  oled.clear();
  oled.setScale(1);
  oled.setCursor(96, 0);
  oled.print(F("v"));
  oled.print(VERSION, 2);

  oled.setCursor(42, 1);
  oled.println(F("MAY THE"));

  oled.setScale(2);
  oled.setCursor(12, 3);
  oled.println(F("SCHWARTZ"));

  oled.setScale(1);
  oled.setCursor(30, 6);
  oled.println(F("BE WITH YOU"));
}


void drawErrorIntroScreen(){
  oled.clear();
  oled.setScale(1);
  oled.setCursor(0, 1);
  oled.println(F("Accel check failed!"));
  oled.setCursor(0, 2);
  oled.println(F("Device is not usable"));
  oled.setCursor(0, 3);
  oled.println(F("Check the wiring"));

  delay(1000);
}
