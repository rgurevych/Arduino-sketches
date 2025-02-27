void checkInternetServices(){
  checkTelegram();
  autoUpdateTime();
}


void checkTelegram(){
  if (checkTelegramTimer.isReady() && mode == 0 && telegramEnabled){
    checkWiFi();
    if (WiFiReady) {
      if (sendDailyMeterValuesViaTelegram && publishDailyTelegramReport) {
        sendDailyMeterValues();
      }

      if (sendMonthlyMeterValuesViaTelegram && publishMonthlyTelegramReport) {
        sendMonthlyMeterValues();
      }
      
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while(numNewMessages) {
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
    }
  }
}


void checkWiFi(){
  if (WiFi.status() != WL_CONNECTED) {
    if (DEBUG_MODE){
      Serial.println(F("WiFi is not connected"));
    }
    WiFiReady = false;
  }
  else {
    if (DEBUG_MODE){
      Serial.print(F("Connected to WiFi. Local IP address:"));
      Serial.println(WiFi.localIP());
    }
    WiFiReady = true;
  }
}


void handleNewMessages(int numNewMessages) {
  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    String from_name = bot.messages[i].from_name;
    
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Sorry, " + from_name + ", you are not authorized to use this bot!", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
//    Serial.println(text);

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += F("The following commands are available for you: \n\n");
      welcome += F("/status: to receive the realtime status of your power line \n");
      welcome += F("/meter: to receive the current values of your power meter \n");
      welcome += F("/month: to request the meter value for the beginning of the month \n");
      welcome += F("/updateTime: to update system time from Internet \n");
      welcome += F("/enableDailyReport: to enable daily meter reports in Telegram \n");
      welcome += F("/disableDailyReport: to disable daily meter reports in Telegram \n");
      welcome += F("/enableMonthlyReport: to enable monthly meter reports in Telegram \n");
      welcome += F("/disableMonthlyReport: to disable monthly meter reports in Telegram \n");
      welcome += F("/options: to return the reply keyboard \n");
      
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/status") {
      String statusMessage = "Current status of your power line:\n";
      if (DEMO_MODE) statusMessage += F("!!! DEMO MODE ENABLED !!!\n");
      if (!meterPowered) statusMessage += F("!!! The meter is not powered !!!\n");
      statusMessage += "Voltage: " + String(mom_voltage / 10.0, 1) + " V\n";
      statusMessage += "Current: " + String(mom_current / 100.0, 2) + " A\n";
      statusMessage += "Power: " + String(mom_power) + " W\n";
      statusMessage += "Frequency: " + String(mom_frequency / 10.0, 1) + " Hz\n";
      statusMessage += "Power factor: " + String(mom_pf / 100.0, 2) + "\n";
      statusMessage += "Energy since last reset: " + String(mom_energy / 10.0, 1) + " kWh\n";
      bot.sendMessage(chat_id, statusMessage, "");
    }
    
    if (text == "/meter") {
      byte s;
      if (DEMO_MODE) s = 100; 
      else s = 0;
      
      getMeterData(s);
      String currentMeterMessage = "Current values of your meter:\n";
      if (DEMO_MODE) currentMeterMessage += F("!!! DEMO MODE ENABLED !!!\n");
      currentMeterMessage += "Day tariff: " + String(day_energy, 1) + " kWh \n";
      currentMeterMessage += "Night tariff: " + String(night_energy, 1) + " kWh \n";
      currentMeterMessage += "Total value: " + String(total_energy, 1) + " kWh \n";
      bot.sendMessage(chat_id, currentMeterMessage, "");
    }
    
    if (text == "/updateTime") {
      getNtpTime();
      saveNewTime(false);
      getTime();
      String updateTimeMessage = F("RTC time updated. New time and date: \n");
      updateTimeMessage += String(hour) + ":" + String(minute) + ":" + String(second) + "  ";
      updateTimeMessage += String(day) + "/" + String(month) + "/" + String(year);
      bot.sendMessage(chat_id, updateTimeMessage, "");
    }

    if (text == "/month") {
      float lastDayEnergy, lastNightEnergy;
      byte s;
      if (DEMO_MODE) s = 100; 
      else s = 0;
      EEPROMr.get(30+s, lastDayEnergy);
      EEPROMr.get(34+s, lastNightEnergy);
      String monthlyMeterMessage = "Meter values recorded for 01.";
      if (month < 10) monthlyMeterMessage += "0";
      monthlyMeterMessage += String(month) + ":\n";
      if (DEMO_MODE) monthlyMeterMessage += F("!!! DEMO MODE ENABLED !!!\n");
      monthlyMeterMessage += "Day: " + String(lastDayEnergy, 1) + " kWh \n";
      monthlyMeterMessage += "Night: " + String(lastNightEnergy, 1) + " kWh \n";
      monthlyMeterMessage += "Total: " + String(lastDayEnergy + lastNightEnergy, 1) + " kWh \n\n";
      bot.sendMessage(chat_id, monthlyMeterMessage, "");
    }

    if (text == "/enableDailyReport") {
      String enableDailyReportMessage = "";
      if (sendDailyMeterValuesViaTelegram) {
        enableDailyReportMessage += F("Daily reports via Telegram are already enabled! \n");
        enableDailyReportMessage += F("Settings were not updated. \n");
      }
      else {
        sendDailyMeterValuesViaTelegram = true;
        EEPROMr.put(40, sendDailyMeterValuesViaTelegram);
        EEPROMr.commit();
        enableDailyReportMessage += F("Daily reports via Telegram enabled! \n");
        enableDailyReportMessage += F("Settings were updated. \n");
      }
      bot.sendMessage(chat_id, enableDailyReportMessage, "");
    }

    if (text == "/disableDailyReport") {
      String disableDailyReportMessage = "";
      if (!sendDailyMeterValuesViaTelegram) {
        disableDailyReportMessage += F("Daily reports via Telegram are already disabled! \n");
        disableDailyReportMessage += F("Settings were not updated. \n");
      }
      else {
        sendDailyMeterValuesViaTelegram = false;
        EEPROMr.put(40, sendDailyMeterValuesViaTelegram);
        EEPROMr.commit();
        disableDailyReportMessage += F("Daily reports via Telegram disabled! \n");
        disableDailyReportMessage += F("Settings were updated. \n");
      }
      bot.sendMessage(chat_id, disableDailyReportMessage, "");
    }

    if (text == "/enableMonthlyReport") {
      String enableMonthlyReportMessage = "";
      if (sendMonthlyMeterValuesViaTelegram) {
        enableMonthlyReportMessage += F("Monthly reports via Telegram are already enabled! \n");
        enableMonthlyReportMessage += F("Settings were not updated. \n");
      }
      else {
        sendMonthlyMeterValuesViaTelegram = true;
        EEPROMr.put(41, sendMonthlyMeterValuesViaTelegram);
        EEPROMr.commit();
        enableMonthlyReportMessage += F("Monthly reports via Telegram enabled! \n");
        enableMonthlyReportMessage += F("Settings were updated. \n");
      }
      bot.sendMessage(chat_id, enableMonthlyReportMessage, "");
    }

    if (text == "/disableMonthlyReport") {
      String disableMonthlyReportMessage = "";
      if (!sendMonthlyMeterValuesViaTelegram) {
        disableMonthlyReportMessage += F("Monthly reports via Telegram are already disabled! \n");
        disableMonthlyReportMessage += F("Settings were not updated. \n");
      }
      else {
        sendMonthlyMeterValuesViaTelegram = false;
        EEPROMr.put(41, sendMonthlyMeterValuesViaTelegram);
        EEPROMr.commit();
        disableMonthlyReportMessage += F("Monthly reports via Telegram disabled! \n");
        disableMonthlyReportMessage += F("Settings were updated. \n");
      }
      bot.sendMessage(chat_id, disableMonthlyReportMessage, "");
    }

    if (text == "/options")
    {
      String keyboardJson = "[[\"/status\", \"/meter\"],[\"/month\", \"/start\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Choose from one of the following options", "", keyboardJson, true);
    }
  }
}


