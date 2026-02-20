
#include "LC75847.h"

const byte CCB_address = 0xA1;
#define SS 10

LC75847 lcd;

int d = 1;

int get_key()   //Вывод нажатой клавиши
{
  // if (digitalRead(4) == 0) return 1;
  if (digitalRead(3) == 0) return 2;
  // if (digitalRead(2) == 0) return 3;
  return 0;
}

void setup() 
{
  Serial.begin(115200);
  // pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  // pinMode(4, INPUT_PULLUP);
  lcd.begin(SS, CCB_address);
 
    //  lcd.display();
    //      lcd.display();
    //  delay(5000);  
  // lcd.clear();
   Serial.println("LCD готов.");
}

void loop() 
{
 
  if (get_key() == 2)   
  { 
    delay(300);                               
    lcd.setD(d, true); // MSB-first (по даташиту)
    lcd.display();

    Serial.print("D");
    Serial.println(d);
    d++;
    if(d > 420) {d = 1; lcd.clear();}
  } 
}