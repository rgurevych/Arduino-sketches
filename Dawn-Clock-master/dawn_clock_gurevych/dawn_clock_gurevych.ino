/*Dawn clock
 * 
 */

// Includes
#include <GyverEncoder.h>
#include <GyverTM1637.h>
#include <CyberLib.h>
#include <GyverButton.h>



// Settings
#define ENCODER_TYPE 1    // 1 or 2


// Pins
#define CLKe 9        // encoder S1
#define DTe 8         // encoder S2
#define SWe 10        // encoder Key

#define CLK 12        // display
#define DIO 11        // display

#define ZERO_PIN 2    // пин детектора нуля (Z-C) для диммера (если он используется)
#define DIM_PIN 5     // мосфет / DIM(PWM) пин диммера

#define BUZZ_PIN 7    // пищалка (по желанию)
#define LED_PIN 6     // светодиод индикатор
#define BUTTON_PIN 4  // кнопка


// Timers


// Objects
Encoder enc(CLKe, DTe, SWe);
GyverTM1637 disp(CLK, DIO);
GButton button(BUTTON_PIN);


// Variables
int lamp = 0;
boolean led_flag;
byte lcd_brightness = 1;
byte led_brightness = 25;
//boolean dotFlag, alarmFlag, minuteFlag, blinkFlag, newTimeFlag;
//int8_t hrs = 21, mins = 55, secs;
//int8_t alm_hrs, alm_mins;
//int8_t dwn_hrs, dwn_mins;
//byte mode;  // 0 - часы, 1 - уст. будильника, 2 - уст. времени
//
//boolean dawn_start = false;
//boolean alarm = false;
volatile int tic, duty;



void setup() {
  Serial.begin(9600);
//  pinMode(DIM_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  led_flag = false;

  enc.setType(ENCODER_TYPE);
  disp.clear();
  disp.brightness(lcd_brightness);

  pinMode(DIM_PIN, OUTPUT);
  pinMode(ZERO_PIN, INPUT);
  digitalWrite(DIM_PIN, LOW);
  attachInterrupt(0, detect_up, FALLING);
  StartTimer1(timer_interrupt, 40);        // время для одного разряда ШИМ
  StopTimer1();                            // остановить таймер
}


void loop() {
  enc.tick();
  button.tick();
  update_lamp();
  check_brightness();
  switch_led();
}


void update_lamp(){
  if (enc.isRight()) lamp++;
  if (enc.isRightH()) lamp += 5;

  if (enc.isLeft()) lamp--;
  if (enc.isLeftH()) lamp -= 5;

  lamp = constrain(lamp, 0, 255);
  disp.displayInt(lamp);
}


void switch_led(){
  if (enc.isClick()) led_flag =! led_flag;
  
  if (led_flag) analogWrite(LED_PIN, led_brightness);
  else digitalWrite(LED_PIN, LOW);
  
  if (led_flag){
    duty = lamp;
  }
  else{
    duty = 0;
    digitalWrite(DIM_PIN, 0);
  }
}


void check_brightness(){
  if (button.isClick()){
    lcd_brightness++;
    led_brightness += 25;
    if (lcd_brightness > 7 ){
      lcd_brightness = 1;
      led_brightness = 25;
    }
    disp.brightness(lcd_brightness);
  }
}


void timer_interrupt() {          // прерывания таймера срабатывают каждые 40 мкс
  if (duty > 0) {
    tic++;                        // счетчик
    if (tic > (255 - duty))       // если настало время включать ток
      digitalWrite(DIM_PIN, 1);   // врубить ток
  }
}

void detect_up() {    // обработка внешнего прерывания на пересекание нуля снизу
  if (duty > 0) {
    tic = 0;                                  // обнулить счетчик
    ResumeTimer1();                           // перезапустить таймер
    attachInterrupt(0, detect_down, RISING);  // перенастроить прерывание
  }
}

void detect_down() {      // обработка внешнего прерывания на пересекание нуля сверху
  if (duty > 0) {
    tic = 0;                                  // обнулить счетчик
    StopTimer1();                             // остановить таймер
    digitalWrite(DIM_PIN, 0);                 // вырубить ток
    attachInterrupt(0, detect_up, FALLING);   // перенастроить прерывание
  }
}
