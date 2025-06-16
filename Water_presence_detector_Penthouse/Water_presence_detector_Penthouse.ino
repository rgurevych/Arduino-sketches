//Water pressure detector with Telegram bot by Rostyslav Gurevych

//---------- Include libraries
#include <GyverTimer.h>
#include <FastBot.h>
#include <EEPROM.h>
#include <GyverNTP.h>
 
//---------- Define pins and constants
#define DETECTOR_PIN 4                     //Button pin

//---------- Define variables
const char* ssid = "Penthouse_72";   // SSID for Penthouse
const char* password = "3Gurevych+1Mirkina";
#define BOTtoken "7107560647:AAEGsxdtWN0NAPAfYt1THw_eIXXI76Fv6yo"     //BOTtoken for Penthouse
#define MASTER_CHAT_ID "1289811885"
#define CHAT_ID "1289811885"     // Chat_ID for Penthouse
#define INIT_ADDR 500                    // Number of EEPROM initial cell
#define INIT_KEY 25                      // First launch key
 

// Define NTP Client and Telegram bot
FastBot bot(BOTtoken);
GTimer checkTimer(MS, 1000);
 
bool waterPresent;
bool currentState;
bool WiFiReady;
bool broadcastFlag = true;
unsigned long savedUnixTime, currentUnixTime, unixTimeDelta;

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

// Визначення поточного стану наявності води і збереження флагів
  waterPresent = digitalRead(DETECTOR_PIN);
  currentState = digitalRead(DETECTOR_PIN);

// Setting up NTP time client
  NTP.begin();
  while(!NTP.tick()) {}
  
// Визначення поточного часу, збереження його та виведення в консоль
  if(savedUnixTime == 0){
    savedUnixTime = getCurrentTimestamp();
    EEPROM.put(40, savedUnixTime);
    EEPROM.commit();
  }

//Визначення поточного часу через бібліотеку GyverNTP  
  currentUnixTime = getCurrentTimestamp();
  Serial.print("Поточний час через бібліотеку GyverNTP: ");
  Serial.println(currentUnixTime);
  
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

// Підключення обробника повідомлень до бота, відправлення стартового повідомлення
  bot.attach(newMsg);
  sendStartupMessage();
}


// Стартове повідомлення, що генерується і відправляється в чат його власнику
void sendStartupMessage(){
  FB_Time currentTime(currentUnixTime);
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
  startUpMessage += defineCurrentWaterState();
  startUpMessage += F("\nПоточний час і дата UTC: ");
  startUpMessage += currentTime.timeString();
  startUpMessage += F(" ");
  startUpMessage += currentTime.dateString();
  bot.sendMessage(startUpMessage, MASTER_CHAT_ID);
}


void loop() {
  checkWaterState();
  NTP.tick();
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


// Вивід поточного статуту води в консоль
void printWaterState(){
  Serial.print("Поточний стан: ");
  Serial.println(defineCurrentWaterState());
}


// Визначення поточного статуса води і повернення строки з відповідним текстом.
String defineCurrentWaterState(){
  if(currentState){
    return F("\U0001F6B0 Вода є!");
  }
  else{
    return F("\U0001F6B1 Воду вимкнули!");
  }
}


// Відправка поточного статусу води в телеграм
void sendCurrentWaterState(String ChatId){
  String botMessage = defineCurrentWaterState();
  if(currentState){
    botMessage += F("\n\U000023F1 Води не було ");
  }
  else{
    botMessage += F("\n\U000023F1 Вода була ");
  }
  botMessage += prepareTimeDeltaString(unixTimeDelta);
  
  Serial.println(botMessage);

  if(broadcastFlag && WiFiReady){
    bot.sendMessage(botMessage, ChatId);
  }
}


// Перевірка наявності тиску води
void checkWaterState(){
  if(checkTimer.isReady()){
    checkWiFi();
    waterPresent = digitalRead(DETECTOR_PIN);
    if(waterPresent != currentState){
      currentState = waterPresent;
      currentUnixTime = getCurrentTimestamp();
      unixTimeDelta = currentUnixTime - savedUnixTime;
      sendCurrentWaterState(CHAT_ID);

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
    welcome += F("Цей бот повідомлятиме про наявність води в ОСББ 'Славутич-22' \n");
    welcome += F("Нагадую команди, які ви можете відправляти боту: \n");
    welcome += F("/start: вивід цього повідомлення \n");
    welcome += F("/status: дізнатись поточну наявність води \n");
    welcome += F("/IPaddress: дізнатись поточну IP-адресу, до якої підключено пристрій \n");
    welcome += F("/broadcast: відсилати повідомлення в загальний канал \n");
    welcome += F("/no_broadcast: не відсилати повідомлення в загальний канал \n");
    welcome += F("/reset_timer: скинути таймер тривалості і почати новий відлік \n");
    welcome += F("Також можна відправити .bin файл з оновленням прошивки \n");
    bot.sendMessage(welcome, MASTER_CHAT_ID);
    return;
  }

  if(msg.text == "/status") {
    currentUnixTime = getCurrentTimestamp();
    unixTimeDelta = currentUnixTime - savedUnixTime;
    String statusMessage = defineCurrentWaterState();
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
    savedUnixTime = getCurrentTimestamp();
    EEPROM.put(40, savedUnixTime);
    EEPROM.commit();
    bot.sendMessage("Таймер розрахунку поточного періоду наявності/вісутності води скинуто, облік часу почався з 0", MASTER_CHAT_ID);
    return;
  }

  if (msg.OTA){
    bot.update();
  }
}

unsigned long getCurrentTimestamp(){
  return NTP.getUnix();
}
