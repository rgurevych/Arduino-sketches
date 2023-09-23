void ledCheck(){
  digitalWrite(LED_BUILTIN, ledFlag);
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
