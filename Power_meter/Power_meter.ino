/* Power meter by R. Gurevych */

// Includes
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <GyverTimer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <EncButton.h>
#include <EEPROM.h>


// Pins
#define PZEM_RX_PIN 12
#define PZEM_TX_PIN 13
#define CLKe A0        // encoder S1
#define DTe A1         // encoder S2
#define SWe A2        // encoder Key


// Timer durations
#define MEASURE_TIMEOUT 1500             //Interval between measure occurs
#define PRINT_TIMEOUT 500               //Interval between printing to the screen occurs


// Settings
#define INIT_ADDR 1023  // номер резервной ячейки
#define INIT_KEY 0     // ключ первого запуска. 0-254, на выбор
#define DEBUG_MODE 0
#define RESET_CLOCK 0
bool DEMO_MODE = true;


// Timers:
GTimer measureTimer(MS, MEASURE_TIMEOUT);
GTimer printTimer(MS, PRINT_TIMEOUT);


// Setting up modules
SoftwareSerial pzemSWSerial(PZEM_TX_PIN, PZEM_RX_PIN);
PZEM004Tv30 pzem;
LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS3231 rtc;
DateTime now;
EncButton<EB_TICK, CLKe, DTe, SWe> enc;


// Global variables
uint8_t hour, min, second, month, day;
uint16_t year;
uint16_t mom_voltage = 0;
uint16_t mom_current = 0;
uint16_t mom_power = 0;
uint32_t mom_energy = 0;
uint16_t mom_frequency = 0;
uint16_t mom_pf = 0;
byte mode = 0;
byte screen = 0;
byte menu = 1;
uint32_t latest_energy = 0;
float day_energy = 0.1;
float night_energy = 0.1;
float total_energy = 0.1;
byte lcd_bright = 50;


//Flags
bool screenReadyFlag = false;
bool lcdBacklight = true;


void setup() {
  if (DEBUG_MODE) {
    Serial.begin(115200);
  }

  // Reset to default settings
  if (EEPROM.read(INIT_ADDR) != INIT_KEY) {
    EEPROM.write(INIT_ADDR, INIT_KEY);
    EEPROM.put(0, latest_energy);
    EEPROM.put(4, day_energy);
    EEPROM.put(8, night_energy);
    EEPROM.put(12, total_energy);
    EEPROM.put(16, lcd_bright);
  }

  // RTC module
  rtc.begin();
  if (RESET_CLOCK || rtc.lostPower()){
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  now = rtc.now();
  
  pzem = PZEM004Tv30(pzemSWSerial);
  
  // analogWrite(BACKLIGHT, LCD_BRIGHTNESS);
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
      lcd.setCursor(5, 0);
      lcd.print(F("The meter"));
      lcd.setCursor(3, 1);
      lcd.print(F("is not powered"));
      lcd.setCursor(2, 3);
      lcd.print(F("Check connection"));
    }
    else {
      lcd.setCursor(3, 1);
      lcd.print(F("Initialization"));
      lcd.setCursor(6, 2);
      lcd.print(F("completed"));
    }
  }
  delay(2000);

  //pzem.resetEnergy();
}


void loop() {
  enc.tick();
  getPowerData();
  checkMode();

}


void getPowerData(){
  if (measureTimer.isReady()){
    if (DEMO_MODE){
      generatePowerData();
    }
    else {
      measurePower();
    }
  }
}


void measurePower(){  
  if (pzem.readAddress() == 0){
    if (DEBUG_MODE) {
      Serial.println(F("The meter is not powered!"));
    }
    if (mode == 0 && screen == 1) {
      screen = 0;
      screenReadyFlag = false;
    }    
    mom_voltage = 0;
    mom_current = 0;
    mom_power = 0;
    mom_frequency = 0;
    mom_pf = 0;
  }
  else {
    readPowerData();
    if (mode == 0 && screen == 0) {
      screen = 1;
      screenReadyFlag = false;
    }
  }
}


