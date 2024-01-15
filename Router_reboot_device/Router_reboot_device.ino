//Router reboot device with Telegram bot by Rostyslav Gurevych

//---------- Include libraries
#include <TimerMs.h>
#include <FastBot.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
 
//---------- Define pins and constants
#define RELAY_PIN 4                     //Relay pin

//---------- Define variables
const char* ssid = "Gurevych_garage";
const char* password = "3Gurevych+1Mirkina";
//const char* remote_host_1 = "www.google.com";
const IPAddress remote_host_1 (8, 8, 8, 8);
#define BOTtoken "6672673335:AAFWsCDTqILKnjJ9CnurgNf8abaO8HSzyd4"
#define MASTER_CHAT_ID "1289811885"
#define CHAT_ID "1289811885"
#define INIT_ADDR 500                    // Number of EEPROM initial cell
#define INIT_KEY 25                      // First launch key
#define NETWORK_CHECK_PERIOD 60          // How often the network is checked, seconds
#define RETRIES_BEFORE_REBOOT 3          // How many times the connection should fail before rebooting the router
#define REBOOT_DELAY 10                  // How long the power is off during reboot
 
FastBot bot(BOTtoken);

TimerMs checkTimer(NETWORK_CHECK_PERIOD*1000, 1);
TimerMs rebootTimer(REBOOT_DELAY*1000, 0, 1);
TimerMs threeSecondTimer(3000, 0, 1);
 
bool networkPresent;
bool pingState;
bool WiFiReady;
bool activateRebootFlag = false;
bool rebootFlag = false;
bool rebootOngoingFlag = false;
bool rebootDoneFlag = false;
uint32_t savedUnixTime, currentUnixTime, unixTimeDelta;
byte ping_retry_counter = 0;


void setup(){
  Serial.begin(9600);
 
//Pin modes
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  powerOn();

// Start EEPROM
  EEPROM.begin(512);
  
  if (EEPROM.read(INIT_ADDR) != INIT_KEY){
    EEPROM.write(INIT_ADDR, INIT_KEY);
    EEPROM.put(40, 0);
    EEPROM.commit();
  }

  EEPROM.get(40, savedUnixTime);
 
// Connect to WiFi
  Serial.print("Connecting to Wifi: ");
  Serial.println(ssid);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(".");
    delay(500);
  }
  digitalWrite(LED_BUILTIN, LOW);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

// Підключення обробника повідомлень до бота, відправлення стартового повідомлення
  bot.attach(newMsg);
  sendStartupMessage();
  
// Визначення поточного часу та збереження його
  if(savedUnixTime == 0){
    savedUnixTime = bot.getUnix();
    EEPROM.put(40, savedUnixTime);
    EEPROM.commit();
  }

  currentUnixTime = bot.getUnix();
  
  FB_Time savedTime(savedUnixTime);
  FB_Time currentTime(currentUnixTime);
}


// Стартове повідомлення, що генерується і відправляється в чат-бот його власнику
void sendStartupMessage(){
  checkPing();
  String startUpMessage = F("\U000026A0 Я втрачав живлення разом із роутером, але зараз знову готовий до роботи! \n");
  startUpMessage += F("WiFi підключено, IP адреса: ");
  startUpMessage += WiFi.localIP().toString();

  startUpMessage += F("\nПоточний стан: ");
  startUpMessage += defineCurrentPingState();
  bot.sendMessage(startUpMessage, MASTER_CHAT_ID);
}


void loop() {
  bot.tick();
  operate();
}


void operate(){
  checkNetworkState();

  peformReboot();
  
  if (activateRebootFlag){
    if(threeSecondTimer.tick()){
      rebootFlag = true;
      return;
    }
  }

  if (rebootDoneFlag){
    if(threeSecondTimer.tick()){
      checkWiFi();
      if(WiFiReady){
        checkPing();
        if(pingState){
          updateRestartEventTime();
          rebootDoneFlag = false;
        }
        else{
          threeSecondTimer.start();
        }
      }
      else{
        threeSecondTimer.start();
      }
    }
  }
}


