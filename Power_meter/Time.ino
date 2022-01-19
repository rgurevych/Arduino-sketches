void getTime() {
  now = rtc.now();
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


void saveNewTime() {
  if (DEBUG_MODE){
    Serial.print(F("Saving new time to RTC module: "));
    Serial.print(new_hour); Serial.print(F(":")); Serial.print(new_minute); 
    Serial.print(F(":")); Serial.print(new_second);
    Serial.print(F("  ")); Serial.print(new_day); 
    Serial.print(F("/")); Serial.print(new_month); 
    Serial.print(F("/")); Serial.println(2000+new_year);
  }
  rtc.adjust(DateTime(2000+new_year, new_month, new_day, new_hour, new_minute, new_second));
}


void autoUpdateTime() {
  if (automaticallyUpdateTime) {
    if (minute == 30 && !autoUpdateTimeDoneFlag) {
      checkWiFi();
      if (WiFiReady) {
        getNtpTime();
        if (new_year + 2000 == year) {
          saveNewTime();
        }
      }
      autoUpdateTimeDoneFlag = true;
    }
    else if (minute != 30) {
      autoUpdateTimeDoneFlag = false;
    }
  }
}
