/* Power meter by R. Gurevych */

// Includes
#include <ArduinoJson.h>
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <GyverTimer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <EncButton.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <DST_RTC.h>
#include <ThingSpeak.h>

// Pins
#define PZEM_RX_PIN 12
#define PZEM_TX_PIN 14
#define CLKe 4             // encoder S1
#define DTe 5              // encoder S2
#define SWe 13             // encoder Key
#define BACKLIGHT 15         //LCD backlight pin


// Timer durations
#define MEASURE_TIMEOUT 1500             //Interval between measure occurs
#define TIME_TICKER 1000                 //Interval for reading the time from RTC module
#define PRINT_TIMEOUT 500                //Interval between printing to the screen occurs
#define MENU_EXIT_TIMEOUT 120000         //Interval for automatic exit from menu
#define CHECK_TELEGRAM_TIMEOUT 10000     //Interval for checking Telegram bot
#define PUBLISH_DATA_TIMEOUT 60000       //Interval for publishing data to ThingSpeak


// Settings
#define INIT_ADDR 500                    // Number of EEPROM initial cell
#define INIT_KEY 0                        // First launch key
#define DEBUG_MODE 1
#define RESET_CLOCK 0
#define DAY_TARIFF_START 7
#define NIGHT_TARIFF_START 23
// Newtork credentials
const char* ssid = "Penthouse_72";
const char* password = "3Gurevych+1Mirkina";
#define BOTtoken "5089942864:AAGk7ItUZyzCrfXsIWWRWaWHzY2TZAEZLjs"
#define CHAT_ID "1289811885"
unsigned long myChannelNumber = 1637460;
const char * myWriteAPIKey = "5QL2A6124OLI8Y1X";
const char rulesDST[] = "EU";
bool DEMO_MODE = true;
bool telegramEnabled = true;
bool automaticallyUpdateTime = true;
bool sendDailyMeterValuesViaTelegram = true;
bool sendMonthlyMeterValuesViaTelegram = true;
bool enableWiFi = true;


// Timers:
GTimer measureTimer(MS, MEASURE_TIMEOUT);
GTimer printTimer(MS, PRINT_TIMEOUT);
GTimer timeTimer(MS, TIME_TICKER);
GTimer menuExitTimer;
GTimer checkTelegramTimer(MS, CHECK_TELEGRAM_TIMEOUT);
GTimer publishDataTimer(MS, PUBLISH_DATA_TIMEOUT);


// Setting up modules
SoftwareSerial pzemSWSerial(PZEM_TX_PIN, PZEM_RX_PIN);
PZEM004Tv30 pzem;
LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS3231 rtc;
DST_RTC dst_rtc;
DateTime now, raw_now;
EncButton<EB_TICK, CLKe, DTe, SWe> enc;
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
WiFiClient tsClient;

// Global variables
uint8_t hour, minute, second, month, day;
int8_t new_hour, new_minute, new_second, new_month, new_day, new_year;
uint16_t year;
uint16_t mom_voltage = 0;
uint16_t mom_current = 0;
uint16_t mom_power = 0;
uint32_t mom_energy = 0;
uint16_t mom_frequency = 0;
uint16_t mom_pf = 0;
byte mode = 0;
byte screen = 0;
int8_t menu = 1;
float latest_energy = 0;
float day_energy = 0;
float night_energy = 0;
float total_energy = 0;
byte lcd_bright = 5;
byte timezone = 2;
long utcOffsetInSeconds = 3600*timezone;
bool newDemoMode;


//Flags
bool screenReadyFlag = false;
bool lcdBacklight = true;
bool recordMeterDoneFlag = false;
bool blinkFlag = true;
bool autoUpdateTimeDoneFlag = false;
bool meterPowered;
bool WiFiReady;

void(* resetFunc) (void) = 0;

