// #include "LC75847.h"
#include <SPI.h>
#include <MD_DS3231.h>
#include <Wire.h>
#include <Time.h> 
// #include <DS3231M.h>

// LC75847 lcd;
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

// byte CCB_address = 0xA1;
// #define SS  10
#define CE_PIN       10
#define LC_ADDRESS   0xA1
#define SPI_MODE_SEL SPI_MODE3
#define SPI_SPEED    200000

//byte Control_Data[2] = {};
byte Display_Data_0[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0b00000000, 0b00000000};
byte Display_Data_1[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0b00000001};
byte Display_Data_2[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0b00000010};
byte Display_Data_3[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0b00000011};

const byte character7SEG[10][5][2] =
{   //0            1             1           2            2            3             3           4
{{0b11001000, 0b11100000}, {0b00001110, 0b10101000}, {0b00000010, 0b10101110}, {0b00000000, 0b00000000}}, //0
{{0b10001000, 0b00000000}, {0b00001100, 0b00000000}, {0b00000010, 0b10000000}, {0b10100000, 0b00000000}}, //1
{{0b11000100, 0b11000000}, {0b00000110, 0b11001000}, {0b00000010, 0b01101100}, {0b01100000, 0b00000000}}, //2
{{0b11001100, 0b10000000}, {0b00001110, 0b11000000}, {0b00000010, 0b11101000}, {0b00000000, 0b00000000}}, //3
{{0b10001100, 0b00100000}, {0b00001100, 0b01100000}, {0b00000010, 0b11000010}, {0b00000000, 0b00000000}}, //4
{{0b01001100, 0b10100000}, {0b00001010, 0b11100000}, {0b00000000, 0b11101010}, {0b00000000, 0b00000000}}, //5
{{0b01001100, 0b11100000}, {0b00001010, 0b11101000}, {0b00000000, 0b11101110}, {0b00000000, 0b00000000}}, //6
{{0b11001000, 0b00000000}, {0b00001110, 0b00000000}, {0b00000010, 0b10100000}, {0b00000000, 0b00000000}}, //7
{{0b11001100, 0b11100000}, {0b00001110, 0b11101000}, {0b00000010, 0b11101110}, {0b00000000, 0b00000000}}, //8
{{0b11001100, 0b10100000}, {0b00001110, 0b11100000}, {0b00000010, 0b11101010}, {0b00000000, 0b00000000}}  //9
};


//byte symbolss[5];
int _textSpeed = 2;
byte _chipEnabledPin = CE_PIN;
unsigned long tepTimer1;
bool tochka = false;
int _address = LC_ADDRESS;


//Установка бита в положение
int set_bit(byte num, byte position)
{
    byte mask = (1 << position);
    return num | mask;
}

// int get_key()   //Вывод нажатой клавиши
// {
//   // if (digitalRead(4) == 0) return 1;
//   if (digitalRead(3) == 0) return 2;
//   // if (digitalRead(2) == 0) return 3;
//   return 0;
// }

//Получение бита с позиции

//---------------------------------------------------------Запись данных в LC75847----------------------------------------------------------------------------------
  // Передача блоков: CE=LOW -> адрес; CE=HIGH -> 16 байт
  void _print() {

    int _chipEnabledPin = CE_PIN;
    int _address = LC_ADDRESS;
    // Блок 0
    digitalWrite(_chipEnabledPin, LOW);
    SPI.transfer((uint8_t)_address);
    digitalWrite(_chipEnabledPin, HIGH);
    for (uint8_t i = 0; i < 16; ++i) SPI.transfer(Display_Data_0[i]);

    // Блок 1
    digitalWrite(_chipEnabledPin, LOW);
    SPI.transfer((uint8_t)_address);
    digitalWrite(_chipEnabledPin, HIGH);
    for (uint8_t i = 0; i < 16; ++i) SPI.transfer(Display_Data_1[i]);

    // Блок 2
    digitalWrite(_chipEnabledPin, LOW);
    SPI.transfer((uint8_t)_address);
    digitalWrite(_chipEnabledPin, HIGH);
    for (uint8_t i = 0; i < 16; ++i) SPI.transfer(Display_Data_2[i]);

    // Блок 3
    digitalWrite(_chipEnabledPin, LOW);
    SPI.transfer((uint8_t)_address);
    digitalWrite(_chipEnabledPin, HIGH);
    for (uint8_t i = 0; i < 16; ++i) SPI.transfer(Display_Data_3[i]);

    digitalWrite(_chipEnabledPin, LOW); // завершить кадр
    delay(_textSpeed);
  }
