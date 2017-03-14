const int LED_PIN=11;     // номер вывода светодиода равен 13
const int BUTTON_PIN=12;  // номер вывода кнопки равен 12

void setup() {
  pinMode(LED_PIN, OUTPUT);    // определяем вывод 13 (светодиод) как выход
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // определяем вывод 12 (кнопка) как вход

}

void loop() {
  digitalWrite(LED_PIN, ! digitalRead(BUTTON_PIN) );
}
