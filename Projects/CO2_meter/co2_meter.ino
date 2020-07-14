#include <GyverTimer.h>         //Library for working with timer
#include <SoftwareSerial.h>     //Library for working with serial port
#include <DHT.h>                //Library for working with DHT sensor
#include <MHZ19_uart.h>         //Library for working with MH-19Z sensor
#include <GyverButton.h>        //Library for working with button
#include <Wire.h>               //Library for working with LED display
#include <LiquidCrystal_I2C.h>  //Library for working with LED display
#include <RTClib.h>             //Library for working with RTC clock module


// Pins:
#define DHTPIN 4            //DHT-22 signal pin
#define RX_PIN 2            //Serial rx pin no
#define TX_PIN 3            //Serial tx pin no
#define BUTTON1_PIN 7       //Button 1 pin
#define BUTTON2_PIN 8       //Button 1 pin
#define RED_LED_PIN 11      //Red LED pin
#define YELLOW_LED_PIN 10   //Yellow LED pin
#define GREEN_LED_PIN 9     //Green LED pin
#define BACKLIGHT 5         //LCD backlight pin


// Timer durations
#define MEASURE_TIMEOUT 15000             //Interval between measure occurs
#define PRINT_TIMEOUT 30000               //Interval between serial printout occurs
#define WARMING_UP_TIMEOUT 175000         //Duration of warming up period
#define BLINK_TIMEOUT 750                //LED blinking interval
#define RESET_MODE_TIMEOUT 10000          //Timeout before the mode is reset to default screen


// Settings
#define RESET_CLOCK 0                     //Should the clock be reset on start
#define DEBUG 1                           //Debug mode, in which the data is printed to serial port


// Timers:
GTimer measureTimer(MS, MEASURE_TIMEOUT);
GTimer printTimer(MS, PRINT_TIMEOUT);
GTimer fiveMinTimer(MS, 300000);
GTimer hourlyTimer(MS, 3600000);
GTimer warmingTimer(MS);
GTimer blinkTimer(MS, BLINK_TIMEOUT);
GTimer clockTimer(MS, 1000);
GTimer resetModeTimer(MS);


// Buttons
GButton button1(BUTTON1_PIN);
GButton button2(BUTTON2_PIN);


// CO2 sensor:
MHZ19_uart mhz19;


// DHT Sensor:
DHT dht(DHTPIN, DHT22);


//Liquid Crystal LCD:
LiquidCrystal_I2C lcd(0x27, 16, 2);


//Real Time Clock:
RTC_DS3231 rtc;
DateTime now;


// Initial variables:
int8_t hrs, mins, secs;
int current_ppm = 0;
int max_5min_ppm = 0;
int min_5min_ppm = 5000;
int max_hour_ppm = 0;
int min_hour_ppm = 5000;

int current_temp = 0;
int max_5min_temp = 0;
int min_5min_temp = 1000;
int max_hour_temp = 0;
int min_hour_temp = 1000;

int current_hum = 0;
int max_5min_hum = 0;
int min_5min_hum = 1000;
int max_hour_hum = 0;
int min_hour_hum = 1000;

int yellow_ppm_level = 850;
int red_ppm_level = 1200;
int alarm_ppm_level = 1600;

int minHourPpmArray[12], maxHourPpmArray[12];
int minDayPpmArray[24], maxDayPpmArray[24];
int minHourTempArray[12], maxHourTempArray[12];
int minDayTempArray[24], maxDayTempArray[24];
int minHourHumArray[12], maxHourHumArray[12];
int minDayHumArray[24], maxDayHumArray[24];

boolean ledState = HIGH;

byte LCD_BRIGHTNESS = 5;
byte LED_BRIGHTNESS = 100;
byte mode = 0;

// Flags
boolean warmedUpFlag = false;
boolean lcdBacklight = true;
boolean changeModeFlag = false;
boolean setTimeFlag = false;
boolean setHours = true;
boolean setMins = false;
boolean setSecs = false;


