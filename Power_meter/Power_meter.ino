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


// Settings
#define DEBUG_MODE 1
#define DEMO_MODE 0


// Timers:
GTimer measureTimer(MS, MEASURE_TIMEOUT);


// Setting up modules
SoftwareSerial pzemSWSerial(PZEM_TX_PIN, PZEM_RX_PIN);
PZEM004Tv30 pzem;
LiquidCrystal_I2C lcd(0x27, 20, 4);


// Global variables
int mom_voltage = 0;
int mom_current = 0;
int mom_power = 0;
int mom_energy = 0;
int mom_frequency = 0;
int mom_pf = 0;


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
  delay(2500);
  lcd.clear();

  //pzem.resetEnergy();
}


void loop() {
  measurePower();
}


void measurePower(){  
  if (measureTimer.isReady()){
    if (!DEMO_MODE){
      if (pzem.readAddress() == 0){
        if (DEBUG_MODE) {
          Serial.println("The meter is not powered!");
        }
        lcd.clear();
        lcd.setCursor(5, 0);
        lcd.print(F("The meter"));
        lcd.setCursor(3, 1);
        lcd.print(F("is not powered"));
        
        mom_voltage = 0;
        mom_current = 0;
        mom_power = 0;
        mom_frequency = 0;
        mom_pf = 0;
      }
      else {
        readPowerData();
        printPowerData();
      }
    }
  }
}


void readPowerData() {
    mom_voltage = pzem.voltage() * 10;
    mom_current = round(pzem.current() * 10);
    mom_power = round(pzem.power());
    mom_energy = round(pzem.energy());
    mom_frequency = pzem.frequency() * 100;
    mom_pf = pzem.pf() * 100;
    if (isnan(mom_voltage)) {
      mom_voltage = 0;
    }
}


void printPowerData() {
  if (DEBUG_MODE) {  
    Serial.print("Voltage: ");      Serial.print(mom_voltage / 10.0);      Serial.println("V");
    Serial.print("Current: ");      Serial.print(mom_current / 10.0);      Serial.println("A");
    Serial.print("Power: ");        Serial.print(mom_power);        Serial.println("W");
    Serial.print("Energy: ");       Serial.print(mom_energy);       Serial.println("kWh");
    Serial.print("Frequency: ");    Serial.print(mom_frequency / 100.0);    Serial.println("Hz");
    Serial.print("PF: ");           Serial.println(mom_pf / 100.0);
    Serial.println("");
  }

  lcd.clear();
  if (DEMO_MODE) {
    lcd.setCursor(5, 0); lcd.print(F("Demo mode:"));
  }
  else {
    lcd.setCursor(3, 0); lcd.print(F("Current status:"));
  }
  lcd.setCursor(0, 1); lcd.print(F("U=")); lcd.setCursor(2,1); lcd.print(mom_voltage / 10.0, 1); lcd.setCursor(7,1); lcd.print(F("V"));
  lcd.setCursor(12, 1); lcd.print(F("I=")); lcd.setCursor(14,1); lcd.print(mom_current / 10.0, 1); lcd.setCursor(19,1); lcd.print(F("A"));
  lcd.setCursor(0, 2); lcd.print(F("P=")); lcd.setCursor(2,2); lcd.print(mom_power); lcd.setCursor(7,2); lcd.print(F("W"));
  lcd.setCursor(12, 2); lcd.print(F("F=")); lcd.setCursor(14,2); lcd.print(mom_frequency / 100.0); lcd.setCursor(18,2); lcd.print(F("Hz"));
  lcd.setCursor(0, 3); lcd.print(F("E=")); lcd.setCursor(2,3); lcd.print(mom_energy); lcd.setCursor(7,3); lcd.print(F("kWh"));
  lcd.setCursor(13, 3); lcd.print(F("PF=")); lcd.setCursor(16,3); lcd.print(mom_pf / 100.0);
  
}
