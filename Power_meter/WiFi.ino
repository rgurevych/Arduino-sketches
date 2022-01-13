void checkTelegram(){
  if (checkTelegramTimer.isReady() && mode == 0){
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(F("Connecting to WiFi.."));
    }
    else {
      if (DEBUG_MODE){
        Serial.print(F("Connected to WiFi. Local IP address:"));
        Serial.println(WiFi.localIP());
      }
      
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while(numNewMessages) {
//        Serial.println(F("New message received"));
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
    }
  }
}


void handleNewMessages(int numNewMessages) {
//  Serial.print(F("Number of new messages:"));
//  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    String from_name = bot.messages[i].from_name;
    
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, F("Sorry, ") + from_name + F(", you are not authorized to use this bot!"), "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
//    Serial.println(text);

    if (text == "/start") {
      String welcome = F("Welcome, ") + from_name + F(".\n");
      welcome += F("The following commands are available for you: \n\n");
      welcome += F("/status to receive the realtime status of your power line \n");
      welcome += F("/meter to receive the current values of your power meter \n");
      welcome += F("/day to receive the power consumption for the last day \n");
      welcome += F("/month to request the meter value for the beginning of the month \n");
      welcome += F("/options to return the reply keyboard \n");
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/status") {
      String statusMessage = "Current status of your power line:\n";
      if (DEMO_MODE) statusMessage += F("!!! DEMO MODE ENABLED !!!\n");
      if (!meterPowered) statusMessage += F("!!! The meter is not powered !!!\n");
      statusMessage += F("Voltage: ") + String(mom_voltage / 10.0, 1) + F(" V\n");
      statusMessage += F("Current: ") + String(mom_current / 100.0, 2) + F(" A\n");
      statusMessage += F("Power: ") + String(mom_power) + F(" W\n");
      statusMessage += F("Frequency: ") + String(mom_frequency / 10.0, 1) + F(" Hz\n");
      statusMessage += F("Power factor: ") + String(mom_pf / 100.0, 2) + F("\n");
      statusMessage += F("Energy since last reset: ") + String(mom_energy / 10.0, 1) + F(" kWh\n");
      bot.sendMessage(chat_id, statusMessage, "");
    }
    
    if (text == "/meter") {
      byte s;
      if (DEMO_MODE) s = 100; 
      else s = 0;
      
      getMeterData(s);
      String currentMeterMessage = "Current values of your meter:\n";
      if (DEMO_MODE) currentMeterMessage += F("!!! DEMO MODE ENABLED !!!\n");
      currentMeterMessage += F("Day tariff: ") + String(day_energy, 1) + F("kWh \n");
      currentMeterMessage += F("Night tariff: ") + String(night_energy, 1) + F("kWh \n");
      currentMeterMessage += F("Total value: ") + String(total_energy, 1) + F("kWh \n");
      bot.sendMessage(chat_id, currentMeterMessage, "");
    }
    
//    if (text == "/state") {
//      if (digitalRead(ledPin)){
//        bot.sendMessage(chat_id, "LED is ON", "");
//      }
//      else{
//        bot.sendMessage(chat_id, "LED is OFF", "");
//      }
//    }

    if (text == "/options")
    {
      String keyboardJson = "[[\"/status\", \"/meter\"],[\"/day\", \"/month\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Choose from one of the following options", "", keyboardJson, true);
    }
  }
}
