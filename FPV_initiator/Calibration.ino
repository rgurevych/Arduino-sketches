void doAccelCalibration() {
  Serial.print(F("Calibration starts in 5 seconds"));
  for(int8_t i = 5; i >= 0; i--){
    Serial.print(F("."));
    delay(1000);
  }
  Serial.println();
  Serial.print(F("Calibrating"));
  
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
    Serial.print(F("."));
  }

  Serial.println();
  Serial.println(F("Verifying readings:"));
  mpu.getAcceleration(&ax, &ay, &az);

  Serial.print(ax); Serial.print(F(" "));
  Serial.print(ay); Serial.print(F(" "));
  Serial.println(az);

  Serial.print("X-");
  if(abs(ax) <= CALIBRATION_TOLERANCE) Serial.print(F("OK"));
  else {
    Serial.print(F("FAIL"));
    calibrationSuccessFlag = false;
  }
  Serial.print(",Y-");
  if(abs(ay) <= CALIBRATION_TOLERANCE) Serial.print(F("OK"));
  else {
    Serial.print(F("FAIL"));
    calibrationSuccessFlag = false;
  }
  Serial.print(",Z-");
  if(abs(az-16384) <= CALIBRATION_TOLERANCE) Serial.print(F("OK"));
  else {
    Serial.print(F("FAIL"));
    calibrationSuccessFlag = false;
  }
  
  Serial.println();
  if(calibrationSuccessFlag){
    Serial.print(F("SUCCESS!"));
    for (byte i = 0; i < 6; i++) {
      if (i < 3) offsets[i] /= 8;
      else offsets[i] /= 4;
    }
    EEPROM.put(ACCEL_OFFSETS_BYTE, offsets);
    EEPROM.put(50, 0);
  }
  else Serial.print(F("FAIL! Reset and retry"));
    
  Serial.println();
  Serial.print(F("Click device button to reset"));
  while (true) {
    btn.tick();
    if(btn.click()) resetFunc();
  } 
}