void readPowerData() {
  mom_voltage = pzem.voltage() * 10;
  mom_current = round(pzem.current() * 100);
  mom_power = round(pzem.power());
  mom_energy = round(pzem.energy() * 10);
  mom_frequency = pzem.frequency() * 10;
  mom_pf = pzem.pf() * 100;
  if (isnan(mom_voltage)) {
    mom_voltage = 0;
  }
}


void generatePowerData(){
  mom_voltage = 2100 + random(200);
  mom_current = random(1500);
  mom_power = round((mom_voltage / 10.0) * (mom_current / 100.0));
  mom_energy += mom_power / 1800;
  mom_frequency = random(498, 502);
  mom_pf = random(90, 101);
}


void checkMode(){
  if (mode == 0) {
    printPowerData();
  }
  if (mode == 1) {
    showMenu();
  }
  if (mode == 2) {
    showMeter();
  }
}

void printPowerData() {
//  if(enc.isRight() || enc.isLeft()){
//    checkBacklight(true);
//  }

  if(enc.click()){
//    checkBacklight(true);
    mode = 1;
    screenReadyFlag = false;
    screen = 0;
    menu = 1;
    enc.resetState();
  }
  
  if (printTimer.isReady()){

    now = rtc.now();
    second = now.second();
    min = now.minute();
    hour = now.hour();
    day = now.day();
    month = now.month();
    year = now.year();
    
    if (DEBUG_MODE) {
      Serial.print(hour); Serial.print(F(":")); Serial.print(min); Serial.print(F(":")); Serial.print(second);
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
      lcd.setCursor(3,0); if (min < 10){lcd.print(F("0"));} lcd.print(min); 
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


//void checkBacklight(bool backlightState){
//  if(lcdBacklight != backlightState){
//    lcdBacklight = backlightState;
//  }
//  lcd.setBacklight(lcdBacklight);
//}


void showMenu(){
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    lcd.setCursor(5, 0);  lcd.print(F("Main menu"));
    lcd.setCursor(1, 1);  lcd.print(F("Back"));
    lcd.setCursor(1, 2);  lcd.print(F("Meter"));
    lcd.setCursor(1, 3);  lcd.print(F("Charts"));
    lcd.setCursor(11,1);  lcd.print(F("Min/max"));
    lcd.setCursor(11,2);  lcd.print(F("Settings"));
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
  }

  if(enc.left()) {
    setMenuCursor();
    lcd.print(F(" "));
    menu -= 1;
    if (menu < 1) menu = 5;
    setMenuCursor();
    lcd.print(F(">"));
  }
  
  if(enc.click()){
    if(menu == 1){
      mode = 0;
      screenReadyFlag = false;
      screen = 1;
    }

    if(menu == 2){
      mode = 2;
      screenReadyFlag = false;

    }
    enc.resetState();
  }
}


void setMenuCursor() {
  if (menu < 4) lcd.setCursor(0, menu);
  else lcd.setCursor(10, menu-3);
}


void showMeter(){
  if (!screenReadyFlag) {
    lcd.clear();
    }
    
  if (!screenReadyFlag){
    EEPROM.get(4, day_energy);
    EEPROM.get(8, night_energy);
    EEPROM.get(12, total_energy);
    
    lcd.setCursor(3, 0);  lcd.print(F("Energy meter"));
    lcd.setCursor(0, 1);  lcd.print(F("Day:"));  printEnergy(day_energy, true);  lcd.print(F("kWh"));
    lcd.setCursor(0, 2);  lcd.print(F("Ngt:"));  printEnergy(night_energy, true);  lcd.print(F("kWh"));
    lcd.setCursor(0, 3);  lcd.print(F("Tot:"));  printEnergy(total_energy, true);  lcd.print(F("kWh"));
    screenReadyFlag = true;
    }
  
  if(enc.click()){
    mode = 1;
    screenReadyFlag = false;

    enc.resetState();
  }
}