void setup() {
  if (DEBUG_MODE) {
    Serial.begin(115200);
  }

  EEPROM.begin(512);
  
  // Reset to default settings
  if (EEPROM.read(INIT_ADDR) != INIT_KEY) {
    EEPROM.write(INIT_ADDR, INIT_KEY);
    EEPROM.put(0, latest_energy);                       //latest recorded energy value for normal mode
    EEPROM.put(4, day_energy);                          //latest recorded day tariff energy for normal mode
    EEPROM.put(8, night_energy);                        //latest recorded night tariff energy for normal mode
    EEPROM.put(12, total_energy);                       //latest recorded total energy for normal mode
    EEPROM.put(16, lcd_bright);                         //LCD backlight brightness value
    EEPROM.put(17, telegramEnabled);                    //is telegram bot enabled?
    EEPROM.put(18, 0);                                  //Demo mode (true/false)
    EEPROM.put(19, automaticallyUpdateTime);            //should time be updated automatically
    EEPROM.put(20, 0);                                  //latest recorded daily value for day tariff for normal mode
    EEPROM.put(24, 0);                                  //latest recorded daily value for night tariff for normal mode
    EEPROM.put(30, 0);                                  //latest recorded monthly value for day tariff for normal mode
    EEPROM.put(34, 0);                                  //latest recorded monthly value for night tariff for normal mode
    EEPROM.put(40, sendDailyMeterValuesViaTelegram);    //Should daily reports be sent via telegram
    EEPROM.put(41, sendMonthlyMeterValuesViaTelegram);  //Should monthly reports be sent via telegram
    EEPROM.put(42, enableWiFi);                         //is WiFi connection enabled?
    EEPROM.put(100, latest_energy);                     //latest recorded energy value for demo mode
    EEPROM.put(104, day_energy);                        //latest recorded day tariff energy for demo mode
    EEPROM.put(108, night_energy);                      //latest recorded night tariff energy for demo mode
    EEPROM.put(112, total_energy);                      //latest recorded total energy for demo mode
    EEPROM.put(120, 0);                                 //latest recorded daily value for day tariff for demo mode
    EEPROM.put(124, 0);                                 //latest recorded daily value for night tariff for demo mode
    EEPROM.put(130, 0);                                 //latest recorded monthly value for day tariff for normal mode
    EEPROM.put(134, 0);                                 //latest recorded monthly value for night tariff for normal mode
    EEPROM.commit();
  }

  getBrightness();

  Wire.begin(0, 2);
  
  // RTC module
  rtc.begin();
  if (RESET_CLOCK || rtc.lostPower()){
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  raw_now = rtc.now();
  
  pzemSWSerial.begin(9600);
  pzem = PZEM004Tv30(pzemSWSerial);

  EEPROM.get(18, DEMO_MODE);
  EEPROM.get(17, telegramEnabled);
  EEPROM.get(19, automaticallyUpdateTime);
  EEPROM.get(40, sendDailyMeterValuesViaTelegram);
  EEPROM.get(41, sendMonthlyMeterValuesViaTelegram);
  EEPROM.get(42, enableWiFi);
  
  lcd.init();
  lcd.setBacklight(lcdBacklight);
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print(F("POWER METER"));
  lcd.setCursor(9, 1);
  lcd.print(F("by"));
  lcd.setCursor(1, 2);
  lcd.print(F("Rostyslav Gurevych"));
  lcd.setCursor(16, 3);
  lcd.print(F("v1.0"));
  delay(3000);
  lcd.clear();
  if (DEMO_MODE) {
    lcd.setCursor(2, 1);
    lcd.print(F("Demo mode enabled"));
    randomSeed(analogRead(0));
    screen = 1;
  }
  else {
    if (pzem.readAddress() == 0) {
      meterPowered = false;
      lcd.setCursor(5, 0);
      lcd.print(F("The meter"));
      lcd.setCursor(3, 1);
      lcd.print(F("is not powered"));
      lcd.setCursor(2, 3);
      lcd.print(F("Check connection"));
    }
    else {
      meterPowered = true;
      lcd.setCursor(3, 1);
      lcd.print(F("Initialization"));
      lcd.setCursor(6, 2);
      lcd.print(F("completed"));
    }
  }
  delay(2000);
  menuExitTimer.setTimeout(MENU_EXIT_TIMEOUT);
  menuExitTimer.start();
  configTime(0, 0, "pool.ntp.org");
  client.setTrustAnchors(&cert);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  if (enableWiFi) {
    WiFi.begin(ssid, password);
  }
  
  timeClient.begin();
  timeClient.setTimeOffset(utcOffsetInSeconds);

  ThingSpeak.begin(tsClient);
}


void loop() {
  enc.tick();
  getPowerData();
  checkMode();
  recordMeter();
  checkInternetServices();
  publishData();
}


void checkMode(){
  if(menuExitTimer.isReady()) {
    lcdBacklight = false;
    lcd.setBacklight(lcdBacklight);
    enc.resetState();
    if (mode != 0) {
      mode = 0;
      screen = 1;
      screenReadyFlag = false;
    }
  }
  
  if (mode == 0) {
    printPowerData();
  }
  else {
    backgroundTime();
  }
  
  if (mode == 1) {
    mainMenu();
  }
  
  if (mode == 2) {
    showMeter();
  }

  if (mode == 4) {
    showNetwork();
  }

  if (mode == 5) {
    settingsMenu();
  }

  if (mode == 6) {
    setTime();
  }

  if (mode == 7) {
    setMeter();
  }

  if (mode == 8) {
    setBright();
  }

  if (mode == 9) {
    setDemoMode();
  }

  if (mode == 10) {
    resetMeter();
  }

  if (mode == 11) {
    performReset();
  }
}


void getBrightness(){
  EEPROM.get(16, lcd_bright);
  analogWrite(BACKLIGHT, lcd_bright*15);
}


void printPowerData() {
  if (enc.turn()) {
    lcdBacklight = true;
    lcd.setBacklight(lcdBacklight);
    menuExitTimer.start();
  }
  
  if (enc.click()) {
    lcdBacklight = true;
    lcd.setBacklight(lcdBacklight);
    mode = 1;
    screenReadyFlag = false;
    screen = 0;
    menu = 1;
    enc.resetState();
    menuExitTimer.start();
  }
  
  if (printTimer.isReady()){

    getTime();
    
    if (DEBUG_MODE) {
      Serial.print(hour); Serial.print(F(":")); Serial.print(minute); Serial.print(F(":")); Serial.print(second);
      Serial.print(F("  ")); Serial.print(day); Serial.print(F("/")); Serial.print(month); Serial.print(F("/")); Serial.println(year);
      Serial.print(F("U: "));      Serial.print(mom_voltage / 10.0);      Serial.println(F("V"));
      Serial.print(F("I: "));      Serial.print(mom_current / 100.0);     Serial.println(F("A"));
      Serial.print(F("P: "));      Serial.print(mom_power);               Serial.println(F("W"));
      Serial.print(F("E: "));      Serial.print(mom_energy / 10.0);       Serial.println(F("kWh"));
      Serial.print(F("F: "));      Serial.print(mom_frequency / 10.0);    Serial.println(F("Hz"));
      Serial.print(F("PF: "));     Serial.println(mom_pf / 100.0);        Serial.println();
    }

    if (!screenReadyFlag) {
      lcd.clear();
    }
    
    if (screen == 0) {
      if (!screenReadyFlag){
        lcd.setCursor(5, 0);
        lcd.print(F("The meter"));
        lcd.setCursor(3, 1);
        lcd.print(F("is not powered"));
        screenReadyFlag = true;
      }
    }
  
    if (screen == 1) {
      if (!screenReadyFlag){
        if (DEMO_MODE) {
          lcd.setCursor(18, 0); lcd.print(F("DM"));
        }
        else {
          lcd.setCursor(18, 0); lcd.print(F("NW"));
        }
        lcd.setCursor(2, 0); lcd.print(F(":")); lcd.setCursor(5,0); lcd.print(F(":"));
        lcd.setCursor(11, 0); lcd.print(F("/")); lcd.setCursor(14,0); lcd.print(F("/"));
        lcd.setCursor(0, 1); lcd.print(F("U=")); lcd.setCursor(7,1); lcd.print(F("V"));
        lcd.setCursor(12, 1); lcd.print(F("I=")); lcd.setCursor(19,1); lcd.print(F("A"));
        lcd.setCursor(0, 2); lcd.print(F("P=")); lcd.setCursor(7,2); lcd.print(F("W"));
        lcd.setCursor(12, 2); lcd.print(F("F=")); lcd.setCursor(18,2); lcd.print(F("Hz"));
        lcd.setCursor(0, 3); lcd.print(F("E=")); lcd.setCursor(8,3); lcd.print(F("kWh"));
        lcd.setCursor(13, 3); lcd.print(F("PF=")); 
        screenReadyFlag = true;
      }
  
      lcd.setCursor(0,0); if (hour < 10){lcd.print(F("0"));} lcd.print(hour); 
      lcd.setCursor(3,0); if (minute < 10){lcd.print(F("0"));} lcd.print(minute); 
      lcd.setCursor(6,0); if (second < 10){lcd.print(F("0"));} lcd.print(second);
      lcd.setCursor(9,0); if (day < 10){lcd.print(F("0"));} lcd.print(day); 
      lcd.setCursor(12,0); if (month < 10){lcd.print(F("0"));} lcd.print(month); 
      lcd.setCursor(15,0); lcd.print(year-2000);
      lcd.setCursor(2,1); lcd.print(mom_voltage / 10.0, 1);
      lcd.setCursor(14,1); if(mom_current < 1000){lcd.print(F(" "));} lcd.print(mom_current / 100.0, 2);
      lcd.setCursor(2,2); if(mom_power < 10000){lcd.print(F(" "));} if(mom_power < 1000){lcd.print(F(" "));} 
      if(mom_power < 100){lcd.print(F(" "));} if(mom_power < 10){lcd.print(F(" "));} lcd.print(mom_power);
      lcd.setCursor(14,2); lcd.print(mom_frequency / 10.0, 1);
      lcd.setCursor(2,3); printEnergy(mom_energy, false);
      lcd.setCursor(16,3); lcd.print(mom_pf / 100.0);
    }
  }
}

void printEnergy(float energy, bool meter_energy){
  if(meter_energy && energy < 100000) lcd.print(F(" "));
  if(energy < 10000) lcd.print(F(" ")); 
  if(energy < 1000) lcd.print(F(" ")); 
  if(energy < 100) lcd.print(F(" "));
  if(meter_energy) lcd.print(energy, 1);
  else lcd.print(energy / 10.0, 1);
}


void backgroundTime(){
  if (timeTimer.isReady()){
    getTime();
  }
}


void mainMenu(){
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(5, 0);  lcd.print(F("Main menu"));
    lcd.setCursor(1, 1);  lcd.print(F("Back       Network"));
    lcd.setCursor(1, 2);  lcd.print(F("Meter      Settings"));
    lcd.setCursor(1, 3);  lcd.print(F("Charts"));
    setMenuCursor(); lcd.print(F(">"));
    screenReadyFlag = true;
    }

  if(enc.right()) {
    setMenuCursor();
    lcd.print(F(" "));
    menu += 1;
    if (menu > 5) menu = 1;
    setMenuCursor();
    lcd.print(F(">"));
    menuExitTimer.start();
  }

  if(enc.left()) {
    setMenuCursor();
    lcd.print(F(" "));
    menu -= 1;
    if (menu < 1) menu = 5;
    setMenuCursor();
    lcd.print(F(">"));
    menuExitTimer.start();
  }
  
  if(enc.click()){
    if(menu == 1){
      mode = 0;
      screenReadyFlag = false;
      screen = 1;
    }

    if(menu == 2){
      enc.turn();
      mode = 2;
      screenReadyFlag = false;
      screen = 0;
    }

    if(menu == 4){
      mode = 4;
      screenReadyFlag = false;
      screen = 1;
    }

    if(menu == 5){
      mode = 5;
      screenReadyFlag = false;
      menu = 1;
    }
    enc.resetState();
    menuExitTimer.start();
  }
}


