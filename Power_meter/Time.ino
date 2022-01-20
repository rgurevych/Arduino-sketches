void getTime() {
  raw_now = rtc.now();
  now = dst_rtc.calculateTime(raw_now);
  second = now.second();
  minute = now.minute();
  hour = now.hour();
  day = now.day();
  month = now.month();
  year = now.year();
}


void getNtpTime(){
  timeClient.update();
  
  new_hour = timeClient.getHours();
  new_minute = timeClient.getMinutes();
  new_second = timeClient.getSeconds();

  //Get a time structure
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  new_day = ptm->tm_mday;
  new_month = ptm->tm_mon+1;
  new_year = ptm->tm_year-100;
}


void saveNewTime(bool autoCorrection) {
  if (DEBUG_MODE){
    Serial.print(F("Saving new time to RTC module: "));
    Serial.print(new_hour); Serial.print(F(":")); Serial.print(new_minute); 
    Serial.print(F(":")); Serial.print(new_second);
    Serial.print(F("  ")); Serial.print(new_day); 
    Serial.print(F("/")); Serial.print(new_month); 
    Serial.print(F("/")); Serial.println(2000+new_year);
  }

  rtc.adjust(DateTime(2000+new_year, new_month, new_day, new_hour, new_minute, new_second));

  if (autoCorrection) {
    DateTime standardTime = rtc.now();
    if (dst_rtc.checkDST(standardTime) == true) {           // check whether we're in DST right now. If we are, subtract an hour.
      standardTime = standardTime.unixtime() - 3600;
    }
    rtc.adjust(standardTime);
  }  
}


void autoUpdateTime() {
  if (automaticallyUpdateTime) {
    if (minute == 30 && !autoUpdateTimeDoneFlag) {
      checkWiFi();
      if (WiFiReady) {
        getNtpTime();
        if (new_year + 2000 == year) {
          saveNewTime(false);
        }
      }
      autoUpdateTimeDoneFlag = true;
    }
    else if (minute != 30) {
      autoUpdateTimeDoneFlag = false;
    }
  }
}