void setup() {
  // Serial
  if (DEBUG){
  Serial.begin(9600);
  }

  //Pin modes
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BACKLIGHT, OUTPUT);

  // RTC module
  rtc.begin();
  if (RESET_CLOCK || rtc.lostPower()){
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();

  // Start warming up timer
  warmingTimer.setTimeout(WARMING_UP_TIMEOUT);

  //LCD
  analogWrite(BACKLIGHT, LCD_BRIGHTNESS);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // MH-Z19
  mhz19.begin(RX_PIN, TX_PIN);
  mhz19.setAutoCalibration(false);  //  uncomment this to disable autocalibration
  mhz19.getStatus();    // first request, should return -1
  delay(500);
  lcd.setCursor(0, 0);
  if (mhz19.getStatus() == 0) {
    lcd.print(F("MH-Z19 OK"));
    if (DEBUG) {
      Serial.println(F("MH-Z19 OK"));
      }
  } 
  else {
    lcd.print(F("MH-Z19 ERROR"));
    if (DEBUG) {
    Serial.println(F("MH-Z19 ERROR"));
    }
  }
  
  // DHT
  dht.begin();
  lcd.setCursor(0, 1);
  if (isnan(measureTemp()) || isnan(measureHum())) {
    lcd.print(F("DHT ERROR"));
    if (DEBUG) {
    Serial.println(F("DHT ERROR"));
    }
  }
  else {
    lcd.print(F("DHT OK"));
    if (DEBUG) {
    Serial.println(F("DHT OK"));
    }
  }

  // Check all LEDs
  lightRedLed();
  delay(1000);
  lightYellowLed();
  delay(1000);
  lightGreenLed();
  delay(1000);
  lcd.clear();
}


void loop(){

  button1.tick();
  button2.tick();
  checkButtons();
  measure();
  printCurrentValues();
  displayScreen();
  updateData();
  switchLed(current_ppm);
  
}


// Get the sensors data
int measureCO2(){
  int ppm = mhz19.getPPM();
  return ppm;
}


int measureTemp(){
  int temp = dht.readTemperature() * 10;
  return temp;
}


int measureHum(){
  int hum = dht.readHumidity() * 10;
  return hum;
}


void measure(){  
  if (measureTimer.isReady()){
    current_ppm = measureCO2();
    current_temp = measureTemp();
    current_hum = measureHum();
  }
}


// Data update for saving hourly and daily values
void updateData(){
  if (warmedUpFlag){
    if (current_ppm > max_5min_ppm) max_5min_ppm = current_ppm;
    if (current_ppm < min_5min_ppm) min_5min_ppm = current_ppm;
    
    if (current_temp > max_5min_temp) max_5min_temp = current_temp;
    if (current_temp < min_5min_temp) min_5min_temp = current_temp;

    if (current_hum > max_5min_hum) max_5min_hum = current_hum;
    if (current_hum < min_5min_hum) min_5min_hum = current_hum;
  }
    
  if (fiveMinTimer.isReady()){
    for (byte i = 0; i < 11; i++) {
      minHourPpmArray[i] = minHourPpmArray[i + 1];
      maxHourPpmArray[i] = maxHourPpmArray[i + 1];
      minHourTempArray[i] = minHourTempArray[i + 1];
      maxHourTempArray[i] = maxHourTempArray[i + 1];
      minHourHumArray[i] = minHourHumArray[i + 1];
      maxHourHumArray[i] = maxHourHumArray[i + 1];
    }
    
    minHourPpmArray[11] = min_5min_ppm;
    maxHourPpmArray[11] = max_5min_ppm;
    minHourTempArray[11] = min_5min_temp;
    maxHourTempArray[11] = max_5min_temp;
    minHourHumArray[11] = min_5min_hum;
    maxHourHumArray[11] = max_5min_hum;
     
    max_5min_ppm = 0;
    min_5min_ppm = 5000;
    max_5min_temp = 0;
    min_5min_temp = 1000;
    max_5min_hum = 0;
    min_5min_hum = 1000;
  }

    if (hourlyTimer.isReady()){

    max_hour_ppm = calculateMaxFromArray(maxHourPpmArray, 12);
    min_hour_ppm = calculateMinFromArray(minHourPpmArray, 12);
    max_hour_temp = calculateMaxFromArray(maxHourTempArray, 12);
    min_hour_temp = calculateMinFromArray(minHourTempArray, 12);
    max_hour_hum = calculateMaxFromArray(maxHourHumArray, 12);
    min_hour_hum = calculateMinFromArray(minHourHumArray, 12);
    
    for (byte i = 0; i < 23; i++) {
      minDayPpmArray[i] = minDayPpmArray[i + 1];
      maxDayPpmArray[i] = maxDayPpmArray[i + 1];
      minDayTempArray[i] = minDayTempArray[i + 1];
      maxDayTempArray[i] = maxDayTempArray[i + 1];
      minDayHumArray[i] = minDayHumArray[i + 1];
      maxDayHumArray[i] = maxDayHumArray[i + 1];
    }
    minDayPpmArray[23] = min_hour_ppm;
    maxDayPpmArray[23] = max_hour_ppm;
    minDayTempArray[23] = min_hour_temp;
    maxDayTempArray[23] = max_hour_temp;
    minDayHumArray[23] = min_hour_hum;
    maxDayHumArray[23] = max_hour_hum;
  }
}


