#include <SoftwareSerial.h>
#include <Wire.h>

//// I2C OLED
//#include "SSD1306Ascii.h"
//#include "SSD1306AsciiWire.h"
//#define I2C_ADDRESS 0x3C
//SSD1306AsciiWire oled;

// CO2 sensor:
SoftwareSerial mySerial(8,9); // RX,TX
byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
unsigned char response[9];

void setup() {
  // Serial
  Serial.begin(9600);
  mySerial.begin(9600);

//  // OLED
//  Wire.begin();         
//  oled.begin(&Adafruit128x32, I2C_ADDRESS);
//  oled.set400kHz();  
//  oled.setFont(ZevvPeep8x16);  
//
//  oled.clear();  
//  oled.println("setup::init()");
}

long t = 0;

void loop() 
{
  mySerial.write(cmd, 9);
  memset(response, 0, 9);
  mySerial.readBytes(response, 9);
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc+=response[i];
  crc = 255 - crc;
  crc++;

//  oled.clear();  
  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    Serial.println("CRC error: " + String(crc) + " / "+ String(response[8]));
//    oled.println("Sensor CRC error");
  } else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    unsigned int ppm = (256*responseHigh) + responseLow;
    Serial.print(String(t)); Serial.print(","); Serial.print(ppm); Serial.println(";");
//    if (ppm <= 400 || ppm > 4900) {
//      oled.println("CO2: no data");          
//    } else {
//      oled.println("CO2: " + String(ppm) + " ppm"); 
//      if (ppm < 450) {   
//        oled.println("Very good");
//      }
//      else if (ppm < 600) {   
//        oled.println("Good");
//      }
//      else if (ppm < 1000) {   
//        oled.println("Acceptable");
//      }
//      else if (ppm < 2500) {   
//        oled.println("Bad");
//      }
//     else {   
//        oled.println("Health risk");
//      }
//    }
  }
  delay(10000);
  t += 10;
}
