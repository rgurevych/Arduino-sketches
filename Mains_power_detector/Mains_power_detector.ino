//Mains power detector with Telegram bot by Rostyslav Gurevych

//---------- Include libraries
#include <GyverTimer.h>
#include <FastBot.h>
 
//---------- Define pins and constants
#define DETECTOR_PIN 4                     //Button pin

//---------- Define variables
const char* ssid = "Gurevych_2";
const char* password = "3Gurevych+1Mirkina";
#define BOTtoken "6408191151:AAG_VAOgyXl1x61B6gV9CJYbDpgD3t1Lygw"
//#define CHAT_ID "1289811885"
#define CHAT_ID "-1001813650904"
 
FastBot bot(BOTtoken);

GTimer checkTimer(MS, 1000);
GTimer checkTelegramTimer(MS, 3000);
 
bool powerPresent;
bool currentState;
 

void setup() {
  Serial.begin(9600);
 
  //Pin modes
  pinMode(DETECTOR_PIN, INPUT_PULLUP);
 
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

  powerPresent = !digitalRead(DETECTOR_PIN);
  currentState = !digitalRead(DETECTOR_PIN);

   bot.setChatID(CHAT_ID);
//   bot.attach(newMsg);
//   sendCurrentPowerState();
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
    powerPresent = !digitalRead(DETECTOR_PIN);
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