void setMenuCursor() {
  if (menu < 4) lcd.setCursor(0, menu);
  else lcd.setCursor(11, menu-3);
}


void settingsMenu(){
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(6, 0);  lcd.print(F("Settings"));
    lcd.setCursor(1, 1);  lcd.print(F("Back       Bright"));
    lcd.setCursor(1, 2);  lcd.print(F("Time&Date  Mode"));
    lcd.setCursor(1, 3);  lcd.print(F("Set meter  Reset"));
    setMenuCursor(); lcd.print(F(">"));
    screenReadyFlag = true;
    }

  if(enc.right()) {
    setMenuCursor();
    lcd.print(F(" "));
    menu += 1;
    if (menu > 6) menu = 1;
    setMenuCursor();
    lcd.print(F(">"));
    menuExitTimer.start();
  }

  if(enc.left()) {
    setMenuCursor();
    lcd.print(F(" "));
    menu -= 1;
    if (menu < 1) menu = 6;
    setMenuCursor();
    lcd.print(F(">"));
    menuExitTimer.start();
  }
  
  if(enc.click()){
    menuExitTimer.start();
    if(menu == 1){
      mode = 1;
      menu = 5;
      screenReadyFlag = false;
      screen = 1;
    }

    else if(menu == 2){
      mode = 6;
      menu = 0;
      screenReadyFlag = false;
    }

    else if(menu == 3){
      mode = 7;
      menu = 1;
      screenReadyFlag = false;
    }

    else if(menu == 4){
      mode = 8;
      menu = 1;
      screenReadyFlag = false;
    }

    else if(menu == 5){
      mode = 9;
      menu = 1;
      screenReadyFlag = false;
    }

    else if(menu == 6){
      mode = 10;
      menu = 1;
      screenReadyFlag = false;
    }
  }
}