int calculateMaxFromArray(int *dataArray, byte arrSize) {
  int max_value = 0;
  for (byte i = 0; i < arrSize; i++) {
    if (dataArray[i] > max_value){
      max_value = dataArray[i];
    }
  }

  return max_value;
}


int calculateMinFromArray(int *dataArray, byte arrSize) {
  int min_value = 5000;
  for (byte i = 0; i < arrSize; i++) {
    if (dataArray[i] < min_value && dataArray[i] != 0){
      min_value = dataArray[i];
    }
  }

  return min_value;
}


void printCurrentValues(){  
  if (printTimer.isReady() && DEBUG){
    now = rtc.now();
    secs = now.second();
    mins = now.minute();
    hrs = now.hour();
    
    Serial.print(hrs);
    Serial.print(F(":"));
    Serial.print(mins);
    Serial.print(F(":"));
    Serial.print(secs);
    
    Serial.print(F(" - CO2: "));
    if (!warmedUpFlag){
      Serial.print(F("**"));
      }
    Serial.print(current_ppm); 
    Serial.print(F(" ppm\t"));
    Serial.print(F("Humidity: ")); 
    Serial.print(current_hum / 10.0, 1); 
    Serial.print(F(" %\t")); 
    Serial.print(F("Temperature: ")); 
    Serial.print(current_temp / 10.0, 1); 
    Serial.println(F(" *C."));
  }
}


void printMainScreen(){
  if (clockTimer.isReady()) {
    // get time
    now = rtc.now();
    secs = now.second();
    mins = now.minute();
    hrs = now.hour();

    // print time to LCD
    //lcd.clear();
    lcd.setCursor(0, 0);
    if (hrs < 10) lcd.print(F("0"));
    lcd.print(hrs);
    lcd.print(F(":"));

    // lcd.setCursor(0, 0);
    if (mins < 10) lcd.print(F("0"));
    lcd.print(mins);
    lcd.print(F(":"));

    // lcd.setCursor(0, 0);
    if (secs < 10) lcd.print("0");
    lcd.print(secs);

    if (setTimeFlag){
      lcd.setCursor(0, 1);
      lcd.print(F("Set time"));
    }
    else if (warmedUpFlag){
      // print temperature, CO2 level and humidity
      lcd.setCursor(11, 0);
      lcd.print(current_temp / 10.0, 1);
      lcd.print(F("C"));
      lcd.setCursor(0, 1);
      lcd.print(current_ppm);
      lcd.print(F("ppm "));
      lcd.setCursor(11, 1);
      lcd.print(current_hum / 10.0, 1);
      lcd.print(F("%"));
      }
    else {
      lcd.setCursor(0, 1);
      lcd.print(F("Warming up..."));
    }
  }
}


