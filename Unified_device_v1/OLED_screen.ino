void changeMode(){
  if(mode != oldMode){
    
    if(mode == 1){
      oled.setCursor(48, 0);
      oled.print(F("IDLE         "));

      oled.setCursor(90, 2);
      oled.print(safetyGuardTimeout);
      if(DEMO_MODE) oled.print(F(" s"));
      else oled.print(F(" m"));

      oled.setCursor(90, 3);
      oled.print(selfDestructTimeout);
      if(DEMO_MODE) oled.print(F(" s"));
      else oled.print(F(" m"));

      oled.setCursor(90, 4);
      oled.print(accelerationLimit);
      oled.print(F(" G"));

      oled.setCursor(0, 6);
      oled.println(F("Hold R for settings  "));
      oled.println(F("Hold L+R 2s to start "));

      clearPointer();
    }

    else if(mode == 2){
      oled.setCursor(48, 0);
      oled.print(F("SETTINGS     "));

      oled.setCursor(90, 2);
      oled.print(safetyGuardTimeout);
      if(DEMO_MODE) oled.print(F(" s"));
      else oled.print(F(" m"));

      oled.setCursor(90, 3);
      oled.print(selfDestructTimeout);
      if(DEMO_MODE) oled.print(F(" s"));
      else oled.print(F(" m"));

      oled.setCursor(90, 4);
      oled.print(accelerationLimit);
      oled.print(F(" G"));

      oled.setCursor(0, 6);
      oled.println(F("L-move, Hold R-change"));
      oled.println(F("Hold L to save & exit"));
    }

    else if(mode == 3){
      updateScreenTimer.reset();
      
      oled.setCursor(48, 0);
      oled.print(F("CHANGE VALUE "));

      oled.setCursor(90, 2);
      oled.print(safetyGuardTimeout);
      if(DEMO_MODE) oled.print(F(" s"));
      else oled.print(F(" m"));

      oled.setCursor(90, 3);
      oled.print(selfDestructTimeout);
      if(DEMO_MODE) oled.print(F(" s"));
      else oled.print(F(" m"));

      oled.setCursor(90, 4);
      oled.print(accelerationLimit);
      oled.print(F(" G"));

      oled.setCursor(0, 6);
      oled.println(F("Click L/R for -/+    "));
      oled.println(F("Hold L to return     "));
    }

    else if(mode == 4){
      updateScreenTimer.reset();
      
      oled.setCursor(48, 0);
      oled.print(F("ACTIVE, SAFE "));

      oled.setCursor(0, 6);
      oled.println(F("                     "));
      oled.println(F("Hold L+R 2s to stop  "));
    }

    else if(mode == 5){
      oled.setCursor(48, 0);
      oled.print(F("ACTIVE, ARMED"));

      oled.setCursor(90, 2);
      oled.print(F("Off   "));

      oled.setCursor(0, 6);
      oled.println(F("                     "));
      oled.println(F("Hold L+R 2s to stop  "));
    }

    else if(mode == 6){
      oled.setCursor(48, 0);
      oled.print(F("COMPLETED    "));

      oled.setCursor(90, 2);
      oled.print(F("Off   "));

      oled.setCursor(90, 3);
      oled.print(F("Boom  "));

      oled.setCursor(0, 6);
      oled.println(F("                     "));
      oled.println(F("Hold L+R 2s to reset "));     
    }

    else if(mode == 7){
      oled.setCursor(48, 0);
      oled.print(F("COMPLETED    "));

      oled.setCursor(90, 4);
      oled.print(F("Boom  "));

      oled.setCursor(0, 6);
      oled.println(F("                     "));
      oled.println(F("Hold L+R 2s to reset "));     
    }

    oldMode = mode;
  }
}


void clearPointer(){
    for(byte i=2; i<5; i++){
      oled.setCursor(0, i);
      oled.print(F(" "));
  }
}


void updateScreen(){
  if(updateScreenTimer.isReady()){
    blinkFlag = !blinkFlag;

    if(mode == 2){
      for(byte i=2; i<5; i++){
        oled.setCursor(0, i);
        if(i == pointer) oled.print(F(">"));
        else oled.print(F(" "));
      }
    }

    if(mode == 3){
      oled.setCursor(0, pointer);
      if(blinkFlag) oled.print(F(">"));
      else oled.print(F(" "));

      if(pointer == 2){
        oled.setCursor(90, 2);
        if(blinkFlag) {
          oled.print(safetyGuardTimeout);
          if(DEMO_MODE) oled.print(F(" s"));
          else oled.print(F(" m"));
        }
        else oled.print(F("       "));
      }

      else if(pointer == 3){
        oled.setCursor(90, 3);
        if(blinkFlag) {
          oled.print(selfDestructTimeout);
          if(DEMO_MODE) oled.print(F(" s"));
          else oled.print(F(" m"));
        }
        else oled.print(F("       "));
      }

      else if(pointer == 4){
        oled.setCursor(90, 4);
        if(blinkFlag) {
          oled.print(accelerationLimit);
          oled.print(F(" G"));
        }
        else oled.print(F("       "));
      }
    }

    else if(mode == 4){
      
      oled.setCursor(90, 2);
      oled.print(safetyGuardTimeoutCounter / 60);
      if(blinkFlag) oled.print(F(":"));
      else oled.print(F(" "));
      if(safetyGuardTimeoutCounter % 60 < 10) oled.print(F("0"));
      oled.print(safetyGuardTimeoutCounter % 60);
      oled.print(F("    "));

      oled.setCursor(90, 3);
      oled.print(selfDestructTimeoutCounter / 60);
      if(blinkFlag) oled.print(F(":"));
      else oled.print(F(" "));
      if(selfDestructTimeoutCounter % 60 < 10) oled.print(F("0"));
      oled.print(selfDestructTimeoutCounter % 60);
      oled.print(F("    "));
    }

    else if(mode == 5 || mode == 7){
      oled.setCursor(90, 3);
      oled.print(selfDestructTimeoutCounter / 60);
      if(blinkFlag) oled.print(F(":"));
      else oled.print(F(" "));
      if(selfDestructTimeoutCounter % 60 < 10) oled.print(F("0"));
      oled.print(selfDestructTimeoutCounter % 60);
      oled.print(F("    "));
    }    
  }
}


void drawDefaultScreen(){
  oled.clear();
  oled.home();
  oled.setScale(1);

  if(DEMO_MODE) oled.print(F("!DEMO!: "));
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
  oled.println(F("Re-connect battery or"));
  oled.setCursor(0, 4);
  oled.println(F("check the wires"));

  delay(1000);
}