// Перевірка нявності підключення до WiFi
void checkWiFi(){
  if (WiFi.status() == WL_CONNECTED){
    digitalWrite(LED_BUILTIN, LOW);
    WiFiReady = true;
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
    WiFiReady = false;
  }
}


// Визначення поточного статуса підключення (пінга) і повернення рядка з відповідним текстом.
String defineCurrentPingState(){
  if(pingState){
    return F("\U00002705 Ping SUCCESS");
  }
  else{
    return F("\U0000274C Ping FAILED");
  }
}


// Відправка повідомлення про перезавантаження роутера в телеграм
void sendRestartMessage(String ChatId){
  String botMessage = F("\n\U0001F504 Роутер було перезавантажено \n");;
  botMessage += defineCurrentPingState();
  unixTimeDelta = currentUnixTime - savedUnixTime;
  botMessage += F("\n\U000023F1 Безперебійне підключення тривало ");
  botMessage += prepareTimeDeltaString(unixTimeDelta);

  if(WiFiReady){
    bot.sendMessage(botMessage, ChatId);
  }
}


// Перевірка стану підключення з обробкою лічильника невдалих пінгів
void checkNetworkState(){
  if(checkTimer.tick()){
    checkWiFi();
    if(WiFiReady){
      checkPing();
    }
    if(!pingState) {
      ping_retry_counter ++;
    }
    else {
      ping_retry_counter = 0;
    }

    if(ping_retry_counter >= RETRIES_BEFORE_REBOOT){
      ping_retry_counter = 0;
      rebootFlag = true;
    }
  }
}


// Збереження часу перезавантаження і відправка повідомлення в телеграм
void updateRestartEventTime(){
  currentUnixTime = bot.getUnix();
  sendRestartMessage(CHAT_ID);
  savedUnixTime = currentUnixTime;
  EEPROM.put(40, currentUnixTime);
  EEPROM.commit();
}


// Перевірка пінга
void checkPing(){
  if (Ping.ping(remote_host_1)){
    pingState = true;
  }
  else{
    pingState = false;
  }
}


// Перетворює змінну у форматі unixtime у текстову розшифровку у форматі D днів, H годин, M хвилин, S секунд
String prepareTimeDeltaString(uint32_t timeDelta){
  String timeDeltaString = "";
  
  uint8_t deltaSecond = unixTimeDelta % 60ul;
  unixTimeDelta /= 60ul;
  uint8_t deltaMinute = unixTimeDelta % 60ul;
  unixTimeDelta /= 60ul;
  uint8_t deltaHour = unixTimeDelta % 24ul;
  uint16_t deltaDay = unixTimeDelta /= 24ul;

  if(deltaDay > 0){
    timeDeltaString += String(deltaDay);
    if(deltaDay >= 5 && deltaDay <= 20){
      timeDeltaString += " днів ";
    }
    else if(deltaDay % 10 == 1){
      timeDeltaString += " день ";
    }
    else if(deltaDay % 10 == 2 || deltaDay % 10 == 3 || deltaDay % 10 == 4){
      timeDeltaString += " дні ";
    }
    else{
      timeDeltaString += " днів ";
    }
  }

  if(deltaHour > 0){
    timeDeltaString += String(deltaHour);
    timeDeltaString += " годин";
    if(deltaHour >= 5 && deltaHour <= 20){
      timeDeltaString += "";
    }
    else if(deltaHour % 10 == 1){
      timeDeltaString += "у";
    }
    else if(deltaHour % 10 == 2 || deltaHour % 10 == 3 || deltaHour % 10 == 4){
      timeDeltaString += "и";
    }
    timeDeltaString += " ";
  }

  if(deltaMinute > 0){
    timeDeltaString += String(deltaMinute);
    timeDeltaString += " хвилин";
    if(deltaMinute >= 5 && deltaMinute <= 20){
      timeDeltaString += "";
    }
    else if(deltaMinute % 10 == 1){
      timeDeltaString += "у";
    }
    else if(deltaMinute % 10 == 2 || deltaMinute % 10 == 3 || deltaMinute % 10 == 4){
      timeDeltaString += "и";
    }
    timeDeltaString += " ";
  }

  if(deltaSecond > 0){
    timeDeltaString += String(deltaSecond);
    timeDeltaString += " секунд";
    if(deltaSecond >= 5 && deltaSecond <= 20){
      timeDeltaString += "";
    }
    else if(deltaSecond % 10 == 1){
      timeDeltaString += "у";
    }
    else if(deltaSecond % 10 == 2 || deltaSecond % 10 == 3 || deltaSecond % 10 == 4){
      timeDeltaString += "и";
    }
  }

  return(timeDeltaString);
}


