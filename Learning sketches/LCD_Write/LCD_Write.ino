#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3f,16,2);  // run ic2_scanner sketch and get the IC2 address, which is 0x3f in my case,it could be 0x3f in many cases
 
void setup()
{
  lcd.init();                      // initialize the lcd
}
 
void loop()
{
// set cursor to first line
lcd.setCursor(0, 0);

// Print a message to the LCD.
  lcd.backlight();
  lcd.print("Hallo, Nina!");
  lcd.setCursor(6,1);
  lcd.print(millis(), 1);
  lcd.setCursor(0,1);
  lcd.print("Time=");
  delay(1000);
  
}