void setTime(){
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(0, 0);  lcd.print(F("Autocorrect time"));
    lcd.setCursor(0, 1);  lcd.print(F("Time:")); lcd.setCursor(7, 1); lcd.print(F(":")); lcd.setCursor(10,1); lcd.print(F(":"));
    lcd.setCursor(0, 2);  lcd.print(F("Date:")); lcd.setCursor(7, 2); lcd.print(F("/")); lcd.setCursor(10,2); lcd.print(F("/"));
    lcd.setCursor(2, 3);  lcd.print(F("Back      Save"));
    
    new_hour = hour;
    new_minute = minute;
    new_second = second;
    new_day = day;
    new_month = month;
    new_year = year - 2000;
    screenReadyFlag = true;
    }

  if(printTimer.isReady()) {
    lcd.setCursor(17,0); if (automaticallyUpdateTime) {lcd.print(F("On "));} else {lcd.print(F("Off"));}
    lcd.setCursor(5,1); if (new_hour < 10){lcd.print(F("0"));} lcd.print(new_hour); 
    lcd.setCursor(8,1); if (new_minute < 10){lcd.print(F("0"));} lcd.print(new_minute); 
    lcd.setCursor(11,1); if (new_second < 10){lcd.print(F("0"));} lcd.print(new_second);
    lcd.setCursor(5,2); if (new_day < 10){lcd.print(F("0"));} lcd.print(new_day); 
    lcd.setCursor(8,2); if (new_month < 10){lcd.print(F("0"));} lcd.print(new_month); 
    lcd.setCursor(11,2); lcd.print(new_year);
    blinkFlag = !blinkFlag;
    if(blinkFlag) 
      timeSetMenu(menu);
  }
  
  if(enc.right()) {
    menu += 1;
    if (menu > 8) menu = 0;
    timeSetMenu(menu);
    menuExitTimer.start();
  }

  if(enc.left()) {
    menu -= 1;
    if (menu < 0) menu = 8;
    timeSetMenu(menu);
    menuExitTimer.start();
  }

  if(enc.rightH()) {
    adjustTimeDate(1, menu);
    timeSetMenu(menu);
    menuExitTimer.start();
  }

  if(enc.leftH()) {
    adjustTimeDate(-1, menu);
    timeSetMenu(menu);
    menuExitTimer.start();
  }
  
  if(enc.click()){
    menuExitTimer.start();
    if(menu == 7){
      EEPROM.get(19, automaticallyUpdateTime);
      mode = 5;
      menu = 2;
      screenReadyFlag = false;
      screen = 1;
      enc.resetState();
    }

    if(menu == 8){
      if (EEPROM.read(19) != automaticallyUpdateTime) {
        EEPROM.put(19, automaticallyUpdateTime);
        EEPROM.commit();
        if (DEBUG_MODE) {
          Serial.print(F("Saving new value for automaticallyUpdateTime flag=")); Serial.println(automaticallyUpdateTime);
        }
      }
      else {
        saveNewTime(true);
        if (DEBUG_MODE) {
          Serial.print(F("Saving new time values"));
        }
      }
      mode = 5;
      menu = 2;
      screen = 1;
      screenReadyFlag = false;
    }
  }
}


