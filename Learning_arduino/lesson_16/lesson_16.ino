// проверка работы сторожевого таймера
#include <MsTimer2.h>
#include <avr/wdt.h>

#define LED_PIN 11 // светодиод подключен к выводу 13
int ledCount;             // счетчик времени мигания светодиода

void setup() {
  pinMode(LED_PIN, OUTPUT);      // определяем вывод светодиода как выход
  Serial.begin(9600);     // инициализируем последовательный порт
  MsTimer2::set(2, timerInterupt); // задаем период прерывания от таймера 2 мс 
  MsTimer2::start();              // разрешаем прерывание от таймеру
  wdt_enable(WDTO_15MS); // разрешение работу сторожевого таймера с тайм-аутом 15 мс  
}

void loop() {
  // мигание светодиода
  if ( ledCount > 250 ) {
     ledCount= 0;  
     digitalWrite(LED_PIN, ! digitalRead(LED_PIN));  // инверсия состояния светодиода
  }   

  // проверка данных в буфере последовательного порта (имитация сбоя)
  if ( Serial.available() != 0 ) MsTimer2::stop(); // запрет прерывания от таймера    
}

// обработчик прерывания 
void  timerInterupt() {
  ledCount++; // счетчик светодиода
  wdt_reset();  // сброс сторожевого таймера  
}
