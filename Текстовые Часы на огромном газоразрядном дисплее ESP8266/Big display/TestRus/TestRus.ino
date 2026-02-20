/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD
 and shows the time.

  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 modified 7 Nov 2016
 by Arturo Guadalupi

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

*/
char Buffer[] = "Hello String";
// include the library code:

byte i;
byte i2;
byte s;
float Temperature = 16.2;


void setup()
{
    Serial.begin(9600);
    Serial.print("\f");
    delay(50);
    //Serial.println("\0\1\2\3\4\5\6\7\8\9\0\1\2\3\4\5\6\7\8\9");
    Serial.println("\230\1\2\3\4\5\6\7\8\9\0\1\2\3\4\5\6\7\8\9");
    delay(2000);
    Serial.println("\0\1\2\3\4\5\6\7\8\9\0\1\2\3\4\5\6\7\8\9");
    delay(2000);
}

void loop()
{
  /*for(s=0; s<255; s = s+20)
  {
    for(i = s; i <(s+20) ;i++){//т.к. экран 16х2, то больше 32 не выведешь
        sprintf(Buffer,"%s%d","\\", i);
        Serial.print("\\");
        Serial.print(i);
        delay(50);
    }
    delay(2000);
    Serial.print("\f");
  }
  delay(20000);
  s=0;
  i=0;*/
}
