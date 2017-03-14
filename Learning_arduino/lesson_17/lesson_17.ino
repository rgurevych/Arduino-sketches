// охранная сигнализация
#include <MsTimer2.h>
#include <Button.h>
#include <avr/wdt.h>
#include <EEPROM.h>

// назначение выводов
#define BUTTON_1_PIN 9  // кнопка 1 пульта
#define BUTTON_2_PIN 10 // кнопка 2 пульта
#define LED_PIN 11    // светодиод пульта и внешняя индикация
#define SIREN_PIN 12  // сирена подключена к выводу 9
#define SHLEIF_1_PIN A0 // шлейф 1

// параметры
#define SET_COD_TIME 3000 // время удержания кнопок для перехода на режим установки кода (* 2 мс = 6 сек)
#define FAST_TIME_FLASH_LED 50 // время частого мигания светодиода (* 2 мс, 5 раз в сек)
#define SLOW_TIME_FLASH_LED 250 // время редкого мигания светодиода (* 2 мс, 1 раз в сек)
#define TIME_SET_GUARD 10000  // время установки на сигнализацию (* 2 мс, 20 сек)
#define TIME_BLOCK 10000  // время на ввод секретного кода (* 2 мс, 30 сек)
#define TIME_ALARM 30000  // время на звучание сирены (* 2 мс, 60 сек)
#define TIME_SET_COD 2500  // время паузы между вводом цифр кода (* 2 мс, 5 сек)
#define MAX_U 75 // верхний предел напряжение шлейфов 
#define MIN_U 25 // нижний предел напряжение шлейфов 

// адреса EEPROM
#define COD_ADR  2    // адрес секретного кода
#define NUMBER_ADR  4 // адрес числа битов секретного кода

Button button1(BUTTON_1_PIN, 25); // создание объекта кнопка 1
Button button2(BUTTON_2_PIN, 25); // создание объекта кнопка 2

unsigned int sumShleif1; // переменные для суммирования кодов АЦП
unsigned int avarageShleif1; // сумма кодов АЦП (среднее напряжение шлейфов * 50)
int avarageCount; // счетчик усреднния кодов АЦП (напряжения шлейфов)
int serialCount;    // счетчик времени передачи отладочных данных через UART
byte mode;  // режим
boolean flagTwoButtons; // признак были нажаты две кнопки
unsigned int commonTimer; // таймер для разных целей
unsigned int ledFlashCount; // счетчик мигания светодиода
byte  secretCod;  // переменная для секретного кода
byte  bitNum;  // переменная для номера бита секретного кода


void setup() {
  pinMode(LED_PIN, OUTPUT);      // вывод светодиода
  pinMode(SIREN_PIN, OUTPUT);    // вывод сирены
  
  MsTimer2::set(2, timerInterrupt); // период прерывания по таймеру 2 мс 
  MsTimer2::start();                // разрешаем прерывание по таймеру

  Serial.begin(9600); // инициализируем порт, скорость 9600
  wdt_enable(WDTO_15MS); // разрешаем сторожевой таймер, тайм-аутом 15 мс  

  flagTwoButtons= false;
}


