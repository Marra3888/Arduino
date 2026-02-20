// UTouch_ButtonTest (C)2010-2014 Henning Karlsen
// web: http://www.henningkarlsen.com/electronics
//
// This program is a quick demo of how create and use buttons.
//
// This program requires the UTFT library.
//
// It is assumed that the display module is connected to an
// appropriate shield or that you know how to change the pin 
// numbers in the setup.
//

#include <UTFT.h>
#include <UTouch.h>

// Initialize display
// ------------------
// Set the pins to the correct ones for your development board
// -----------------------------------------------------------
// Standard Arduino Uno/2009 Shield            : <display model>,19,18,17,16
// Standard Arduino Mega/Due shield            : <display model>,38,39,40,41
// CTE TFT LCD/SD Shield for Arduino Due       : <display model>,25,26,27,28
// Teensy 3.x TFT Test Board                   : <display model>,23,22, 3, 4
// ElecHouse TFT LCD/SD Shield for Arduino Due : <display model>,22,23,31,33
//
// Remember to change the model parameter to suit your display module!
UTFT    myGLCD(SSD1963_800ALT,38,39,40,41);  // For 4.3" 480x272 display

// Initialize touchscreen
// ----------------------
// Set the pins to the correct ones for your development board
// -----------------------------------------------------------
// Standard Arduino Uno/2009 Shield            : 15,10,14, 9, 8
// Standard Arduino Mega/Due shield            :  6, 5, 4, 3, 2
// CTE TFT LCD/SD Shield for Arduino Due       :  6, 5, 4, 3, 2
// Teensy 3.x TFT Test Board                   : 26,31,27,28,29
// ElecHouse TFT LCD/SD Shield for Arduino Due : 25,26,27,29,30
//
UTouch  myTouch( 6, 5, 4, 3, 2);

// Declare which fonts we will be using
extern uint8_t BigFont[];

int x, y;
char stCurrent[20]="";
int stCurrentLen=0;
char stLast[20]="";

/*************************
**   Custom functions   **
*************************/

void drawButtons()
{
// Draw the upper row of buttons (shifted left to x=20)
  for (x=0; x<5; x++)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (20+(x*90), 10, 100+(x*90), 60);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (20+(x*90), 10, 100+(x*90), 60);
    myGLCD.printNumI(x+1, 20+(x*90)+38, 27);  // Точно по центру: +38 для BigFont
  }
// Draw the center row of buttons (shifted left to x=20)
  for (x=0; x<4; x++)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (20+(x*90), 70, 100+(x*90), 120);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (20+(x*90), 70, 100+(x*90), 120);
    myGLCD.printNumI(x+6, 20+(x*90)+38, 87);  // Точно по центру: +38 для BigFont
  }
  // Draw button 0 separately (shifted left)
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (20+(4*90), 70, 100+(4*90), 120);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (20+(4*90), 70, 100+(4*90), 120);
  myGLCD.print("0", 20+(4*90)+38, 87);  // Точно по центру: +38 для BigFont
// Draw the lower row of buttons (Clear and Enter centered)
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (20, 130, 220, 180);  // Clear: ширина 200, слева
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (20, 130, 220, 180);
  myGLCD.print("Clear", 80, 147);  // Точно по центру кнопки Clear (учтена ширина текста)
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (260, 130, 460, 180);  // Enter: ширина 200, справа (по центру экрана)
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (260, 130, 460, 180);
  myGLCD.print("Enter", 320, 147);  // Точно по центру кнопки Enter (учтена ширина текста)
  myGLCD.setBackColor (0, 0, 0);
}

void updateStr(int val)
{
  if (stCurrentLen<20)
  {
    stCurrent[stCurrentLen]=val;
    stCurrent[stCurrentLen+1]='\0';
    stCurrentLen++;
    myGLCD.setColor(0, 255, 0);
    myGLCD.print(stCurrent, LEFT, 200);
  }
  else
  {
    myGLCD.setColor(255, 0, 0);
    myGLCD.print("BUFFER FULL!", CENTER, 160);
    delay(500);
    myGLCD.print("            ", CENTER, 160);
    delay(500);
    myGLCD.print("BUFFER FULL!", CENTER, 160);
    delay(500);
    myGLCD.print("            ", CENTER, 160);
    myGLCD.setColor(0, 255, 0);
  }
}

// Draw a red frame while a button is touched
void waitForIt(int x1, int y1, int x2, int y2)
{
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
  while (myTouch.dataAvailable())
    myTouch.read();
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
}