void timeSetMenu(byte menu){
  lcd.setCursor(1, 3);  lcd.print(F(" "));
  lcd.setCursor(11, 3); lcd.print(F(" "));
  
  switch(menu){
    case 0:
      lcd.setCursor(17,0);
      lcd.print(F("   "));
      break;
    
    case 1:
      lcd.setCursor(5,1);
      lcd.print(F("  "));
      break;

    case 2:
      lcd.setCursor(8,1);
      lcd.print(F("  "));
      break;

    case 3:
      lcd.setCursor(11,1);
      lcd.print(F("  "));
      break;

    case 4:
      lcd.setCursor(5,2);
      lcd.print(F("  "));
      break;

    case 5:
      lcd.setCursor(8,2);
      lcd.print(F("  "));
      break;

    case 6:
      lcd.setCursor(11,2);
      lcd.print(F("  "));
      break;

    case 7:
      lcd.setCursor(1, 3);
      lcd.print(F(">"));
      break;

    case 8:
      lcd.setCursor(11, 3);
      lcd.print(F(">"));
      break;
  }
}


void setMeter(){
  byte s;
  if (DEMO_MODE) s = 100; 
  else s = 0;
  
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(2, 0);  lcd.print(F("Adjust meter"));
    lcd.setCursor(0, 1);  lcd.print(F("Day:")); lcd.setCursor(12, 1); lcd.print(F("kWh"));
    lcd.setCursor(0, 2);  lcd.print(F("Ngt:")); lcd.setCursor(12, 2); lcd.print(F("kWh"));
    lcd.setCursor(2, 3);  lcd.print(F("Back      Save"));
    getMeterData(s);
    screenReadyFlag = true;
    }

  if(printTimer.isReady()) {
    lcd.setCursor(4,1); printEnergy(day_energy, true);
    lcd.setCursor(4,2); printEnergy(night_energy, true);
    meterSetMenu(menu);
    blinkFlag = !blinkFlag;
  }
  
  if(enc.right()) {
    menu += 1;
    if (menu > 4) menu = 1;
    meterSetMenu(menu);
    menuExitTimer.start();
  }

  if(enc.left()) {
    menu -= 1;
    if (menu < 1) menu = 4;
    meterSetMenu(menu);
    menuExitTimer.start();
  }

  if(enc.rightH()) {
    if (enc.fast()) adjustTimeDate(5, menu + 10);
    else adjustTimeDate(0.1, menu + 10);
    meterSetMenu(menu);
  }

  if(enc.leftH()) {
    if (enc.fast()) adjustTimeDate(-5, menu + 10);
    else adjustTimeDate(-0.1, menu + 10);
    meterSetMenu(menu);
  }
  
  if(enc.click()){
    menuExitTimer.start();
    if(menu == 3){
      mode = 5;
      menu = 3;
      screen = 1;
      screenReadyFlag = false;
    }

    if(menu == 4){
      EEPROM.put(4+s, day_energy);
      EEPROM.put(8+s, night_energy);
      total_energy = day_energy + night_energy;
      EEPROM.put(12+s, total_energy);
      EEPROM.commit();
      mode = 5;
      menu = 3;
      screen = 1;
      screenReadyFlag = false;
    }
    enc.resetState();
  }
}


