//Mains power detector with Telegram bot by Rostyslav Gurevych

//---------- Include libraries
#include <GyverTimer.h>
#include <FastBot.h>
 
//---------- Define pins and constants
#define BUTTON_1_PIN 0                     //Button pin

//---------- Define variables
const char* ssid = "Gurevych_2";
const char* password = "3Gurevych+1Mirkina";
#define BOTtoken "6408191151:AAG_VAOgyXl1x61B6gV9CJYbDpgD3t1Lygw"
#define CHAT_ID "1289811885"
 
FastBot bot(BOTtoken);

GTimer checkTimer(MS, 1000);
GTimer checkTelegramTimer(MS, 3000);
 
bool powerPresent;
bool currentState;
 

void setup() {
  Serial.begin(9600);
 
  //Pin modes
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
 
// Connect to WiFi
  Serial.print("Connecting to Wifi: ");
  Serial.println(ssid);
 
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  powerPresent = digitalRead(BUTTON_1_PIN);
  currentState = digitalRead(BUTTON_1_PIN);

   bot.setChatID(CHAT_ID); // передайте "" (пустую строку) чтобы отключить проверку
   bot.attach(newMsg);
   bot.sendMessage("ESP запущен и готов к работе!");
   sendCurrentPowerState();
}
 
void loop() {
  checkPowerState();
  bot.tick();
}

void printPowerState(){
  Serial.print("Current power state: ");
  Serial.println(currentState);
}

void sendCurrentPowerState(){
  if(currentState){
    bot.sendMessage("\U0001F4A1 Світло є!");
  }
  else{
    bot.sendMessage("\U0000274C Світло вимкнули!");
  }
}

void checkPowerState(){
  if(checkTimer.isReady()){
    powerPresent = digitalRead(BUTTON_1_PIN);
    if(powerPresent != currentState){
      currentState = powerPresent;
      printPowerState();
      sendCurrentPowerState();
    }
  }
}

void newMsg(FB_msg& msg) {
  Serial.println(msg.toString());
  if (msg.text == "/start") {
    String welcome = "Привіт, " + msg.username + "!\n";
    welcome += F("Цей бот повідомлятиме про наявність світла в ОСББ 'Парковий' \n");
    welcome += F("/status: для того щоб дізнатись поточну наявність світла. \n");
    bot.sendMessage(welcome);
  }

  if (msg.text == "/status") {
    sendCurrentPowerState();
  }
}

//U+2705
//void checkTelegram(){
//  if (checkTelegramTimer.isReady()){
//    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
//    while(numNewMessages) {
//      handleNewMessages(numNewMessages);
//      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
//    }
//  }
//}

//void handleNewMessages(int numNewMessages) {
//  for (int i=0; i<numNewMessages; i++) {
//    // Chat id of the requester
//    String chat_id = String(bot.messages[i].chat_id);
//    String from_name = bot.messages[i].from_name;
//    
//    if (chat_id != CHAT_ID){
//      bot.sendMessage(chat_id, "Sorry, " + from_name + ", you are not authorized to use this bot!", "");
//      continue;
//    }
//    
//    // Print the received message
//    String text = bot.messages[i].text;
//    Serial.println(text);
//
//    if (text == "/start") {
//      String welcome = "Welcome, " + from_name + ".\n";
//      welcome += F("This bot will inform you about the current power status in Bozdosh, OSBB Parkoviy \n");
//      welcome += F("/status: to receive the realtime status of your power line \n");
//      
//      bot.sendMessage(chat_id, welcome, "");
//    }
//  }
//}