void sendDailyMeterValues() {
  // checkWiFi();
  if (WiFiReady) {
    String dailyMeterValueMessage = "Power consumption for yesterday was:\n";
    if (DEMO_MODE) dailyMeterValueMessage += F("!!! DEMO MODE ENABLED !!!\n");
    dailyMeterValueMessage += "Day: " + String(dailyDayEnergyDelta, 1) + " kWh \n";
    dailyMeterValueMessage += "Night: " + String(dailyNightEnergyDelta, 1) + " kWh \n";
    dailyMeterValueMessage += "Total: " + String(dailyDayEnergyDelta + dailyNightEnergyDelta, 1) + " kWh \n";
    if (bot.sendMessage(CHAT_ID, dailyMeterValueMessage, "")) {
      publishDailyTelegramReport = false;
    }
  }
}


void sendMonthlyMeterValues() {
  //checkWiFi();
  if (WiFiReady) {
    byte s;
    float currentDayEnergy, currentNightEnergy;
    if (DEMO_MODE) s = 100; 
    else s = 0;
    EEPROMr.get(30+s, currentDayEnergy);
    EEPROMr.get(34+s, currentNightEnergy);
    
    String MonthlyMeterValueMessage = "Power consumption for last month was:\n";
    if (DEMO_MODE) MonthlyMeterValueMessage += F("!!! DEMO MODE ENABLED !!!\n");
    MonthlyMeterValueMessage += "Day: " + String(monthlyDayEnergyDelta, 1) + " kWh \n";
    MonthlyMeterValueMessage += "Night: " + String(monthlyNightEnergyDelta, 1) + " kWh \n";
    MonthlyMeterValueMessage += "Total: " + String(monthlyDayEnergyDelta + monthlyNightEnergyDelta, 1) + " kWh \n\n";
    MonthlyMeterValueMessage += "Meter values recorded for 01.";
    if (month < 10) MonthlyMeterValueMessage += "0";
    MonthlyMeterValueMessage += String(month) + ":\n";
    MonthlyMeterValueMessage += "Day: " + String(currentDayEnergy, 1) + " kWh \n";
    MonthlyMeterValueMessage += "Night: " + String(currentNightEnergy, 1) + " kWh \n";
    MonthlyMeterValueMessage += "Total: " + String(currentDayEnergy + currentNightEnergy, 1) + " kWh \n\n";
    if (bot.sendMessage(CHAT_ID, MonthlyMeterValueMessage, "")) {
      publishMonthlyTelegramReport = false;
    }
  }
}