void loop() {

  if ( mode == 0 )  {
    // СИГНАЛИЗАЦИЯ ОТКЛЮЧЕНА
    digitalWrite(LED_PIN, LOW); // светодиод не горит
    digitalWrite(SIREN_PIN, LOW); // сирена не звучит
    if ( (button1.flagPress == true) && (button2.flagPress == true) ) flagTwoButtons= true;
    if ( (flagTwoButtons == true) && (button1.flagPress == false) && (button2.flagPress == false)  ) {
      // переход на установку сигнализации
      commonTimer= 0;
      button1.flagClick= false;
      button2.flagClick= false;
      mode = 1;   
    }

    // переход на установку кода (долгое удержание двух кнопок)  
    // если не нажата хотя бы одна кнопка, то обнуляем commonTimer
    // как только commonTimer насчитывает 6 сек, то переходим на режим 5  
    if ( (button1.flagPress == false) || (button2.flagPress == false) ) commonTimer= 0;
    if ( commonTimer > SET_COD_TIME ) {
      // переход на режим 5
      commonTimer= 0;
      button1.flagClick= false;
      button2.flagClick= false;
      secretCod= 0;
      bitNum= 0;      
      mode= 5;      
    }   


        
  }

  else if ( mode == 1 )  {
    // УСТАНОВКА НА СИГНАЛИЗАЦИЮ
    digitalWrite(SIREN_PIN, LOW); // сирена не звучит

    // светодиод мигает 5 раз в сек
    if ( ledFlashCount > FAST_TIME_FLASH_LED ) {
      ledFlashCount= 0;
      digitalWrite(LED_PIN, ! digitalRead(LED_PIN));  // инверсия светодиода
    }

    // проверка времени перехода на режим ОХРАНА
    if ( commonTimer >= TIME_SET_GUARD ) {
      // переход на режим 2 (ОХРАНА)
      commonTimer= 0;
      button1.flagClick= false;
      button2.flagClick= false;
      secretCod= 0;
      bitNum= 0;
      mode= 2;                        
    }

    // отказ при нажатии на любую кнопку
    if ( (button1.flagClick == true) || (button2.flagClick == true) ) {
      // переход на режим 0 (СИГНАЛИЗАЦИЯ ОТКЛЮЧЕНА)
      commonTimer= 0;
      button1.flagClick= false;
      button2.flagClick= false;
      flagTwoButtons= false;
      mode= 0;              
    }                                    
  }



  else if ( mode == 2 )  {
    // ОХРАНА  
    digitalWrite(SIREN_PIN, LOW); // сирена не звучит

    // светодиод мигает 1 раз в сек
    if ( ledFlashCount > SLOW_TIME_FLASH_LED ) {
      ledFlashCount= 0;
      digitalWrite(LED_PIN, ! digitalRead(LED_PIN));  // инверсия светодиода
    }

    // проверка состояния шлейфов (датчиков)
    if ( ((avarageShleif1/100) > MAX_U) || ((avarageShleif1/100) < MIN_U)) {
          // переход на режим 3 (БЛОКИРОВКА)
          commonTimer= 0;
          button1.flagClick= false;
          button2.flagClick= false;
          secretCod= 0;
          bitNum= 0;
          mode= 3;                        
         }

    // проверка секретного кода (оформлена функцией)
    secretCodCheck();  
  }



  else if ( mode == 3 )  {
    // БЛОКИРОВКА
    digitalWrite(SIREN_PIN, LOW); // сирена не звучит

    // светодиод мигает 5 раз в сек
    if ( ledFlashCount > FAST_TIME_FLASH_LED ) {
      ledFlashCount= 0;
      digitalWrite(LED_PIN, ! digitalRead(LED_PIN));  // инверсия светодиода
    }

    // проверка времени на ввод секретного кода
    if ( commonTimer >= TIME_BLOCK ) {
      // переход на режим 4 (ТРЕВОГА)
      commonTimer= 0;
      button1.flagClick= false;
      button2.flagClick= false;
      secretCod= 0;
      bitNum= 0;
      mode= 4;                        
    }

    // проверка секретного кода (оформлена функцией)
    secretCodCheck();    
      
  }



  else if ( mode == 4 )  {
    // ТРЕВОГА  
    digitalWrite(SIREN_PIN, HIGH); // звучит сирена

    // светодиод мигает 5 раз в сек
    if ( ledFlashCount > FAST_TIME_FLASH_LED ) {
      ledFlashCount= 0;
      digitalWrite(LED_PIN, ! digitalRead(LED_PIN));  // инверсия светодиода
    }

    // проверка времени звучания сирены
    if ( commonTimer >= TIME_ALARM ) {
      // переход на режим 0 (СИГНАЛИЗАЦИЯ ОТКЛЮЧЕНА)
      commonTimer= 0;
      button1.flagClick= false;
      button2.flagClick= false;
      flagTwoButtons= false;
      mode= 2;    
    }                   

    // проверка секретного кода (оформлена функцией)
    secretCodCheck();          
  }

  

  else if ( mode == 5 )  {
    // УСТАНОВКА КОДА    
    digitalWrite(SIREN_PIN, LOW); // сирена не звучит
    digitalWrite(LED_PIN, HIGH);  // светодиод светится

    // ввод секретного кода
    if ( (button1.flagClick == true) || (button2.flagClick == true) ) {
      // кнопку нажали
      commonTimer= 0; // сброс счетчика времени
      secretCod= secretCod << 1; // сдвиг secretCod на 1 бит
      // если нажали кнопку 1 установка 0 в младший бит secretCod 
      if ( button1.flagClick == true ) { 
        button1.flagClick= false; 
        secretCod &= 0xfe;  // установка 0 в младший бит
      }
      // если нажали кнопку 2 установка 1 в младший бит secretCod 
      if ( button2.flagClick == true ) { 
        button2.flagClick= false; 
        secretCod |= 1;  // установка 1 в младший бит
      }
      bitNum++; // еще один бит ввели      
    }

    // проверка времени паузы между вводом цифр
    if ( commonTimer >= TIME_SET_COD ) {
      // запись кода и числа цифр кода в EEPROM
      EEPROM.write(COD_ADR, secretCod);  
      EEPROM.write(NUMBER_ADR, bitNum);        
      // переход на режим 0 (СИГНАЛИЗАЦИЯ ОТКЛЮЧЕНА)
      commonTimer= 0;
      button1.flagClick= false;
      button2.flagClick= false;
      flagTwoButtons= false;
      mode= 0;    
    }                        
  }
  else  mode= 0;
  
  // передача отладочных данных через UART
  // каждые 500 мс
  if ( serialCount >= 250 ) {

    // состояние кнопок
    if ( button1.flagPress == true ) Serial.print("Btn1 -_- ");
    else Serial.print("Btn1 _-_ ");
    if ( button2.flagPress == true ) Serial.print("Btn2 -_- ");
    else Serial.print("Btn2 _-_ ");

    Serial.print("Mode= ");  
    Serial.print(mode);
    
    // напряжения шлейфов
    Serial.print(" Shleif1= ");
    Serial.print( (float)avarageShleif1/100, 2);
    Serial.println(" V");

    
    serialCount = 0;
    
    // * 0.000097656 = / 50. * 5. / 1024.            
  }
}

