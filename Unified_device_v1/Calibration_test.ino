
void calibrateAccel(){
  oled.clear();
  oled.home();
  oled.setScale(1);
  oled.println(F("Calibrate accel?"));
  oled.setCursor(0, 2);
  oled.println(F("L-Yes, R-Cancel"));

  while (true) {
    rightBtn.tick();
    leftBtn.tick();
    if(rightBtn.click()){
      resetFunc();
    }
    if(leftBtn.click()){
      oled.clear();
      oled.home();
      oled.println(F("Put device on flat"));
      oled.println(F("surface with Z-axis"));
      oled.println(F("strictly vertical"));
      oled.setCursor(0, 4);
      oled.println(F("L-Start, R-Reset"));
    
      while (true) {
        rightBtn.tick();
        leftBtn.tick();
        if(rightBtn.click()){
          resetFunc();
        }
        if(leftBtn.click()){
          doAccelCalibration();
          return;
        }
      }
    }
  }
}


void doAccelCalibration() {
  oled.clear();
  oled.home();
  oled.println(F("Calibration starts"));
  oled.println(F("in   seconds"));
  for(int8_t i = 5; i >= 0; i--){
    oled.setCursor(18, 1);
    oled.print(i);
    delay(1000);
  }
  oled.setCursor(0, 2);
  oled.print(F("Calibrating"));
  
  long offsets[6];
  long offsetsOld[6];
  int16_t mpuGet[6];
  boolean calibrationSuccessFlag = true;

  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);

  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  delay(5);
  
  for (byte n = 0; n < 10; n++){
    for (byte j = 0; j < 6; j++){
      offsets[j] = 0;
    }
    for (byte i = 0; i < 100 + CALIBRATION_BUFFER_SIZE; i++) {
      mpu.getMotion6(&mpuGet[0], &mpuGet[1], &mpuGet[2], &mpuGet[3], &mpuGet[4], &mpuGet[5]);
      if (i >= 99) {
        for (byte j = 0; j < 6; j++) {
          offsets[j] += (long)mpuGet[j];
        }
      }
    }
    for (byte i = 0; i < 6; i++) {
      offsets[i] = offsetsOld[i] - ((long)offsets[i] / CALIBRATION_BUFFER_SIZE);
      if (i == 2) offsets[i] += 16384;
      offsetsOld[i] = offsets[i];
    }

    mpu.setXAccelOffset(offsets[0] / 8);
    mpu.setYAccelOffset(offsets[1] / 8);
    mpu.setZAccelOffset(offsets[2] / 8);
    mpu.setXGyroOffset(offsets[3] / 4);
    mpu.setYGyroOffset(offsets[4] / 4);
    mpu.setZGyroOffset(offsets[5] / 4);
    delay(2);
    oled.print(F("."));
  }

  oled.setCursor(0, 3);
  oled.print(F("Verifying readings:"));
  mpu.getAcceleration(&ax, &ay, &az);

  oled.setCursor(0, 4);
  oled.print(ax); oled.print(F(" "));
  oled.print(ay); oled.print(F(" "));
  oled.print(az);

  oled.setCursor(0, 5);
  oled.print("X-");
  if(abs(ax) <= CALIBRATION_TOLERANCE) oled.print(F("OK"));
  else {
    oled.print(F("FAIL"));
    calibrationSuccessFlag = false;
  }
  oled.print(",Y-");
  if(abs(ay) <= CALIBRATION_TOLERANCE) oled.print(F("OK"));
  else {
    oled.print(F("FAIL"));
    calibrationSuccessFlag = false;
  }
  oled.print(",Z-");
  if(abs(az-16384) <= CALIBRATION_TOLERANCE) oled.print(F("OK"));
  else {
    oled.print(F("FAIL"));
    calibrationSuccessFlag = false;
  }
  
  oled.setCursor(0, 6);
  if(calibrationSuccessFlag){
    oled.print(F("SUCCESS!"));
    for (byte i = 0; i < 6; i++) {
      if (i < 3) offsets[i] /= 8;
      else offsets[i] /= 4;
    }
    EEPROM.put(ACCEL_OFFSETS_BYTE, offsets);
  }
  else oled.print(F("FAIL! Reset and retry"));
    
  oled.setCursor(0, 7);
  oled.print(F("Click L or R to reset"));
  while (true) {
    rightBtn.tick();
    leftBtn.tick();
    if(rightBtn.click() || leftBtn.click()){
      resetFunc();
    }
  } 
}