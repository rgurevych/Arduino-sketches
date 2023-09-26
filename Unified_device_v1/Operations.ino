void ledCheck(){
  digitalWrite(LED_BUILTIN, ledFlag);
  if(mode == 4) digitalWrite(RELAY_1_TEST_PIN, ledFlag);
}

void safetyGuardEnable(){
  digitalWrite(RELAY_1_PIN, LOW);
}


void safetyGuardDisable(){
  digitalWrite(RELAY_1_PIN, HIGH);
}


void detonateEnable(){
  digitalWrite(RELAY_2_PIN, LOW);
}


void detonateDisable(){
  digitalWrite(RELAY_2_PIN, HIGH);
}
