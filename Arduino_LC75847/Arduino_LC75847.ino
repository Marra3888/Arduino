#include "LC75847.h"
//#include <SPI.h>
#include <MD_DS3231.h>
#include <Wire.h>
#include <Time.h> 
// #include <DS3231M.h>

LC75847 lcd;
// DS3231M_Class rrtc;
//RTClib RTCC;

#define seg_A  0
#define seg_B  1
#define seg_C  2
#define seg_D  3
#define seg_E  4
#define seg_F  5
#define seg_G  6


//char full[9] = "########";
//char alphabet[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
//char number[11] = "0123456789";

byte CCB_address = 0xA1;
#define SS  10

//byte Control_Data[2] = {};
//extern byte Display_Data_0[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0b00000000, 0b00000000};
//extern byte Display_Data_1[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0b00000001};
//extern byte Display_Data_2[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0b00000010};
//extern byte Display_Data_3[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0b00000011};


//byte symbolss[5];
int _textSpeed = 0;
byte _chipEnabledPin = 10;
unsigned long tepTimer1;
bool tochka = false;

//Установка бита в положение
int set_bit(byte num, byte position)
{
    byte mask = (1 << position);
    return num | mask;
}

int get_key()   //Вывод нажатой клавиши
{
  // if (digitalRead(4) == 0) return 1;
  if (digitalRead(3) == 0) return 2;
  // if (digitalRead(2) == 0) return 3;
  return 0;
}

//Получение бита с позиции

//---------------------------------------------------------Запись данных в LC75847----------------------------------------------------------------------------------

//----------------------------------------------------------------------Clock_Display-------------------------------------------------------------------------------------

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup() 
{ 
//  Wire.begin(); 
//  DateTime now (__DATE__, __TIME__);   
//  rrtc.adjust(DateTime(__DATE__, __TIME__)); // set to compile time; 
     
  // pinMode(2, INPUT_PULLUP);                 // подключаем кнопку 1 к пину и подтягиваем её к питанию
  pinMode(3, INPUT_PULLUP);                 // подключаем кнопку 2 к пину и подтягиваем её к питанию
  // pinMode(4, INPUT_PULLUP);                 // подключаем кнопку 3 к пину и подтягиваем её к питанию
  Serial.begin(9600);
  lcd.begin(SS, CCB_address);

//  Serial.println(F(" (compiled " __DATE__ " " __TIME__ ")"));   
//  lcd.reset();
//  to_massiv(0, 0, 0, 0);
  lcd.clear();  
//-----------------------------------------------------------------------------------------------------------------------------------------------  
/*  for(byte j = 0; j < 13; j++)
  {
  byte bbit = 1;   
    for(int i = 0; i <= 7; i++)
    {
      Display_Data_0[j] = bbit;
      bbit <<= 1;
      Serial.print(j);
      Serial.print("  ");            
      Serial.print(Display_Data_0[j], BIN);
      Serial.print("  ");
      Serial.println(Display_Data_0[j], DEC);                  
      _print(CCB_address); 
    }        

  } */
/*  for(byte j = 0; j < 14; j++)
  {
    for(int i = 0; i <= 255; i++)
    {
      Display_Data_1[j] = i;
      Serial.print(j);
      Serial.print("  ");            
      Serial.println(Display_Data_1[j]); 
      _print(CCB_address); 
    }
              
  }
  for(byte j = 0; j < 14; j++)
  {
    for(int i = 0; i <= 255; i++)
    {
      Display_Data_2[j] = i;
      Serial.print(j);
      Serial.print("  ");            
      Serial.println(Display_Data_2[j]); 
      _print(CCB_address); 
    }        
  }*/
//        _print(CCB_address); 

}


void loop() 
{


//   // bool tochka = false; 
//    if(millis() - tepTimer1 > 500)
//     {         
//        tepTimer1 = millis();
//        RTC.readTime();
//       tochka = !tochka;
//       // if(RTC.s%2)   tochka = true;
//       // else  tochka = false;
 
// //      if(RTC.s%2)   lcd.Display_Data_0[1] |= (1 << 0);
// //      else  lcd.Display_Data_0[1] &= ~(1 << 0);
// //      lcd.display();         
//       lcd.Clock_Display(RTC.h/10, RTC.h%10, RTC.m/10, RTC.m%10, tochka);

// //       _print(CCB_address);

    // }

//----------------------------------------------------------------------------------------------

  static byte  j = 0, d = 0, f = 0, k = 0;
  static byte i = 0;  
  int key = get_key();
  static  byte bbit = 1;
     

//    int key = 2;   
  if (key == 2)   
  { 
    delay(50);                               
    if (j < 14)
          { 
            if(i <= 7)
                {
                    lcd.Display_Data_3[j] = bbit;
                    Serial.print(j);
                    Serial.print("  ");            
                    Serial.print(lcd.Display_Data_3[j], BIN);
                    Serial.print("  ");
                    Serial.print(lcd.Display_Data_3[j], DEC); 
                    Serial.print("   D");
                    Serial.print((d + 1), DEC);
                    Serial.print("  ");
                    Serial.print("bit ");
                    Serial.println(i);
                    bbit = (bbit << 1);
                 // to_massiv(k, f, d, i);  
                 // _print(CCB_address);
                    lcd.display();
                    i++;
                }
              else 
                {
                  bbit = 1;
                  i = 0;
                  j++;
                  d++;
                } 

            if(d > 9)  
              {
                d = 0;
                f++;
                  if(f > 9) 
                    {
                      f = 0;                    
                      k++;
                      if(k > 2)    k = 0;      
                    }          

              }        
          }
else j = 0;    
  }  

}