void meterSetMenu(byte menu){
  lcd.setCursor(1, 3);  lcd.print(F(" "));
  lcd.setCursor(11, 3); lcd.print(F(" "));
  lcd.setCursor(15, 1); lcd.print(F(" "));
  lcd.setCursor(15, 2); lcd.print(F(" "));
  
  switch(menu){
    case 1:
      lcd.setCursor(15,1);
      if(blinkFlag) lcd.print(F("<")); 
      break;

    case 2:
      lcd.setCursor(15,2);
      if(blinkFlag) lcd.print(F("<")); 
      break;

    case 3:
      lcd.setCursor(1, 3);
      lcd.print(F(">"));
      break;

    case 4:
      lcd.setCursor(11, 3);
      lcd.print(F(">"));
      break;
  }
}


void setBright(){
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(1, 0);  lcd.print(F("Adjust brightness"));
    lcd.setCursor(0, 1);  lcd.print(F("Brightness:")); 
    lcd.setCursor(2, 3);  lcd.print(F("Back      Save"));
    getBrightness();
    screenReadyFlag = true;
    }

  if(printTimer.isReady()) {
    lcd.setCursor(12,1);  
    lcd.print(lcd_bright);
    brightSetMenu(menu);
    blinkFlag = !blinkFlag;
  }
  
  if(enc.right()) {
    menu += 1;
    if (menu > 3) menu = 1;
    menuExitTimer.start();
    brightSetMenu(menu);
  }

  if(enc.left()) {
    menu -= 1;
    if (menu < 1) menu = 3;
    menuExitTimer.start();
    brightSetMenu(menu);
  }

  if(enc.rightH()) {
    adjustTimeDate(1, 21);
    brightSetMenu(menu);
  }

  if(enc.leftH()) {
    adjustTimeDate(-1, 21);
    brightSetMenu(menu);
  }
  
  if(enc.click()){
    menuExitTimer.start();
    if(menu == 2){
      getBrightness();
      mode = 5;
      menu = 4;
      screen = 1;
      screenReadyFlag = false;
    }

    if(menu == 3){
      EEPROM.put(16, lcd_bright);
      EEPROM.commit();
      analogWrite(BACKLIGHT, lcd_bright*15);
      mode = 5;
      menu = 4;
      screen = 1;
      screenReadyFlag = false;
    }
  }
}