// обработчик прерывания, 2 мс
void  timerInterrupt() {
  wdt_reset();  // сброс сторожевого таймера 

  button1.filterAvarage();  // вызов метода фильтрации сигнала кнопки 1  
  button2.filterAvarage();  // вызов метода фильтрации сигнала кнопки 2

  // чтение АЦП и усреднение значений напряжений шлейфов
  // в результате avarageShleif = среднее напряжение шлейфа * 50   
  avarageCount++;  // +1 счетчик усреднения
  sumShleif1 += analogRead(SHLEIF_1_PIN);  // суммирование кодов АЦП

  // проверка количества выборок усреднения (50)
  if ( avarageCount >= 50 ) {
    avarageCount= 0;
    avarageShleif1 = sumShleif1; // перегрузка среднего значения 
    sumShleif1 = 0;
    }

  serialCount++;  // счетчик времени передачи отладочных данных через UART
  commonTimer++;  // таймер
  ledFlashCount++; // счетчик мигания светодиода  
}


void secretCodCheck() {
    
    // ввод секретного кода
    if ( (button1.flagClick == true) || (button2.flagClick == true) ) {
      // кнопку нажали
      secretCod= secretCod << 1; // сдвиг secretCod на 1 бит
      // если нажали кнопку 1 установка 0 в младший бит secretCod 
      if ( button1.flagClick == true ) { 
        button1.flagClick= false; 
        secretCod &= 0xfe;  // установка 0 в младший бит
      }
      // если нажали кнопку 2 установка 1 в младший бит secretCod 
      if ( button2.flagClick == true ) { 
        button2.flagClick= false; 
        secretCod |= 1;  // установка 1 в младший бит
      }
      bitNum++; // еще один бит ввели      
    }

    // проверка секретного кода
      // проверка числа цифр    
      if ( bitNum == EEPROM.read(NUMBER_ADR) ) {
        // ввели все цифры кода
        // проверка секретного кода 
        if ( secretCod == EEPROM.read(COD_ADR) ) {
          // код совпал, переход на режим 0 (СИГНАЛИЗАЦИЯ ОТКЛЮЧЕНА)
          commonTimer= 0;
          button1.flagClick= false;
          button2.flagClick= false;
          flagTwoButtons= false;
          mode= 0;              
        }    
        // код набрали не правильный
        else { secretCod= 0; bitNum= 0; }    
      }

    // сброс кода по нажатию на две кнопки
    if ( (button1.flagPress == true) && (button2.flagPress == true) ) {
        button1.flagClick= false;
        button2.flagClick= false;
        secretCod= 0;
        bitNum= 0;      
    }    
}
