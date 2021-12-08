/* Power meter by R. Gurevych */

// Includes
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <GyverTimer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


// Pins
#define PZEM_RX_PIN 12
#define PZEM_TX_PIN 13


// Timer durations
#define MEASURE_TIMEOUT 2000             //Interval between measure occurs
#define PRINT_TIMEOUT 1000               //Interval between printing to the screen occurs


// Settings
bool DEBUG_MODE = true;
bool DEMO_MODE = true;


// Timers:
GTimer measureTimer(MS, MEASURE_TIMEOUT);
GTimer printTimer(MS, PRINT_TIMEOUT);


// Setting up modules
SoftwareSerial pzemSWSerial(PZEM_TX_PIN, PZEM_RX_PIN);
PZEM004Tv30 pzem;
LiquidCrystal_I2C lcd(0x27, 20, 4);


// Global variables
int mom_voltage = 0;
int mom_current = 0;
int mom_power = 0;
long mom_energy = 0;
int mom_frequency = 0;
int mom_pf = 0;
byte mode = 1;

//Flags
bool screenReadyFlag = false;


void setup() {
  if (DEBUG_MODE) {
    Serial.begin(115200);
  }
  
  pzem = PZEM004Tv30(pzemSWSerial);
  // analogWrite(BACKLIGHT, LCD_BRIGHTNESS);
  lcd.init();
  lcd.backlight();
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
  getPowerData();
}


void getPowerData(){
  if (measureTimer.isReady()){
    if (DEMO_MODE){
      generatePowerData();
    }
    else {
      measurePower();
    }
  printPowerData();
  }
}


void measurePower(){  
  if (pzem.readAddress() == 0){
    if (DEBUG_MODE) {
      Serial.println(F("The meter is not powered!"));
    }
    if (mode == 1) {
      mode = 0;
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
    if (mode == 0) {
      mode = 1;
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
  mom_energy += mom_power / 360;
  mom_frequency = random(498, 502);
  mom_pf = random(90, 101);
  
}

void printPowerData() {
  if (DEBUG_MODE) {  
    Serial.print(F("U: "));      Serial.print(mom_voltage / 10.0);      Serial.println(F("V"));
    Serial.print(F("I: "));      Serial.print(mom_current / 100.0);     Serial.println(F("A"));
    Serial.print(F("P: "));      Serial.print(mom_power);               Serial.println(F("W"));
    Serial.print(F("E: "));      Serial.print(mom_energy / 10.0);       Serial.println(F("kWh"));
    Serial.print(F("F: "));      Serial.print(mom_frequency / 100.0);   Serial.println(F("Hz"));
    Serial.print(F("PF: "));     Serial.println(mom_pf / 100.0);        Serial.println();
  }

  if (!screenReadyFlag) {
    lcd.clear();
  }
  
  if (mode == 0) {
    if (!screenReadyFlag){
      lcd.setCursor(5, 0);
      lcd.print(F("The meter"));
      lcd.setCursor(3, 1);
      lcd.print(F("is not powered"));
      screenReadyFlag = true;
    }
  }

  if (mode == 1) {
    if (!screenReadyFlag){
      if (DEMO_MODE) {
        lcd.setCursor(5, 0); lcd.print(F("Demo mode:"));
      }
      else {
        lcd.setCursor(3, 0); lcd.print(F("Current status:"));
      }
      lcd.setCursor(0, 1); lcd.print(F("U=")); lcd.setCursor(7,1); lcd.print(F("V"));
      lcd.setCursor(12, 1); lcd.print(F("I=")); lcd.setCursor(19,1); lcd.print(F("A"));
      lcd.setCursor(0, 2); lcd.print(F("P=")); lcd.setCursor(7,2); lcd.print(F("W"));
      lcd.setCursor(12, 2); lcd.print(F("F=")); lcd.setCursor(18,2); lcd.print(F("Hz"));
      lcd.setCursor(0, 3); lcd.print(F("E=")); lcd.setCursor(8,3); lcd.print(F("kWh"));
      lcd.setCursor(13, 3); lcd.print(F("PF=")); 
      screenReadyFlag = true;
    }

    lcd.setCursor(2,1); lcd.print(mom_voltage / 10.0, 1);
    lcd.setCursor(14,1); if(mom_current < 1000){lcd.print(F(" "));} lcd.print(mom_current / 100.0, 2);
    lcd.setCursor(2,2); if(mom_power < 10000){lcd.print(F(" "));} if(mom_power < 1000){lcd.print(F(" "));} 
    if(mom_power < 100){lcd.print(F(" "));} if(mom_power < 10){lcd.print(F(" "));} lcd.print(mom_power);
    lcd.setCursor(14,2); lcd.print(mom_frequency / 10.0, 1);
    lcd.setCursor(2,3); if(mom_energy < 10000){lcd.print(F(" "));} if(mom_energy < 1000){lcd.print(F(" "));} 
    if(mom_energy < 100){lcd.print(F(" "));} lcd.print(mom_energy / 10.0, 1);
    lcd.setCursor(16,3); lcd.print(mom_pf / 100.0);
  }
}