// Working with LEDs
void switchLed(int ppm_level){
  if (warmingTimer.isReady()){
    lcd.clear();
    warmedUpFlag = true;
  }
  
  if (!warmedUpFlag) {
    blinkLed(YELLOW_LED_PIN);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
  }
  
  else if (ppm_level <= yellow_ppm_level){
    lightGreenLed();
  }

  else if (ppm_level > yellow_ppm_level && ppm_level <= red_ppm_level){
    lightYellowLed();
  }

  else if (ppm_level > red_ppm_level && ppm_level <= alarm_ppm_level){
    lightRedLed();
  }

  else {
    blinkLed(RED_LED_PIN);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
  }
}


void lightGreenLed(){
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  analogWrite(GREEN_LED_PIN, LED_BRIGHTNESS);
}


void lightYellowLed(){
  digitalWrite(RED_LED_PIN, LOW);
  analogWrite(YELLOW_LED_PIN, LED_BRIGHTNESS);
  digitalWrite(GREEN_LED_PIN, LOW);
}


void lightRedLed(){
  analogWrite(RED_LED_PIN, LED_BRIGHTNESS);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
}


void blinkLed(byte led_pin){
  if (blinkTimer.isReady()){
      if (ledState){
        analogWrite(led_pin, LED_BRIGHTNESS);
      }
      else{
        digitalWrite(led_pin, LOW);
      }
      ledState =! ledState;
    } 
}


// Check each button state
void checkButtons(){
  if (!setTimeFlag){
    if (button1.isSingle()){
      switchMode();
    }
  }

  if (button1.isTriple()){
    setTimeFlag = !setTimeFlag;
    switchMode();
  }

  if (button2.isSingle()){
    switchLcdBacklight();
  }

  if (button2.isDouble()){
    updateLedBrightness();
  }

  if (button2.isHolded()){
    updateLcdBrightness();
  }
}


// Switch mode
void switchMode(){
  if (setTimeFlag){
    mode = 8;
  }
  else if (!warmedUpFlag){
    mode = 7;
  }
  else if (mode == 6 || mode == 8){
    mode = 0;
  }
  else{
    mode ++;
  }
  lcd.clear();
  changeModeFlag = true;
  resetModeTimer.setTimeout(RESET_MODE_TIMEOUT);
}


// Switch LCD backlight on and off
void switchLcdBacklight(){
  lcdBacklight = !lcdBacklight;
  lcd.setBacklight(lcdBacklight);
}


// Change the LCD backlight brightness
void updateLcdBrightness(){
  if (LCD_BRIGHTNESS >= 55) {
    LCD_BRIGHTNESS = 5;
  }
  else {
    LCD_BRIGHTNESS += 10;
  }

  if (!lcdBacklight) {
    switchLcdBacklight();
  }
  
  analogWrite(BACKLIGHT, LCD_BRIGHTNESS);
}


// Change the LED brightness
void updateLedBrightness(){
  if (LED_BRIGHTNESS >= 200) {
    LED_BRIGHTNESS = 50;
  }
  else {
    LED_BRIGHTNESS += 50;
  }
}


// Change info on the screen depending on current mode
void displayScreen(){

  if (resetModeTimer.isReady()){
    mode = 0;
    lcd.noBlink();
    lcd.clear();
    setTimeFlag = false;
    setHours = true;
    setMins = false;
    setSecs = false;
  }
  
  switch(mode){
    case 0:
      printMainScreen();
      resetModeTimer.stop();
      changeModeFlag = false;
      break;

    case 1:
      if (changeModeFlag) {
        printHourPpm();
        changeModeFlag = false;
        }
      break;

    case 2:
      if (changeModeFlag) {
        printDayPpm();
        changeModeFlag = false;
        }
      break;

    case 3:
      if (changeModeFlag) {
        printHourTemp();
        changeModeFlag = false;
        }
      break;

    case 4:
      if (changeModeFlag) {
        printDayTemp();
        changeModeFlag = false;
        }
      break;

    case 5:
      if (changeModeFlag) {
        printHourHum();
        changeModeFlag = false;
        }
      break;

    case 6:
      if (changeModeFlag) {
        printDayHum();
        changeModeFlag = false;
        }
      break;

    case 7:
      if (changeModeFlag) {
        lcd.setCursor(0, 0);
        lcd.print(F("Not available"));
        lcd.setCursor(0, 1);
        lcd.print(F("while warming up"));
        changeModeFlag = false;
      }
      break;

    case 8:
      if (changeModeFlag) {
        printMainScreen();
        setTime();
      }
      break;
  }
}