void showNetwork() {
  if (!screenReadyFlag) {
    lcd.clear();
  }

  if (!screenReadyFlag) {
    lcd.setCursor(0, 0);  lcd.print(F("WiFi connected: "));
    checkWiFi();
    if (WiFiReady) {
      lcd.print(F("Yes"));
    }
    else {
      lcd.print(F("No"));
    }
    lcd.setCursor(0, 1);  lcd.print(F("SSID: ")); lcd.print(WiFi.SSID());
    lcd.setCursor(0, 2);  lcd.print(F("IPv4: ")); lcd.print(WiFi.localIP());
    lcd.setCursor(0, 3);  lcd.print(F("Telegram bot: "));
    if (telegramEnabled) {
      lcd.print(F("On"));
    }
    else {
      lcd.print(F("Off"));
    }
    screenReadyFlag = true;
  }
  
  if (enc.click()) {
    mode = 1;
    screenReadyFlag = false;
    enc.resetState();
    menuExitTimer.start();
  }
}


void publishData() {
  if (publishDataTimer.isReady() && enableWiFi) {
    checkWiFi();
    if (WiFiReady) {
      float ts_voltage = av_voltage / 10.0;
      float ts_current = av_current / 100.0;
      float ts_energy = mom_energy / 10.0;
      if (ts_voltage > 0) ThingSpeak.setField(1, ts_voltage);
      if (ts_current > 0) ThingSpeak.setField(2, ts_current);
      if (av_power > 0) ThingSpeak.setField(3, int(av_power));
      ThingSpeak.setField(4, ts_energy);
      if (publishHourlyEnergyFlag) ThingSpeak.setField(5, energyDelta);
      if (publishDailyEnergyFlag) ThingSpeak.setField(6, dailyDayEnergyDelta + dailyNightEnergyDelta);
  
      int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

      if (x == 200) {
        publishHourlyEnergyFlag = false;
        publishDailyEnergyFlag = false;
      }
      if (DEBUG_MODE) {
        if(x == 200){
          Serial.println("Channel update successful.");
        }
        else{
          Serial.println("Problem updating channel. HTTP error code " + String(x));
        }
      }
    }
    resetAverageDataFlag = true;
  }
}
