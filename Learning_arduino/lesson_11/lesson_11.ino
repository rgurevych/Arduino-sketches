#include <MsTimer2.h>
#include <Button.h>

int DOOR_SENS_PIN=12;      // датчик двери подключен к выводу 12
int SECRET_BUTTON_PIN=8;  // скрытая кнопка подключена к выводу 11
int LED_PIN=11;            // светодиод подключен к выводу 10
int SIREN_PIN=9;           // сирена подключена к выводу 9
int TIME_LED_PERIOD=500;   // время периода мигания светодиода (* 2 мс)
int TIME_LED_ON=100;       // время включенного светодиода
int TIME_LED_ALARM=62;   // время периода мигания светодиода при ТРЕВОГЕ (* 2 мс)
unsigned int TIME_ALARM=15000;   // время в режиме ТРЕВОГА (* 2 мс)

volatile unsigned int ledTimeCount; // счетчик времени для светодиода
volatile unsigned int alarmTimeCount; // счетчик времени тревоги

boolean sirenOn;

Button doorSens(DOOR_SENS_PIN, 50);  // создание объекта датчик двери, типа кнопка
Button secretButton(SECRET_BUTTON_PIN, 25); // создание объекта скрытая кнопка, типа кнопка
 
void setup() {
  pinMode(LED_PIN, OUTPUT);      // определяем вывод светодиода как выход
  pinMode(SIREN_PIN, OUTPUT);      // определяем вывод сирены как выход
  MsTimer2::set(2, timerInterupt); // задаем период прерывания по таймеру 2 мс 
  MsTimer2::start();              // разрешаем прерывание по таймеру
}

void loop() {
//---------------------- режим ОТКЛЮЧЕНА --------------------------
guard_off:
  while (true)  {
    digitalWrite(LED_PIN, LOW); // светодиод не горит
    sirenOn= false;           // сирена не звучит

    // если нажали кнопку, переход на режим ОХРАНА
    if ( secretButton.flagClick == true ) {
      secretButton.flagClick= false;
      goto  guard_on;      
    }                 
  }

//---------------------- режим ОХРАНА --------------------------------
guard_on:
  while (true)  {
    sirenOn= false;   // сирена не звучит
    alarmTimeCount= 0;  // сброс счетчика времени тревоги      
    
    // светодиод мигает раз в секунду
    if ( ledTimeCount >= TIME_LED_PERIOD )  ledTimeCount= 0;
    if ( ledTimeCount < TIME_LED_ON )  digitalWrite(LED_PIN, HIGH);
    else digitalWrite(LED_PIN, LOW);

    // если нажали кнопку, переход на режим ОТКЛЮЧЕНА
    if ( secretButton.flagClick == true ) {
      secretButton.flagClick= false;
      goto  guard_off;      
    }    
    
    // если сработал датчик двери, переход на режим ТРЕВОГА
    if ( doorSens.flagPress == true ) goto  alarm;
  }

//---------------------- режим ТРЕВОГА --------------------------------
alarm:
  while (true)  {
    //sirenOn= true;   // звучит сирена
   sirenOn = !sirenOn;
   digitalWrite(SIREN_PIN, sirenOn);
   delay(1);

    // светодиод мигает 4 раза в секунду
    if ( ledTimeCount >= TIME_LED_ALARM ) {
      ledTimeCount= 0;
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }

    // если нажали кнопку, переход на режим ОТКЛЮЧЕНА
    if ( secretButton.flagClick == true ) {
      secretButton.flagClick= false;
      goto  guard_off;      
    }   

    // проверка времени тревоги ( 30 сек )
    if ( alarmTimeCount >= TIME_ALARM ) goto  guard_off;
    
    }                       
}

// обработчик прерывания 2 мс
void  timerInterupt() {

  doorSens.filterAvarage();  // вызов метода фильтрации сигнала для датчика двери  
  secretButton.filterAvarage();  // вызов метода фильтрации сигнала для скрытой кнопки
  //if ( sirenOn == true) digitalWrite(SIREN_PIN, ! digitalRead(SIREN_PIN));
  ledTimeCount++;  // счетчик времени мигания светодиода
  alarmTimeCount++;  // счетчик времени тревоги    

}