void setTime(){
  if (button1.isHolded()){
    switchTimeSetMode();
  }

  if (button1.isSingle()){
    now = rtc.now();
    secs = now.second();
    mins = now.minute();
    hrs = now.hour();
    int nowYear = now.year();
    byte nowMonth = now.month();
    byte nowDay = now.day();

    if (setHours){
      if (hrs == 23) {
        hrs = 0;
      }
      else {
        hrs ++;
      }
    }

     else if (setMins){
      if (mins == 59) {
        mins = 0;
      }
      else {
        mins ++;
      }
    }

     else if (setSecs){
      secs = 0;
    }

    rtc.adjust(DateTime(nowYear, nowMonth, nowDay, hrs, mins, secs));
    resetModeTimer.setTimeout(RESET_MODE_TIMEOUT * 2);
  }
  
  if (setHours){
    lcd.setCursor(1, 0);
    lcd.blink();
  }

  else if (setMins){
    lcd.setCursor(4, 0);
    lcd.blink();
  }

  else if (setSecs){
    lcd.setCursor(7, 0);
    lcd.blink();
  }
}


void switchTimeSetMode(){
  if (setHours){
    setHours = false;
    setMins = true;
    setSecs = false;
  }
  else if (setMins){
    setHours = false;
    setMins = false;
    setSecs = true;  
  }
  else {
    setHours = true;
    setMins = false;
    setSecs = false;  
  }
  resetModeTimer.setTimeout(RESET_MODE_TIMEOUT * 2);
}


