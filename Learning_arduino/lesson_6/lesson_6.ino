/*  Программа sketch_7_2 урока 7 
 *  Подключены две кнопки и светодиод
 *  Каждое нажатие кнопки 1 инвертирует состояние светодиода на плате Ардуино
 *  Каждое нажатие кнопки 2 инвертирует состояние светодиода на макетной плате */
#include <MsTimer2.h>

const int LED_1_PIN=11;     // номер вывода светодиода 1 равен 13
const int LED_2_PIN=9;     // номер вывода светодиода 2 равен 10
const int BUTTON_1_PIN=12;  // номер вывода кнопки 1 равен 12
const int BUTTON_2_PIN=8;  // номер вывода кнопки 2 равен 11

// Описание класса обработки сигналов кнопок
class Button {
  public:
    Button(byte pin, byte timeButton);  // описание конструктора
    boolean flagPress;    // признак кнопка сейчас нажата
    boolean flagClick;    // признак кнопка была нажата (клик)
    void scanState();     // метод проверки состояние сигнала
    void  filterAvarage();  // метод фильтрации сигнала по среднему значению
    void setPinTime(byte pin, byte timeButton); // метод установки номера вывода и времени (числа) подтверждения
  private:
    byte _buttonCount;    // счетчик подтверждений стабильного состояния   
    byte _timeButton;     // время подтверждения состояния кнопки
    byte _pin;            // номер вывода
};

boolean ledState1;         // переменная состояния светодиода 1
boolean ledState2;         // переменная состояния светодиода 2

Button button1(BUTTON_1_PIN, 12);  // создание объекта для кнопки 1
Button button2(BUTTON_2_PIN, 12);  // создание объекта для кнопки 2
   
void setup() {
  pinMode(LED_1_PIN, OUTPUT);           // определяем вывод светодиода 1 как выход
  pinMode(LED_2_PIN, OUTPUT);           // определяем вывод светодиода 2 как выход
  MsTimer2::set(2, timerInterupt); // задаем период прерывания по таймеру 2 мс 
  MsTimer2::start();              // разрешаем прерывание по таймеру
}

// бесконечный цикл с периодом 2 мс
void loop() {
  
  
  // блок управления светодиодом 1
  if ( button1.flagClick == true ) {
    // было нажатие кнопки
    button1.flagClick= false;         // сброс признака клика
    ledState1= ! ledState1;             // инверсия состояния светодиода 1
    digitalWrite(LED_1_PIN, ledState1);  // вывод состояния светодиода 1    
  }

  // блок управления светодиодом 2
  if ( button2.flagClick == true ) {
    // было нажатие кнопки
    button2.flagClick= false;         // сброс признака клика
    ledState2= ! ledState2;             // инверсия состояние светодиода 2
    digitalWrite(LED_2_PIN, ledState2);  // вывод состояния светодиода 2    
  }

 // delay(2);  // задержка на 2 мс
}

// метод проверки состояния кнопки
// flagPress= true  - нажата 
//  flagPress= false - отжата
//  flagClick= true - была нажата (клик)
void Button::scanState() {
  if ( flagPress == (! digitalRead(_pin)) ) {
     // состояние сигнала осталось прежним 
     _buttonCount= 0;  // сброс счетчика состояния сигнала
}
  else {
     // состояние сигнала изменилось
     _buttonCount++;   // +1 к счетчику состояния сигнала

     if ( _buttonCount >= _timeButton ) {
      // состояние сигнала не менялось заданное время
      // состояние сигнала стало устойчивым
      flagPress= ! flagPress; // инверсия признака состояния
_buttonCount= 0;  // сброс счетчика состояния сигнала

 

      if ( flagPress == true ) flagClick= true; // признак клика на нажатие      
     }    
  }
}

/*// метод установки номера вывода и времени подтверждения
void Button::setPinTime(byte pin, byte timeButton)  {

  _pin= pin;
  _timeButton= timeButton;
  pinMode(_pin, INPUT_PULLUP);  // определяем вывод как вход
}*/
void  timerInterupt(){
  button1.filterAvarage();  // вызов метода сканирования сигнала кнопки 1
  button2.scanState();  // вызов метода сканирования сигнала кнопки 2
  }


void Button::filterAvarage() {

 if ( flagPress != digitalRead(_pin) ) {
     //  состояние кнопки осталось прежним
     if ( _buttonCount != 0 ) _buttonCount--; // счетчик подтверждений - 1 с ограничением на 0 
  }
  else {
     // состояние кнопки изменилось
     _buttonCount++;   // +1 к счетчику подтверждений

     if ( _buttonCount >= _timeButton ) {
      // состояние сигнала достигло порога _timeButton
      flagPress= ! flagPress; // инверсия признака состояния
     _buttonCount= 0;  // сброс счетчика подтверждений

      if ( flagPress == true ) flagClick= true; // признак клика кнопки       
     }    
  }
}

// описание конструктора класса Button
Button::Button(byte pin, byte timeButton) {

  _pin= pin;
  _timeButton= timeButton;
  pinMode(_pin, INPUT_PULLUP);  // определяем вывод как вход
}
