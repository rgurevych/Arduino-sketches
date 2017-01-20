/* Use buzzer to make sound
 * tutorial http://osoyoo.com/?p=251
 */
int buzzer=12;
void setup()
{
  pinMode(buzzer,OUTPUT);
}
void loop()
{
  for(int i=0;i<200;i++)
    {
      digitalWrite(buzzer,HIGH);
      delay(1);
      digitalWrite(buzzer,LOW);
      delay(1);
    }
  for(int i=0;i<100;i++)
    {
      digitalWrite(buzzer,HIGH);
      delay(2);
      digitalWrite(buzzer,LOW);
      delay(2);
    }
}
