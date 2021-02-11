/* Hardware Connections */
// -- LEDs --
#define RED_LED 5
#define GREEN_LED 6
#define BLUE_LED 7
#define YELLOW_LED 8 

// -- Buttons --
#define RED_BUTTON 14
#define GREEN_BUTTON 15
#define BLUE_BUTTON 16
#define YELLOW_BUTTON 17  

// -- Display Selection (uncomment ONE of the options) -- 
#define LCD_SSD1309
//#define LCD_SH1106      //If you see abnormal vertical line at the left edge of the display, select LCD_SSD1306
//#define LCD_SSD1306     //If you see abnormal vertical line at the right edge of the display, select LCD_SH1106

// -- Display Connection for SSD1309 --
#define LCD_SSD1309_CS 2
#define LCD_SSD1309_DC 3
#define LCD_SSD1309_RESET 4
//LCD_SSD1309_SDA -> 11 (SPI SCK)
//LCD_SSD1309_SCL -> 13 (SPI MOSI)

// -- Display Connection for SH1106/SSD1306 --
//LCD_SH1106_SDA -> A4 (I2C SDA)
//LCD_SH1106_SCL -> A5 (I2C SCL)

/* Misc. Settings */
//#define AUTO_PLAY //uncomment to enable auto-play mode
//#define RESET_HI_SCORE //uncomment to reset HI score, flash your device, than comment it back and flash again
//#define PRINT_DEBUG_INFO

/* Game Balance Settings */
#define PLAYER_SAFE_ZONE_WIDTH 32 //minimum distance between obstacles (px)
#define CACTI_RESPAWN_RATE 50 //lower -> more frequent, max 255
#define GROUND_CACTI_SCROLL_SPEED 3 //pixels per game cycle
#define PTERODACTY_SPEED 5 //pixels per game cycle
#define PTERODACTY_RESPAWN_RATE 255 //lower -> more frequent, max 255
#define INCREASE_FPS_EVERY_N_SCORE_POINTS 256 //better to be power of 2
#define LIVES_START 3
#define LIVES_MAX 5
#define SPAWN_NEW_LIVE_MIN_CYCLES 800
#define DAY_NIGHT_SWITCH_CYCLES 1024 //better to be power of 2
#define TARGET_FPS_START 23
#define TARGET_FPS_MAX 48 //gradually increase FPS to that value to make the game faster and harder

/* Display Settings */
#define LCD_HEIGHT 64U
#define LCD_WIDTH 128U

/* Render Settings */
#ifdef LCD_SSD1309
  //#define VIRTUAL_HEIGHT_BUFFER_ROWS_BY_8_PIXELS 1
  #define VIRTUAL_WIDTH_BUFFER_COLS 16
#else
  //VIRTUAL_HEIGHT_BUFFER_ROWS_BY_8_PIXELS is not supported
  #define VIRTUAL_HEIGHT_BUFFER_ROWS_BY_8_PIXELS 4
#endif

/* Includes */
#include <EEPROM.h>
#include "array.h"
#include "TrexPlayer.h"
#include "Ground.h"
#include "Cactus.h"
#include "Pterodactyl.h"
#include "HeartLive.h"

/* Defines and globals */
#define EEPROM_HI_SCORE 16 //2 bytes
#define LCD_BYTE_SZIE (LCD_WIDTH*LCD_HEIGHT/8)

#ifdef VIRTUAL_WIDTH_BUFFER_COLS
  #define LCD_IF_VIRTUAL_WIDTH(TRUE_COND, FALSE_COND) TRUE_COND
  #define LCD_PART_BUFF_WIDTH VIRTUAL_WIDTH_BUFFER_COLS
  #define LCD_PART_BUFF_HEIGHT LCD_HEIGHT
#else
  #define LCD_IF_VIRTUAL_WIDTH(TRUE_COND, FALSE_COND) FALSE_COND
  #ifdef VIRTUAL_HEIGHT_BUFFER_ROWS_BY_8_PIXELS
    #define LCD_PART_BUFF_WIDTH LCD_WIDTH
    #define LCD_PART_BUFF_HEIGHT (VIRTUAL_HEIGHT_BUFFER_ROWS_BY_8_PIXELS*8)
  #else
    #define LCD_PART_BUFF_WIDTH LCD_WIDTH
    #define LCD_PART_BUFF_HEIGHT LCD_HEIGHT
  #endif
#endif
#define LCD_PART_BUFF_SZ ((LCD_PART_BUFF_HEIGHT/8)*LCD_PART_BUFF_WIDTH)

#ifdef LCD_SSD1309
  #include <SPI.h>
  #include "SSD1309.h"
  static SSD1309<SPIClass> lcd(SPI, LCD_SSD1309_CS, LCD_SSD1309_DC, LCD_SSD1309_RESET, LCD_BYTE_SZIE);
#else
  #include "I2C.h"
  #include "SH1106.h"
  I2C i2c;
  SH1106<I2C> lcd(i2c, LCD_BYTE_SZIE);
#endif

boolean RED_LED_FLAG=false, GREEN_LED_FLAG=false, BLUE_LED_FLAG=false, YELLOW_LED_FLAG=false;