void printOutAllArrays(){
  // Print out hourly array values
    Serial.print(F("maxHourPpmArray = "));
    for (byte i = 0; i < 12; i++) {
      Serial.print(maxHourPpmArray[i]);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("minHourPpmArray = "));
    for (byte i = 0; i < 12; i++) {
      Serial.print(minHourPpmArray[i]);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("maxHourTempArray = "));
    for (byte i = 0; i < 12; i++) {
      Serial.print(maxHourTempArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("minHourTempArray = "));
    for (byte i = 0; i < 12; i++) {
      Serial.print(minHourTempArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("maxHourHumArray = "));
    for (byte i = 0; i < 12; i++) {
      Serial.print(maxHourHumArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("minHourHumArray = "));
    for (byte i = 0; i < 12; i++) {
      Serial.print(minHourHumArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));
    
    // Print out daily array values
    Serial.print(F("maxDayPpmArray = "));
    for (byte i = 0; i < 24; i++) {
      Serial.print(maxDayPpmArray[i]);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("minDayPpmArray = "));
    for (byte i = 0; i < 24; i++) {
      Serial.print(minDayPpmArray[i]);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("maxDayTempArray = "));
    for (byte i = 0; i < 24; i++) {
      Serial.print(maxDayTempArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("minDayTempArray = "));
    for (byte i = 0; i < 24; i++) {
      Serial.print(minDayTempArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("maxDayHumArray = "));
    for (byte i = 0; i < 24; i++) {
      Serial.print(maxDayHumArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));

    Serial.print(F("minDayHumArray = "));
    for (byte i = 0; i < 24; i++) {
      Serial.print(minDayHumArray[i] / 10.0, 1);
      Serial.print(F("; "));
    }
    Serial.println(F(""));
}


// Calculate min and max values for last hour and last day
int calculateCurrentMaxHourPpm(){
    int current_max_hour_ppm = calculateMaxFromArray(maxHourPpmArray, 12);
    current_max_hour_ppm = max(current_max_hour_ppm, max_5min_ppm);
    return current_max_hour_ppm;
}


int calculateCurrentMinHourPpm(){
    int current_min_hour_ppm = calculateMinFromArray(minHourPpmArray, 12);
    current_min_hour_ppm = min(current_min_hour_ppm, min_5min_ppm);
    return current_min_hour_ppm;
}


int calculateCurrentMaxHourTemp(){
    int current_max_hour_temp = calculateMaxFromArray(maxHourTempArray, 12);
    current_max_hour_temp = max(current_max_hour_temp, max_5min_temp);
    return current_max_hour_temp;
}


int calculateCurrentMinHourTemp(){
    int current_min_hour_temp = calculateMinFromArray(minHourTempArray, 12);
    current_min_hour_temp = min(current_min_hour_temp, min_5min_temp);
    return current_min_hour_temp;
}


int calculateCurrentMaxHourHum(){
    int current_max_hour_hum = calculateMaxFromArray(maxHourHumArray, 12);
    current_max_hour_hum = max(current_max_hour_hum, max_5min_hum);
    return current_max_hour_hum;
}


int calculateCurrentMinHourHum(){
    int current_min_hour_hum = calculateMinFromArray(minHourHumArray, 12);
    current_min_hour_hum = min(current_min_hour_hum, min_5min_hum);
    return current_min_hour_hum;
}


int calculateCurrentMaxDayPpm(){
    int current_max_day_ppm = calculateMaxFromArray(maxDayPpmArray, 24);
    int current_max_hour_ppm = calculateCurrentMaxHourPpm();
    current_max_day_ppm = max(current_max_day_ppm, current_max_hour_ppm);
    return current_max_day_ppm;
}


int calculateCurrentMinDayPpm(){
    int current_min_day_ppm = calculateMinFromArray(minDayPpmArray, 24);
    int current_min_hour_ppm = calculateCurrentMinHourPpm();
    current_min_day_ppm = min(current_min_day_ppm, current_min_hour_ppm);
    return current_min_day_ppm;
}


int calculateCurrentMaxDayTemp(){
    int current_max_day_temp = calculateMaxFromArray(maxDayTempArray, 24);
    int current_max_hour_temp = calculateCurrentMaxHourTemp();
    current_max_day_temp = max(current_max_day_temp, current_max_hour_temp);
    return current_max_day_temp;
}


int calculateCurrentMinDayTemp(){
    int current_min_day_temp = calculateMinFromArray(minDayTempArray, 24);
    int current_min_hour_temp = calculateCurrentMinHourTemp();
    current_min_day_temp = min(current_min_day_temp, current_min_hour_temp);
    return current_min_day_temp;
}


int calculateCurrentMaxDayHum(){
    int current_max_day_hum = calculateMaxFromArray(maxDayHumArray, 24);
    int current_max_hour_hum = calculateCurrentMaxHourHum();
    current_max_day_hum = max(current_max_day_hum, current_max_hour_hum);
    return current_max_day_hum;
}


int calculateCurrentMinDayHum(){
    int current_min_day_hum = calculateMinFromArray(minDayHumArray, 24);
    int current_min_hour_hum = calculateCurrentMinHourHum();
    current_min_day_hum = min(current_min_day_hum, current_min_hour_hum);
    return current_min_day_hum;
}


//Print out min and max ppm values for last hour
void printHourPpm(){
    int current_max_hour_ppm = calculateCurrentMaxHourPpm();
    int current_min_hour_ppm = calculateCurrentMinHourPpm();
    
    if (DEBUG){
    Serial.print(F("Max CO2 value for last hour: ")); 
    Serial.print(current_max_hour_ppm); 
    Serial.print(F("; Min CO2 value for last hour:")); 
    Serial.println(current_min_hour_ppm);
    }

    lcd.setCursor(0, 0);
    lcd.print(F("Min/Max CO2, ppm"));
    lcd.setCursor(0, 1);
    lcd.print(current_min_hour_ppm);
    lcd.setCursor(4, 1);
    lcd.print(F("< 1 hr <"));
    if (current_max_hour_ppm < 1000) {
      lcd.setCursor(13, 1);
    }
    lcd.print(current_max_hour_ppm);
}


void printHourTemp(){
    int current_max_hour_temp = calculateCurrentMaxHourTemp();
    int current_min_hour_temp = calculateCurrentMinHourTemp();

    if (DEBUG){
    Serial.print(F("Max Temp value for last hour: ")); 
    Serial.print(current_max_hour_temp / 10.0, 1); 
    Serial.print(F("; Min Temp value for last hour:")); 
    Serial.println(current_min_hour_temp / 10.0, 1);
    }

    lcd.setCursor(0, 0);
    lcd.print(F("Min/Max Temp, *C"));
    lcd.setCursor(0, 1);
    lcd.print(current_min_hour_temp / 10.0, 1);
    lcd.setCursor(4, 1);
    lcd.print(F("< 1 hr <"));
    lcd.print(current_max_hour_temp / 10.0, 1);
}


void printHourHum(){
    int current_max_hour_hum = calculateCurrentMaxHourHum();
    int current_min_hour_hum = calculateCurrentMinHourHum();

    if (DEBUG){
    Serial.print(F("Max Humidity value for last hour: ")); 
    Serial.print(current_max_hour_hum / 10.0, 1); 
    Serial.print(F("; Min Humidity value for last hour:")); 
    Serial.println(current_min_hour_hum / 10.0, 1);
    }

    lcd.setCursor(0, 0);
    lcd.print(F("Min/Max Humid, %"));
    lcd.setCursor(0, 1);
    lcd.print(current_min_hour_hum / 10.0, 1);
    lcd.setCursor(4, 1);
    lcd.print(F("< 1 hr <"));
    lcd.print(current_max_hour_hum / 10.0, 1);
}


//Print out min and max ppm values for last 24 hours
void printDayPpm(){
    int current_max_day_ppm = calculateCurrentMaxDayPpm();
    int current_min_day_ppm = calculateCurrentMinDayPpm();
    
    if (DEBUG){
    Serial.print(F("Max CO2 value for last day: ")); 
    Serial.print(current_max_day_ppm); 
    Serial.print(F("; Min CO2 value for last day:")); 
    Serial.println(current_min_day_ppm);
    }

    lcd.setCursor(0, 0);
    lcd.print(F("Min/Max CO2, ppm"));
    lcd.setCursor(0, 1);
    lcd.print(current_min_day_ppm);
    lcd.setCursor(4, 1);
    lcd.print(F("<24 hrs<"));
    if (current_max_day_ppm < 1000) {
      lcd.setCursor(13, 1);
    }
    lcd.print(current_max_day_ppm);
}


void printDayTemp(){
    int current_max_day_temp = calculateCurrentMaxDayTemp();
    int current_min_day_temp = calculateCurrentMinDayTemp();
    
    if (DEBUG){
    Serial.print(F("Max Temp value for last 24 hours: ")); 
    Serial.print(current_max_day_temp / 10.0, 1); 
    Serial.print(F("; Min Temp value for last 24 hours:")); 
    Serial.println(current_min_day_temp / 10.0, 1);
    }

    lcd.setCursor(0, 0);
    lcd.print(F("Min/Max Temp, *C"));
    lcd.setCursor(0, 1);
    lcd.print(current_min_day_temp / 10.0, 1);
    lcd.setCursor(4, 1);
    lcd.print(F("<24 hrs<"));
    lcd.print(current_max_day_temp / 10.0, 1);
}


void printDayHum(){
    int current_max_day_hum = calculateCurrentMaxDayHum();
    int current_min_day_hum = calculateCurrentMinDayHum();
    
    if (DEBUG){
    Serial.print(F("Max Humidity value for last 24 hours: ")); 
    Serial.print(current_max_day_hum / 10.0, 1); 
    Serial.print(F("; Min Humidity value for last 24 hours:")); 
    Serial.println(current_min_day_hum / 10.0, 1);
    }

    lcd.setCursor(0, 0);
    lcd.print(F("Min/Max Humid, %"));
    lcd.setCursor(0, 1);
    lcd.print(current_min_day_hum / 10.0, 1);
    lcd.setCursor(4, 1);
    lcd.print(F("<24 hrs<"));
    lcd.print(current_max_day_hum / 10.0, 1);
}
