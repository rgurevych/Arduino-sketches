void checkInternetServices(){
  checkTelegram();
  autoUpdateTime();
}


void checkTelegram(){
  if (checkTelegramTimer.isReady() && mode == 0 && telegramEnabled){
    checkWiFi();
    if (WiFiReady) {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while(numNewMessages) {
//        Serial.println(F("New message received"));
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
      welcome += F("/day: to receive the power consumption for the last day \n");
      welcome += F("/month: to request the meter value for the beginning of the month \n");
      welcome += F("/options: to return the reply keyboard \n");
      welcome += F("/updateTime: to update system time from Internet");
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
      saveNewTime();
      String updateTimeMessage = F("RTC time updated. New time and date: \n");
      updateTimeMessage += String(new_hour) + ":" + String(new_minute) + ":" + String(new_second) + "  ";
      updateTimeMessage += String(new_day) + "/" + String(new_month) + "/" + String(2000+new_year);
      bot.sendMessage(chat_id, updateTimeMessage, "");
    }

    if (text == "/options")
    {
      String keyboardJson = "[[\"/status\", \"/meter\"],[\"/day\", \"/month\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Choose from one of the following options", "", keyboardJson, true);
    }
  }
}


void sendDailyMeterValues(float dailyDayEnergyDelta, float dailyNightEnergyDelta) {
  checkWiFi();
  if (WiFiReady) {
    String dailyMeterValueMessage = "Power consumption for yesterday was:\n";
    if (DEMO_MODE) dailyMeterValueMessage += F("!!! DEMO MODE ENABLED !!!\n");
    dailyMeterValueMessage += "Day: " + String(dailyDayEnergyDelta, 1) + " kWh \n";
    dailyMeterValueMessage += "Night: " + String(dailyNightEnergyDelta, 1) + " kWh \n";
    dailyMeterValueMessage += "Total: " + String(dailyDayEnergyDelta + dailyNightEnergyDelta, 1) + " kWh \n";
    bot.sendMessage(CHAT_ID, dailyMeterValueMessage, "");
  }
}


void sendMonthlyMeterValues(float currentDayEnergy, float monthlyDayEnergyDelta, float currentNightEnergy, float monthlyNightEnergyDelta) {
  checkWiFi();
  if (WiFiReady) {
    String MontlyMeterValueMessage = "Power consumption for last month was:\n";
    if (DEMO_MODE) MontlyMeterValueMessage += F("!!! DEMO MODE ENABLED !!!\n");
    MontlyMeterValueMessage += "Day: " + String(monthlyDayEnergyDelta, 1) + " kWh \n";
    MontlyMeterValueMessage += "Night: " + String(monthlyNightEnergyDelta, 1) + " kWh \n";
    MontlyMeterValueMessage += "Total: " + String(monthlyDayEnergyDelta + monthlyNightEnergyDelta, 1) + " kWh \n\n";
    MontlyMeterValueMessage += "Meter values to be recorded for 01.";
    if (month < 10) MontlyMeterValueMessage += "0";
    MontlyMeterValueMessage += String(month) + ":\n";
    MontlyMeterValueMessage += "Day: " + String(currentDayEnergy, 1) + " kWh \n";
    MontlyMeterValueMessage += "Night: " + String(currentNightEnergy, 1) + " kWh \n";
    MontlyMeterValueMessage += "Total: " + String(currentDayEnergy + currentNightEnergy, 1) + " kWh \n\n";
    bot.sendMessage(CHAT_ID, MontlyMeterValueMessage, "");
  }
}