void brightSetMenu(byte menu){
  lcd.setCursor(1, 3);  lcd.print(F(" "));
  lcd.setCursor(11, 3); lcd.print(F(" "));
  
  switch(menu){
    case 1:
      lcd.setCursor(12,1); 
      if(blinkFlag) lcd.print(F(" "));
      break;

    case 2:
      lcd.setCursor(1, 3);
      lcd.print(F(">"));
      break;

    case 3:
      lcd.setCursor(11, 3);
      lcd.print(F(">"));
      break;
  }
}


void setDemoMode(){
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(0, 0);  lcd.print(F("Demo mode:"));
    lcd.setCursor(0, 1);  lcd.print(F("WiFi enabled:"));
    lcd.setCursor(0, 2);  lcd.print(F("Telegram bot:"));
    lcd.setCursor(2, 3);  lcd.print(F("Back      Save"));
    newDemoMode = DEMO_MODE;
    screenReadyFlag = true;
    }

  if(printTimer.isReady()) {
    lcd.setCursor(14,0);  
    if (newDemoMode) lcd.print(F("On "));
    else lcd.print(F("Off"));

    lcd.setCursor(14,1);
    if (enableWiFi) lcd.print(F("On "));
    else lcd.print(F("Off"));

    lcd.setCursor(14,2);
    if (telegramEnabled) lcd.print(F("On "));
    else lcd.print(F("Off"));
    modeSetMenu(menu);
    blinkFlag = !blinkFlag;
  }
  
  if(enc.right()) {
    menu += 1;
    if (menu > 5) menu = 1;
    menuExitTimer.start();
    modeSetMenu(menu);
  }

  if(enc.left()) {
    menu -= 1;
    if (menu < 1) menu = 5;
    menuExitTimer.start();
    modeSetMenu(menu);
  }

  if(enc.rightH() || enc.leftH()){
    menuExitTimer.start();
    if(menu == 1) {
      newDemoMode = !newDemoMode;
    }

    if(menu == 2) {
      enableWiFi = !enableWiFi;
    }

    if(menu == 3) {
      telegramEnabled = !telegramEnabled;
    }
    modeSetMenu(menu);
  }
    
  if(enc.click()){  
    if(menu == 4){
      EEPROM.get(42, enableWiFi);
      EEPROM.get(17, telegramEnabled);
      mode = 5;
      menu = 5;
      screen = 1;
      screenReadyFlag = false;
    }

    if(menu == 5){
      bool commitNeeded = false;
      
      if (DEMO_MODE != newDemoMode) {
        DEMO_MODE = newDemoMode;
        EEPROM.put(18, DEMO_MODE);
        commitNeeded = true;
      }

      if (EEPROM.read(42) != enableWiFi) {
        EEPROM.put(42, enableWiFi);
        commitNeeded = true;
        if (enableWiFi) {
          WiFi.begin(ssid, password);
        }
        else {
          WiFi.disconnect();
        }
      }

      if (EEPROM.read(17) != telegramEnabled) {
        EEPROM.put(17, telegramEnabled);
        commitNeeded = true;
      }
      
      if (commitNeeded) {
        EEPROM.commit();
      }
      mode = 5;
      menu = 5;
      screen = 1;
      screenReadyFlag = false;
    }
  }
}


void modeSetMenu(byte menu){
  lcd.setCursor(1, 3);  lcd.print(F(" "));
  lcd.setCursor(11, 3); lcd.print(F(" "));
  
  switch(menu){
    case 1:
      lcd.setCursor(14,0); 
      if(blinkFlag) lcd.print(F("   "));
      break;

    case 2:
      lcd.setCursor(14,1); 
      if(blinkFlag) lcd.print(F("   "));
      break;

    case 3:
      lcd.setCursor(14,2); 
      if(blinkFlag) lcd.print(F("   "));
      break;

    case 4:
      lcd.setCursor(1, 3);
      lcd.print(F(">"));
      break;

    case 5:
      lcd.setCursor(11, 3);
      lcd.print(F(">"));
      break;
  }
}