// Обробка повідомлень, що надсилають боту
void newMsg(FB_msg& msg) {
  Serial.println(msg.toString());

  if(msg.chatID != MASTER_CHAT_ID){
    if(msg.chatID != CHAT_ID){
      bot.sendMessage("Вибачте, " + msg.username + ", але ви не можете керувати цим ботом!", msg.chatID);
    }
    return;
  }
  
  if(msg.text == "/start") {
    String welcome = "Привіт, " + msg.username + "!\n";
    welcome += F("Цей бот повідомлятиме по перезавантаження роутера, підключеного до пристрою. \n");
    welcome += F("Нагадую команди, які ви можете відправляти боту: \n");
    welcome += F("/start: вивід цього повідомлення \n");
    welcome += F("/status: перевірити ping визначеного хоста \n");
    welcome += F("/reboot: перезавантажити роутер \n");
    welcome += F("/IPaddress: дізнатись поточну IP-адресу, до якої підключено пристрій \n");
    welcome += F("/reset_timer: скинути таймер тривалості і почати новий відлік \n");
    welcome += F("Також можна відправити .bin файл з оновленням прошивки \n");
    bot.sendMessage(welcome, MASTER_CHAT_ID);
    return;
  }

  if(msg.text == "/status") {
    checkPing();
    currentUnixTime = bot.getUnix();
    unixTimeDelta = currentUnixTime - savedUnixTime;
    String statusMessage = defineCurrentPingState();
    statusMessage += F("\n\U000023F1 Останнє перезавантаження роутера відбулось ");
    statusMessage += prepareTimeDeltaString(unixTimeDelta);
    statusMessage += F(" тому\n");
    bot.sendMessage(statusMessage, MASTER_CHAT_ID);
    return;
  }

  if(msg.text == "/reboot") {
    bot.sendMessage("Роутер буде перезавантажено через 3 секунди", MASTER_CHAT_ID);
    threeSecondTimer.start();
    activateRebootFlag = true;
    return;
  }

  if(msg.text == "/IPaddress") {
    bot.sendMessage(WiFi.localIP().toString(), MASTER_CHAT_ID);
    return;
  }

  if(msg.text == "/reset_timer") {
    savedUnixTime = bot.getUnix();
    EEPROM.put(40, savedUnixTime);
    EEPROM.commit();
    bot.sendMessage("Таймер поточного періоду наявності підключення до мережі Internet скинуто, облік часу почався з 0", MASTER_CHAT_ID);
    return;
  }

  if (msg.OTA){
    bot.update();
  }
}


void powerOff(){
  digitalWrite(RELAY_PIN, LOW);
}


void powerOn(){
  digitalWrite(RELAY_PIN, HIGH);
}


void peformReboot(){
  if (rebootFlag){
    if (!rebootOngoingFlag){
      powerOff();
      rebootOngoingFlag = true;
      rebootTimer.start();
    }
    else{
      if(rebootTimer.tick()){
        powerOn();
        rebootOngoingFlag = false;
        rebootFlag = false;
        activateRebootFlag = false;
        rebootDoneFlag = true;
        threeSecondTimer.start();
      }
    }
  }
}