//----------------------------------------------------------------------Clock_Display-------------------------------------------------------------------------------------
void Clock_Display(byte hhour, byte lhour, byte hminute, byte lminute, bool tochka)
{
//Position 1 (Low minute)
bool  _bit = false;

  Display_Data_0[0] = character7SEG[lminute][0][0];
//  Serial.print("Number ");
//  Serial.println(lminute);
  for(byte a = 4; a <= 7; a++)
  {
    _bit = bitRead(character7SEG[lminute][0][1], a);
    (_bit) ? Display_Data_0[1] |= (1 << a) : Display_Data_0[1] &= ~(1 << a);     
  }
//  Serial.println(Display_Data_0[1], BIN);
//  Serial.print("   ");
//  Serial.println(Display_Data_0[2], BIN);


//Position 1 (High minute)
  for(byte a = 0; a <= 3; a++)
    {
      _bit = bitRead(character7SEG[hminute][1][0], a);
//      (_bit) ? Display_Data_0[1] |= (1 << a) : Display_Data_0[1] &= ~(1 << a); 
      bitWrite(Display_Data_0[1], a, _bit);   
    }

    
  for(byte a = 3; a <= 7; a++)
    {
      _bit = bitRead(character7SEG[hminute][1][1], a);
//      (_bit) ? Display_Data_0[2] |= (1 << a) : Display_Data_0[2] &= ~(1 << a);
      bitWrite(Display_Data_0[2], a, _bit);     
    }

    
//Position 1 (Low hour)    
  for(byte a = 0; a <= 2; a++)
    {
      _bit = bitRead(character7SEG[lhour][2][0], a);
//      (_bit) ? Display_Data_0[2] |= (1 << a) : Display_Data_0[2] &= ~(1 << a);
      bitWrite(Display_Data_0[2], a, _bit);
      if(a == 2) 
      {
//        (tochka) ?  Display_Data_0[2] |= (1 << a) : Display_Data_0[2] &= ~(1 << a);
      (tochka) ? bitWrite(Display_Data_0[2], a, true) :  bitWrite(Display_Data_0[2], a, false);
      }        
    }
  for(byte a = 1; a <= 7; a++)
    {
      _bit = bitRead(character7SEG[lhour][2][1], a);
//      (_bit) ? Display_Data_0[3] |= (1 << a) : Display_Data_0[3] &= ~(1 << a); 
      bitWrite(Display_Data_0[3], a, _bit);    
    }

        
//Position 1 (High hour) 
     
  if(hhour == 1 || hhour == 2) 
  {
    for(byte a = 5; a <= 7; a++)
      {
        _bit = bitRead(character7SEG[hhour][3][0], a);
//        (_bit) ? Display_Data_0[4] |= (1 << a) : Display_Data_0[4] &= ~(1 << a);
      bitWrite(Display_Data_0[4], a, _bit);     
      }
  }
  if(hhour == 0) 
  {
    for(byte a = 5; a <= 7; a++)
      {
        _bit = bitRead(character7SEG[hhour][3][0], a);
//        (_bit) ? Display_Data_0[4] |= (1 << a) : Display_Data_0[4] &= ~(1 << a);
      bitWrite(Display_Data_0[4], a, _bit);     
      }
  } 
        
  _print();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void begin(int chipEnabledPin, int address) 
{
    _chipEnabledPin = chipEnabledPin;
    _address = address;

    pinMode(_chipEnabledPin, OUTPUT);
    digitalWrite(_chipEnabledPin, HIGH); // CE HIGH в простое

    SPI.begin();
    SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE_SEL));

    clear();
  }
//--------------------------------------------------------------------------------------------------------------------------------
  // Очистка D‑бит
  void clear() {
    memset(Display_Data_0, 0, 14);
    memset(Display_Data_1, 0, 15);
    memset(Display_Data_2, 0, 15);
    memset(Display_Data_3, 0, 15);
    _print();
  }




//####################################################################################################################################
void setup() 
{ 
 Wire.begin(); 
//  DateTime now (__DATE__, __TIME__);   
//  rrtc.adjust(DateTime(__DATE__, __TIME__)); // set to compile time; 
     
  // pinMode(2, INPUT_PULLUP);                 // подключаем кнопку 1 к пину и подтягиваем её к питанию
  // pinMode(3, INPUT_PULLUP);                 // подключаем кнопку 2 к пину и подтягиваем её к питанию
  // pinMode(4, INPUT_PULLUP);                 // подключаем кнопку 3 к пину и подтягиваем её к питанию
  // Serial.begin(9600);
  // lcd.begin(SS, CCB_address);
  begin(CE_PIN, LC_ADDRESS);

//  Serial.println(F(" (compiled " __DATE__ " " __TIME__ ")"));   
//  lcd.reset();
//  to_massiv(0, 0, 0, 0);
  // clear();  
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


  // bool tochka = false; 
   if(millis() - tepTimer1 > 500)
    {         
       tepTimer1 = millis();
       RTC.readTime();
      tochka = !tochka;
      // if(RTC.s%2)   tochka = true;
      // else  tochka = false;
 
//      if(RTC.s%2)   lcd.Display_Data_0[1] |= (1 << 0);
//      else  lcd.Display_Data_0[1] &= ~(1 << 0);
//      lcd.display();         
      Clock_Display(RTC.h/10, RTC.h%10, RTC.m/10, RTC.m%10, tochka);

//       _print(CCB_address);

    }
}
//----------------------------------------------------------------------------------------------

//   static byte  j = 0, d = 0, f = 0, k = 0;
//   static byte i = 0;  
//   int key = get_key();
//   static  byte bbit = 1;
     

// //    int key = 2;   
//   if (key == 2)   
//   { 
//     delay(50);                               
//     if (j < 14)
//           { 
//             if(i <= 7)
//                 {
//                     lcd.Display_Data_3[j] = bbit;
//                     Serial.print(j);
//                     Serial.print("  ");            
//                     Serial.print(lcd.Display_Data_3[j], BIN);
//                     Serial.print("  ");
//                     Serial.print(lcd.Display_Data_3[j], DEC); 
//                     Serial.print("   D");
//                     Serial.print((d + 1), DEC);
//                     Serial.print("  ");
//                     Serial.print("bit ");
//                     Serial.println(i);
//                     bbit = (bbit << 1);
//                  // to_massiv(k, f, d, i);  
//                  // _print(CCB_address);
//                     lcd.display();
//                     i++;
//                 }
//               else 
//                 {
//                   bbit = 1;
//                   i = 0;
//                   j++;
//                   d++;
//                 } 

//             if(d > 9)  
//               {
//                 d = 0;
//                 f++;
//                   if(f > 9) 
//                     {
//                       f = 0;                    
//                       k++;
//                       if(k > 2)    k = 0;      
//                     }          

//               }        
//           }
// else j = 0;    
//   }  

// }