void resetMeter(){
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(4, 0);  lcd.print(F("Reset device"));
    lcd.setCursor(1, 1);  lcd.print(F("Back"));
    lcd.setCursor(1, 2);  lcd.print(F("Reset PZEM energy")); 
    lcd.setCursor(1, 3);  lcd.print(F("Factory reset"));
    setMenuCursor(); lcd.print(F(">"));
    screenReadyFlag = true;
    }

  if(enc.right()) {
    setMenuCursor();
    lcd.print(F(" "));
    menu += 1;
    if (menu > 3) menu = 1;
    setMenuCursor();
    lcd.print(F(">"));
    menuExitTimer.start();
  }

  if(enc.left()) {
    setMenuCursor();
    lcd.print(F(" "));
    menu -= 1;
    if (menu < 1) menu = 3;
    setMenuCursor();
    lcd.print(F(">"));
    menuExitTimer.start();
  }

  if(enc.click()){
    menuExitTimer.start();
    if(menu == 1) {
      mode = 5;
      menu = 6;
      screen = 1;
      screenReadyFlag = false;
    }
    
    if(menu == 2){
      mode = 11;
      menu = 1;
      screen = 1;
      screenReadyFlag = false;
    }

    if(menu == 3){
      mode = 11;
      menu = 1;
      screen = 2;
      screenReadyFlag = false;
    }
  }
}


void performReset(){
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(1, 0);
    if (screen == 1) lcd.print(F("Reset PZEM energy"));
    else lcd.print(F("Factory reset"));
    lcd.setCursor(3, 1);  lcd.print(F("Are you sure?")); 
    lcd.setCursor(2, 3);  lcd.print(F("Back      Reset"));
    screenReadyFlag = true;
    performResetMenu(menu);
    }

  if(enc.right()) {
    menu += 1;
    if (menu > 2) menu = 1;
    menuExitTimer.start();
    performResetMenu(menu);
  }

  if(enc.left()) {
    menu -= 1;
    if (menu < 1) menu = 2;
    menuExitTimer.start();
    performResetMenu(menu);
  }

  if(enc.click()){
    menuExitTimer.start();  
    if(menu == 1){
      mode = 10;
      menu = screen + 1;
      screen = 1;
      screenReadyFlag = false;
    }

    else if(menu == 2){
      if (screen == 1 && DEMO_MODE){
        mode = 5;
        menu = 6;
        screen = 1;
        screenReadyFlag = false;
        }
      else if (screen == 1 && meterPowered && !DEMO_MODE) {
        pzem.resetEnergy();
        mode = 10;
        menu = 2;
        screen = 1;
        screenReadyFlag = false;
        }
      else if (screen == 2) {
        EEPROM.write(INIT_ADDR, 1);
        EEPROM.commit();
        resetFunc();
      }
    }
  }
}


void performResetMenu(byte menu){
  lcd.setCursor(1, 3);  lcd.print(F(" "));
  lcd.setCursor(11, 3); lcd.print(F(" "));
  
  switch(menu){
    case 1:
      lcd.setCursor(1, 3);
      lcd.print(F(">"));
      break;

    case 2:
      lcd.setCursor(11, 3);
      lcd.print(F(">"));
      break;
  }
}


void adjustTimeDate(float delta, byte menu){
  switch(menu){
    case 0:
      automaticallyUpdateTime = !automaticallyUpdateTime;
      break;
    
    case 1:
      new_hour += delta;
      if(new_hour > 23) new_hour = 0;
      if(new_hour < 0) new_hour = 23;
      break;

    case 2:
      new_minute += delta;
      if(new_minute > 59) new_minute = 0;
      if(new_minute < 0) new_minute = 59;
      break;

    case 3:
      new_second += delta;
      if(new_second > 59) new_second = 0;
      if(new_second < 0) new_second = 59;
      break;

    case 4:
      new_day += delta;
      if(new_day > 31) new_day = 0;
      if(new_day < 0) new_day = 31;
      break;

    case 5:
      new_month += delta;
      if(new_month > 31) new_month = 0;
      if(new_month < 0) new_month = 31;
      break;

    case 6:
      new_year += delta;
      if(new_year > 99) new_year = 20;
      if(new_year < 20) new_year = 99;
      break;

    case 11:
      day_energy += delta;
      if(day_energy < 0) day_energy = 0;
      break;

    case 12:
      night_energy += delta;
      if(night_energy < 0) night_energy = 0;
      break;

    case 21:
      lcd_bright += delta;
      lcd_bright = constrain(lcd_bright, 1, 9);
      analogWrite(BACKLIGHT, lcd_bright*15);
      break;
  }
}
