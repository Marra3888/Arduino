//***************************************************************
// UTFT_Color_Alternate_Demo
//***************************************************************

#include <UTFT.h>

// Declare which fonts we will be using (optional, but can be used if needed)
extern uint8_t BigFont[];

// Set the pins to the correct ones for Arduino Mega
// ------------------------------------------------------------
// Standard Arduino Mega/Due shield: <display model>,38,39,40,41
// Remember to change the model parameter to suit your display module!
UTFT myGLCD(SSD1963_800ALT,38,39,40,41);  // For 480x272 display

void setup()
{
  // Setup the LCD
  myGLCD.InitLCD();
  myGLCD.setFont(BigFont);  // Optional: set font if you want to print text
  myGLCD.setBrightness(15);
}

void loop()
{
  // Clear the screen and fill with red
  myGLCD.fillScr(255, 0, 0);  // Red
  delay(10000);  // Wait 1 second

  // Fill with green
  myGLCD.fillScr(0, 255, 0);  // Green
  delay(10000);

  // Fill with blue
  myGLCD.fillScr(0, 0, 255);  // Blue
  delay(10000);

  // Fill with yellow
  myGLCD.fillScr(255, 255, 0);  // Yellow
  delay(10000);

  // Fill with cyan
  myGLCD.fillScr(0, 255, 255);  // Cyan
  delay(10000);

  // Fill with magenta
  myGLCD.fillScr(255, 0, 255);  // Magenta
  delay(10000);

  // Fill with white
  myGLCD.fillScr(255, 255, 255);  // White
  delay(10000);

  // Fill with black (to reset)
  myGLCD.fillScr(0, 0, 0);  // Black
  delay(10000);
}