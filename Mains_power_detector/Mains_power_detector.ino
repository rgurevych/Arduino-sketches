//Mains power detector with Telegram bot by Rostyslav Gurevych

//---------- Include libraries
#include <GyverTimer.h>
#include <FastBot.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
 
//---------- Define pins and constants
#define DETECTOR_PIN 4                     //Button pin

//---------- Define variables
const char* ssid = "Gurevych_2";
const char* password = "3Gurevych+1Mirkina";
#define BOTtoken "6408191151:AAG_VAOgyXl1x61B6gV9CJYbDpgD3t1Lygw"
#define MASTER_CHAT_ID "1289811885"
#define CHAT_ID "-1001813650904"
#define INIT_ADDR 500                    // Number of EEPROM initial cell
#define INIT_KEY 25                      // First launch key
 
FastBot bot(BOTtoken);

GTimer checkTimer(MS, 1000);
 
bool powerPresent;
bool currentState;
bool WiFiReady;
bool broadcastFlag = true;
uint32_t savedUnixTime, currentUnixTime, unixTimeDelta;


void setup(){
  Serial.begin(9600);
 
//Pin modes
  pinMode(DETECTOR_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

// Start EEPROM
  EEPROM.begin(512);
  
  if (EEPROM.read(INIT_ADDR) != INIT_KEY){
    EEPROM.write(INIT_ADDR, INIT_KEY);
    EEPROM.put(0, broadcastFlag);
    EEPROM.put(40, 0);
    EEPROM.commit();
  }

  EEPROM.get(0, broadcastFlag);
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

// Визначення поточного стану електроенергії і збереження флагів
  powerPresent = !digitalRead(DETECTOR_PIN);
  currentState = !digitalRead(DETECTOR_PIN);

// Підключення обробника повідомлень до бота, відправлення стартового повідомлення
  bot.attach(newMsg);
  sendStartupMessage();
  
// Визначення поточного часу, збереження його та виведення в консоль
  if(savedUnixTime == 0){
    savedUnixTime = bot.getUnix();
    EEPROM.put(40, savedUnixTime);
    EEPROM.commit();
  }

  currentUnixTime = bot.getUnix();
  
  FB_Time savedTime(savedUnixTime);
  FB_Time currentTime(currentUnixTime);

  Serial.print("Останній збережений час: ");
  Serial.print(savedTime.timeString());
  Serial.print(' ');
  Serial.println(savedTime.dateString());

  Serial.print("Поточний час: ");
  Serial.print(currentTime.timeString());
  Serial.print(' ');
  Serial.println(currentTime.dateString());
}


// Стартове повідомлення, що генерується і відправляється в чат його власнику
void sendStartupMessage(){
  String startUpMessage = F("\U000026A0 Я втрачав живлення, але зараз знову готовий до роботи! \n");
  startUpMessage += F("WiFi підключено, IP адреса: ");
  startUpMessage += WiFi.localIP().toString();
  if(broadcastFlag){
      startUpMessage += F("\nБот надсилає повідомлення в канал");
    }
    else{
      startUpMessage += F("\nБот не надсилає повідомлення в канал");
    }
  startUpMessage += F("\nПоточний стан: ");
  startUpMessage += defineCurrentPowerState();
  bot.sendMessage(startUpMessage, MASTER_CHAT_ID);
}


void loop() {
  checkPowerState();
  bot.tick();
}


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


// Вивід поточного статуту електроенергії в консоль
void printPowerState(){
  Serial.print("Поточний стан: ");
  Serial.println(defineCurrentPowerState());
}


// Визначення поточного статуту електроенергії і повернення строки з відповідним текстом.
String defineCurrentPowerState(){
  if(currentState){
    return F("\U0001F4A1 Світло є!");
  }
  else{
    return F("\U0000274C Світло вимкнули!");
  }
}


// Відправка поточного статусу електроенергії в телеграм
void sendCurrentPowerState(String ChatId){
  String botMessage = defineCurrentPowerState();
  unixTimeDelta = currentUnixTime - savedUnixTime;
  if(currentState){
    botMessage += F("\n\U000023F1 Темна пора тривала ");
  }
  else{
    botMessage += F("\n\U000023F1 Світла пора тривала ");
  }
  botMessage += prepareTimeDeltaString(unixTimeDelta);
  
  Serial.println(botMessage);

  if(broadcastFlag && WiFiReady){
    bot.sendMessage(botMessage, ChatId);
  }
}


// Перевірка наявності електроенергії
void checkPowerState(){
  if(checkTimer.isReady()){
    checkWiFi();
    powerPresent = !digitalRead(DETECTOR_PIN);
    if(powerPresent != currentState){
      currentState = powerPresent;
      currentUnixTime = bot.getUnix();
      sendCurrentPowerState(CHAT_ID);

      savedUnixTime = currentUnixTime;
      EEPROM.put(40, currentUnixTime);
      EEPROM.commit();
    }
  }
}


// Перетворює змінну у форматі unixtime у текстову розшифровку у форматі Х днів, Х годин, Х хвилин, Х секунд
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
    welcome += F("Цей бот повідомлятиме про наявність світла в ОСББ 'Парковий' \n");
    welcome += F("Нагадую команди, які ви можете відправляти боту: \n");
    welcome += F("/start: вивід цього повідомлення \n");
    welcome += F("/status: дізнатись поточну наявність світла \n");
    welcome += F("/IPaddress: дізнатись поточну IP-адресу, до якої підключено пристрій \n");
    welcome += F("/broadcast: відсилати повідомлення в загальний канал \n");
    welcome += F("/no_broadcast: не відсилати повідомлення в загальний канал \n");
    welcome += F("/reset_timer: скинути таймер тривалості і почати новий відлік \n");
    welcome += F("Також можна відправити .bin файл з оновленням прошивки \n");
    bot.sendMessage(welcome, MASTER_CHAT_ID);
    return;
  }

  if(msg.text == "/status") {
    currentUnixTime = bot.getUnix();
    unixTimeDelta = currentUnixTime - savedUnixTime;
    String statusMessage = defineCurrentPowerState();
    statusMessage += F("\n\U000023F1 Це триває вже ");
    statusMessage += prepareTimeDeltaString(unixTimeDelta);
    bot.sendMessage(statusMessage, MASTER_CHAT_ID);
    return;
  }

  if(msg.text == "/IPaddress") {
    bot.sendMessage(WiFi.localIP().toString(), MASTER_CHAT_ID);
    return;
  }

  if(msg.text == "/broadcast") {
    broadcastFlag = true;
    EEPROM.put(0, broadcastFlag);
    EEPROM.commit();
    bot.sendMessage("Бот БУДЕ надсилати повідомлення в канал", MASTER_CHAT_ID);
    return;
  }

  if(msg.text == "/no_broadcast") {
    broadcastFlag = false;
    EEPROM.put(0, broadcastFlag);
    EEPROM.commit();
    bot.sendMessage("Бот НЕ БУДЕ надсилати повідомлення в канал", MASTER_CHAT_ID);
    return;
  }

  if(msg.text == "/reset_timer") {
    savedUnixTime = bot.getUnix();
    EEPROM.put(40, savedUnixTime);
    EEPROM.commit();
    bot.sendMessage("Таймер розрахунку поточного періоду наявності/вісутності світла скинуто, облік часу почався з 0", MASTER_CHAT_ID);
    return;
  }

  if (msg.OTA){
    bot.update();
  }
}