void Serial_Print(char number)
{
          Serial.print(number);
          Serial.print("  x = ");
          Serial.print(x);
          Serial.print("  y = ");
          Serial.println(y);
}
/*************************
**  Required functions  **
*************************/

void setup()
{
  Serial.begin(9600);
// Initial setup
  myGLCD.InitLCD();
  myGLCD.clrScr();

  myTouch.InitTouch();
  myTouch.setPrecision(PREC_EXTREME);

  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 255);
  drawButtons();  
}

void loop()
{
  while (true)
  {
    if (myTouch.dataAvailable())
    {
      myTouch.read();
      int rawX = myTouch.getX();  // Сырые координаты для отладки
      x = rawX + 50;  // Offset +50 для стабильности
      y = myTouch.getY();
      
      // Отладка: вывод сырых и скорректированных координат (включено для теста!)
      Serial.print("Pressed: Raw x="); Serial.print(rawX); 
      Serial.print(", Corrected x="); Serial.print(x); 
      Serial.print(", y="); Serial.println(y);
      
      if ((y>=10) && (y<=60))  // Upper row
      {
        if ((x>=20) && (x<=100))  // Button: 1 (shifted left)
        {
          waitForIt(20, 10, 100, 60);
          updateStr('1');
          Serial_Print('1');
        }
        else if ((x>=110) && (x<=190))  // Button: 2
        {
          waitForIt(110, 10, 190, 60);
          updateStr('2');
          Serial_Print('2');
        }
        else if ((x>=200) && (x<=280))  // Button: 3
        {
          waitForIt(200, 10, 280, 60);
          updateStr('3');
          Serial_Print('3');
        }
        else if ((x>=290) && (x<=370))  // Button: 4
        {
          waitForIt(290, 10, 370, 60);
          updateStr('4');
          Serial_Print('4');
        }
        else if ((x>=380) && (x<=460))  // Button: 5
        {
          waitForIt(380, 10, 460, 60);
          updateStr('5');
          Serial_Print('5');
        }
      }

      if ((y>=70) && (y<=120))  // Center row
      {
        if ((x>=20) && (x<=100))  // Button: 6 (shifted left)
        {
          waitForIt(20, 70, 100, 120);
          updateStr('6');
          Serial_Print('6');
        }
        else if ((x>=110) && (x<=190))  // Button: 7
        {
          waitForIt(110, 70, 190, 120);
          updateStr('7');
          Serial_Print('7');
        }
        else if ((x>=200) && (x<=280))  // Button: 8
        {
          waitForIt(200, 70, 280, 120);
          updateStr('8');
          Serial_Print('8');
        }
        else if ((x>=290) && (x<=370))  // Button: 9
        {
          waitForIt(290, 70, 370, 120);
          updateStr('9');
          Serial_Print('9');
        }
        else if ((x>=380) && (x<=460))  // Button: 0
        {
          waitForIt(380, 70, 460, 120);
          updateStr('0');
          Serial_Print('0');
        }
      }

      if ((y>=130) && (y<=180))  // Lower row
      {
        if ((x>=20) && (x<=220))  // Button: Clear (обновлено под новую ширину)
        {
          waitForIt(20, 130, 220, 180);
          stCurrent[0]='\0';
          stCurrentLen=0;
          myGLCD.setColor(0, 0, 0);
          myGLCD.fillRect(0, 200, 479, 215);
        }
        else if ((x>=260) && (x<=460))  // Button: Enter (обновлено под новую ширину)
        {
          waitForIt(260, 130, 460, 180);
          if (stCurrentLen>0)
          {
            for (int i=0; i<stCurrentLen+1; i++)
            {
              stLast[i]=stCurrent[i];
            }
            stCurrent[0]='\0';
            stCurrentLen=0;
            myGLCD.setColor(0, 0, 0);
            myGLCD.fillRect(0, 180, 479, 195);
            myGLCD.setColor(0, 255, 0);
            myGLCD.print(stLast, LEFT, 180);
          }
          else
          {
            myGLCD.setColor(255, 0, 0);
            myGLCD.print("BUFFER EMPTY", CENTER, 160);
            delay(500);
            myGLCD.print("            ", CENTER, 160);
            delay(500);
            myGLCD.print("BUFFER EMPTY", CENTER, 160);
            delay(500);
            myGLCD.print("            ", CENTER, 160);
            myGLCD.setColor(0, 255, 0);
          }
        }
      }
    }
  }
}