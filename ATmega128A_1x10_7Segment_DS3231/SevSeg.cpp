/* SevSeg Library

  Copyright 2017 Dean Reading

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.


  This library allows an Arduino to easily display numbers in decimal format on
  a 7-segment display without a separate 7-segment display controller.

  Direct any questions or suggestions to deanreading@hotmail.com
  See the included readme for instructions.
  https://github.com/DeanIsMe/SevSeg

  CHANGELOG
  Version 3.3.0 - February 2017
  Added the ability to keep leading zeros. This is now an extra
  parameter in the begin() function.
  
  Version 3.2.0 - December 2016
  Updated to Arduino 1.5 Library Specification
  New display function - no longer consumes processor time with delay()
  Now supports hexadecimal number printing
  The decimal point can now be omitted with a negative decPlaces
  decPlaces is now optional in setNumber
  Alphanumeric strings can be displayed (inaccurately) with setChars()
  Removed #define RESISTORS_ON_SEGMENTS. Now a begin() input
  Can now blank() the display

  Version 3.1 - September 2016
  Bug Fixes. No longer uses dynamic memory allocation.
  Version 3.0 - November 2014
  Library re-design. A display with any number of digits can be used.
  Floats are supported. Support for using transistors for switching.
  Much more user friendly. No backwards compatibility.
  Uploaded to GitHub to simplify any further development.
  Version 2.3; Allows for brightness control.
  Version 2.2; Allows 1, 2 or 3 digit displays to be used.
  Version 2.1; Includes a bug fix.
  Version 2.0; Now works for any digital pin arrangement.
  Supports both common anode and common cathode displays.
*/

#include "SevSeg.h"

#define BLANK_IDX 36 // Must match with 'digitCodeMap'
#define DASH_IDX 37
#define BBLANK_IDX 38

static const long powersOf10[] = {
  1, // 10^0
  10,
  100,
  1000,
  10000,
  100000,
  1000000,
  10000000,
  100000000,
  1000000000
}; // 10^9

static const long powersOf16[] = {
  0x1, // 16^0
  0x10,
  0x100,
  0x1000,
  0x10000,
  0x100000,
  0x1000000,
  0x10000000
}; // 16^7

// Приведенные ниже коды указывают, какие сегменты должны подсвечиваться для отображения каждого номера.
static const byte digitCodeMap[] = {
  //     GFEDCBA  Segments      7-segment map:
  B00111111, // 0   "0"          AAA
  B00000110, // 1   "1"         F   B
  B01011011, // 2   "2"         F   B
  B01001111, // 3   "3"          GGG
  B01100110, // 4   "4"         E   C
  B01101101, // 5   "5"         E   C
  B01111101, // 6   "6"          DDD
  B00000111, // 7   "7"
  B01111111, // 8   "8"
  B01101111, // 9   "9"
  B01110111, // 65  'A'
  B01111100, // 66  'b'
  B00111001, // 67  'C'
  B01011110, // 68  'd'
  B01111001, // 69  'E'
  B01110001, // 70  'F'
  B00111101, // 71  'G'
  B01110110, // 72  'H'
  B00000110, // 73  'I'
  B00001110, // 74  'J'
  B01110110, // 75  'K'  Same as 'H'
  B00111000, // 76  'L'
  B00000000, // 77  'M'  NO DISPLAY
  B01010100, // 78  'n'
  B00111111, // 79  'O'
  B01110011, // 80  'P'
  B01100111, // 81  'q'
  B01010000, // 82  'r'
  B01101101, // 83  'S'
  B01111000, // 84  't'
  B00111110, // 85  'U'
  B00111110, // 86  'V'  Same as 'U'
  B00000000, // 87  'W'  NO DISPLAY
  B01110110, // 88  'X'  Same as 'H'
  B01101110, // 89  'y'
  B01011011, // 90  'Z'  Same as '2'
  B00000000, // 32  ' '  BLANK
  B01000000, // 45  '-'  DASH
  B00001001, 
};

// Constant pointers to constant data
const byte * const numeralCodes = digitCodeMap;
const byte * const alphaCodes = digitCodeMap + 10;

// SevSeg Constructor
/******************************************************************************/
SevSeg::SevSeg()
{
  // Initial value
  ledOnTime = 1000; // Corresponds to a brightness of 100
  numDigits = 0;
  prevUpdateIdx = 0;
  prevUpdateTime = 0;
  resOnSegments = 0;
  updateWithDelays = 0;
}


// begin
/******************************************************************************/
// Saves the input pin numbers to the class and sets up the pins to be used.
// If you use current-limiting resistors on your segment pins instead of the
// digit pins, then set resOnSegments as true.
// Set updateWithDelays to true if you want to use the 'pre-2017' update method
// That method occupies the processor with delay functions.
void SevSeg::begin(byte hardwareConfig, byte numDigitsIn, byte digitPinsIn[],
                   byte segmentPinsIn[], bool resOnSegmentsIn,
                   bool updateWithDelaysIn, bool leadingZerosIn) 
{

  resOnSegments = resOnSegmentsIn;
  updateWithDelays = updateWithDelaysIn;
  leadingZeros = leadingZerosIn;

  numDigits = numDigitsIn;
  //Ограничьте максимальное количество цифр, чтобы предотвратить переполнение
  if (numDigits > MAXNUMDIGITS) numDigits = MAXNUMDIGITS;

  switch (hardwareConfig) 
  {

    case 0: // Common cathode
      digitOn = LOW;
      segmentOn = HIGH;
      break;

    case 1: // Common anode
      digitOn = HIGH;
      segmentOn = LOW;
      break;

    case 2: // С активными высокими и низкими боковыми выключателями (чаще всего полевые транзисторы N-типа)
      digitOn = HIGH;
      segmentOn = HIGH;
      break;

    case 3: // С активными переключателями низкого и высокого уровня (чаще всего полевые транзисторы P-типа)
      digitOn = LOW;
      segmentOn = LOW;
      break;
  }

  digitOff = !digitOn;
  segmentOff = !segmentOn;

  // Сохранить номера входных контактов в библиотечных переменных
  for (byte segmentNum = 0 ; segmentNum < 8 ; segmentNum++) 
  {
    segmentPins[segmentNum] = segmentPinsIn[segmentNum];
  }

  for (byte digitNum = 0 ; digitNum < numDigits ; digitNum++) 
  {
    digitPins[digitNum] = digitPinsIn[digitNum];
  }

  // Установите контакты как выходы и отключите их
  for (byte digit = 0 ; digit < numDigits ; digit++) 
  {
    pinMode(digitPins[digit], OUTPUT);
    digitalWrite(digitPins[digit], digitOff);
  }

  for (byte segmentNum = 0 ; segmentNum < 8 ; segmentNum++) 
  {
    pinMode(segmentPins[segmentNum], OUTPUT);
    digitalWrite(segmentPins[segmentNum], segmentOff);
  }

  setNewNum(0, 0); // Инициализируйте отображаемое число на 0
}


// refreshDisplay
/******************************************************************************/
// Включает сегменты, указанные в digitCodes []
// Существует 4 версии этой функции, выбор которых зависит от расположения резисторов с ограничением тока и от того,
// хотите ли вы использовать «update delays задержки обновления» (стандартный метод до 2017 года).
// Для резисторов на * digits цифрах * мы будем циклически проходить все 8 сегментов (7 + период), включая * digits цифры * в зависимости от конкретного сегмента,
// прежде чем перейти к следующему сегменту.
// Для резисторов на * segments сегментах * мы будем циклически проходить все __ # цифры,
// включите * segments сегменты * в соответствии с данной цифрой, прежде чем переходить к следующей цифре.
// При использовании updateDelays задержек обновления, refreshDisplay имеет задержку между каждым сегментом цифры, поскольку это циклически проходит.
// Выход со всеми светодиодами выключен.
// Если не использовать updateDelays, refreshDisplay завершается с одной цифрой / сегментом.
// После повторного вызова он перейдет к следующей цифре / сегменту (если прошло достаточно времени).

void SevSeg::refreshDisplay() 
{

  if (!updateWithDelays) 
  {

			// Выход, если не пришло время для следующего изменения дисплея
			if (micros() - prevUpdateTime < ledOnTime) return;
			prevUpdateTime = micros();

			if (!resOnSegments) 
			{
			  /**********************************************/
			  // РЕЗИСТОРЫ НА ЦИФРАХ, ОБНОВЛЕНИЕ БЕЗ ЗАДЕРЖКИ

			  // Выключить все огни для предыдущего сегмента
			  for (byte digitNum = 0 ; digitNum < numDigits ; digitNum++) 
			  {
				digitalWrite(digitPins[digitNum], digitOff);
			  }
			  digitalWrite(segmentPins[prevUpdateIdx], segmentOff);

			  prevUpdateIdx++;
			  if (prevUpdateIdx >= 8) prevUpdateIdx = 0;

			  byte segmentNum = prevUpdateIdx;

			  // Подсветите необходимые цифры для нового сегмента
			  digitalWrite(segmentPins[segmentNum], segmentOn);
			  for (byte digitNum = 0 ; digitNum < numDigits ; digitNum++) 
			  {
				if (digitCodes[digitNum] & (1 << segmentNum)) 
				{ // Проверьте один бит
				  digitalWrite(digitPins[digitNum], digitOn);
				}
			  }
			}
			else 
			{
			  /**********************************************/
			  // РЕЗИСТОРЫ НА СЕГМЕНТАХ, ОБНОВЛЕНИЕ БЕЗ ЗАДЕРЖКИ


			  // Выключить все огни для предыдущей цифры
			  for (byte segmentNum = 0 ; segmentNum < 8 ; segmentNum++) 
			  {
				digitalWrite(segmentPins[segmentNum], segmentOff);
			  }
			  digitalWrite(digitPins[prevUpdateIdx], digitOff);

			  prevUpdateIdx++;
			  if (prevUpdateIdx >= numDigits) prevUpdateIdx = 0;

			  byte digitNum = prevUpdateIdx;

			  // Подсветите необходимые сегменты для новой цифры
			  digitalWrite(digitPins[digitNum], digitOn);
			  for (byte segmentNum = 0 ; segmentNum < 8 ; segmentNum++) 
			  {
				if (digitCodes[digitNum] & (1 << segmentNum)) 
				{ // Проверьте один бит
				  digitalWrite(segmentPins[segmentNum], segmentOn);
				}
			  }
			}
  }

  else 
  {
				if (!resOnSegments) 
				{
					  /**********************************************/
					  // РЕЗИСТОРЫ НА ЦИФРАХ, ОБНОВЛЕНИЕ С ЗАДЕРЖКАМИ
					  for (byte segmentNum = 0 ; segmentNum < 8 ; segmentNum++) 
					  {

						// Подсветите необходимые цифры для этого сегмента
						digitalWrite(segmentPins[segmentNum], segmentOn);
						for (byte digitNum = 0 ; digitNum < numDigits ; digitNum++) 
						{
						  if (digitCodes[digitNum] & (1 << segmentNum)) 
						  { // Проверьте один бит
							digitalWrite(digitPins[digitNum], digitOn);
						  }
						}

						//Ждите с включенными огнями (чтобы увеличить яркость)
						delayMicroseconds(ledOnTime);

						//Выключить все огни
						for (byte digitNum = 0 ; digitNum < numDigits ; digitNum++) 
						{
						  digitalWrite(digitPins[digitNum], digitOff);
						}
						digitalWrite(segmentPins[segmentNum], segmentOff);
					  }
				}
				else 
				{
				  /**********************************************/
				  // РЕЗИСТОРЫ НА СЕГМЕНТАХ, ОБНОВЛЕНИЕ С ЗАДЕРЖКАМИ
				  for (byte digitNum = 0 ; digitNum < numDigits ; digitNum++) 
				  {

					// Подсветите необходимые сегменты для этой цифры
					digitalWrite(digitPins[digitNum], digitOn);
					for (byte segmentNum = 0 ; segmentNum < 8 ; segmentNum++) 
					{
					  if (digitCodes[digitNum] & (1 << segmentNum)) 
					  { // Проверьте один бит
						digitalWrite(segmentPins[segmentNum], segmentOn);
					  }
					}

					//Ждите с включенными огнями (чтобы увеличить яркость)
					delayMicroseconds(ledOnTime);

					//Выключить все огни
					for (byte segmentNum = 0 ; segmentNum < 8 ; segmentNum++) 
					{
					  digitalWrite(segmentPins[segmentNum], segmentOff);
					}
					digitalWrite(digitPins[digitNum], digitOff);
				  }
				}
			  }
}

// setBrightness
/******************************************************************************/

void SevSeg::setBrightness(int brightness) {
  brightness = constrain(brightness, 0, 100);
  ledOnTime = map(brightness, 0, 100, 1, 2000);
}


// setNumber
/******************************************************************************/
// This function only receives the input and passes it to 'setNewNum'.
// It is overloaded for all number data types, so that floats can be handled
// correctly.

void SevSeg::setNumber(long numToShow, char decPlaces, bool hex) //long
{
  setNewNum(numToShow, decPlaces, hex);
}

void SevSeg::setNumber(unsigned long numToShow, char decPlaces, bool hex) //unsigned long
{
  setNewNum(numToShow, decPlaces, hex);
}

void SevSeg::setNumber(int numToShow, char decPlaces, bool hex) //int
{
  setNewNum(numToShow, decPlaces, hex);
}

void SevSeg::setNumber(unsigned int numToShow, char decPlaces, bool hex) //unsigned int
{
  setNewNum(numToShow, decPlaces, hex);
}

void SevSeg::setNumber(char numToShow, char decPlaces, bool hex) //char
{
  setNewNum(numToShow, decPlaces, hex);
}

void SevSeg::setNumber(byte numToShow, char decPlaces, bool hex) //byte
{
  setNewNum(numToShow, decPlaces, hex);
}

void SevSeg::setNumber(float numToShow, char decPlaces, bool hex) //float
{
  char decPlacesPos = constrain(decPlaces, 0, MAXNUMDIGITS);
  if (hex) {
    numToShow = numToShow * powersOf16[decPlacesPos];
  }
  else {
    numToShow = numToShow * powersOf10[decPlacesPos];
  }
  // Modify the number so that it is rounded to an integer correctly
  numToShow += (numToShow >= 0) ? 0.5f : -0.5f;
  setNewNum(numToShow, decPlaces, hex);
}


// setNewNum
/******************************************************************************/
// Changes the number that will be displayed.

void SevSeg::setNewNum(long numToShow, char decPlaces, bool hex) {
  byte digits[numDigits];
  findDigits(numToShow, decPlaces, hex, digits);
  setDigitCodes(digits, decPlaces);
}


// setSegments
/******************************************************************************/
// Sets the 'digitCodes' that are required to display the desired segments.
// Using this function, one can display any arbitrary set of segments (like
// letters, symbols or animated cursors). See setDigitCodes() for common
// numeric examples.
//
// Bit-segment mapping:  0bHGFEDCBA
//      Visual mapping:
//                        AAAA          0000
//                       F    B        5    1
//                       F    B        5    1
//                        GGGG          6666
//                       E    C        4    2
//                       E    C        4    2        (Segment H is often called
//                        DDDD  H       3333  7      DP, for Decimal Point)

void SevSeg::setSegments(byte segs[])
{
  for (byte digit = 0; digit < numDigits; digit++) {
    digitCodes[digit] = segs[digit];
  }
}

// setChars
/******************************************************************************/
// Displays the string on the display, as best as possible.
// Only alphanumeric characters plus '-' and ' ' are supported
void SevSeg::setChars(char str[], char decPlaces=-1)
{
  for (byte digit = 0; digit < numDigits; digit++) 
  {
    digitCodes[digit] = 0;
  }

  for (byte digitNum = 0; digitNum < numDigits; digitNum++) 
  {
    char ch = str[digitNum];
    if (ch == '\0') return; // NULL string terminator
    if (ch >= '0' && ch <= '9') 
    { // Numerical
      digitCodes[digitNum] = numeralCodes[ch - '0'];
    }
    else if (ch >= 'A' && ch <= 'Z') 
    {
      digitCodes[digitNum] = alphaCodes[ch - 'A'];
    }
    else if (ch >= 'a' && ch <= 'z') 
    {
      digitCodes[digitNum] = alphaCodes[ch - 'a'];
    }
    else if (ch == ' ') 
    {
      digitCodes[digitNum] = digitCodeMap[BLANK_IDX];
    }
    else 
    {
      // Every unknown character is shown as a dash
      digitCodes[digitNum] = digitCodeMap[DASH_IDX];
// digitCodes[digitNum] = digitCodeMap[ch];
    }
    if(ch == '&')  digitCodes[digitNum] = digitCodeMap[BBLANK_IDX];
    
    if (decPlaces >= 0) 
    {
      if (digitNum == numDigits - 1 - decPlaces) 
      {
        digitCodes[digitNum] |= B10000000;
      }
    }
  }
}

// blank
/******************************************************************************/
void SevSeg::blank(void) {
  for (byte digitNum = 0 ; digitNum < numDigits ; digitNum++) {
    digitCodes[digitNum] = digitCodeMap[BLANK_IDX];
  }
  refreshDisplay();
}

// findDigits
/******************************************************************************/
// Decides what each digit will display.
// Enforces the upper and lower limits on the number to be displayed.

void SevSeg::findDigits(long numToShow, char decPlaces, bool hex, byte digits[]) {
  const long * powersOfBase = hex ? powersOf16 : powersOf10;
  const long maxNum = powersOfBase[numDigits] - 1;
  const long minNum = -(powersOfBase[numDigits - 1] - 1);

  // If the number is out of range, just display dashes
  if (numToShow > maxNum || numToShow < minNum) {
    for (byte digitNum = 0 ; digitNum < numDigits ; digitNum++) {
      digits[digitNum] = DASH_IDX;
    }
  }
  else {
    byte digitNum = 0;

    // Convert all number to positive values
    if (numToShow < 0) {
      digits[0] = DASH_IDX;
      digitNum = 1; // Skip the first iteration
      numToShow = -numToShow;
    }

    // Find all digits for base's representation, starting with the most
    // significant digit
    for ( ; digitNum < numDigits ; digitNum++) {
      long factor = powersOfBase[numDigits - 1 - digitNum];
      digits[digitNum] = numToShow / factor;
      numToShow -= digits[digitNum] * factor;
    }

    // Find unnnecessary leading zeros and set them to BLANK
    if (decPlaces < 0) decPlaces = 0;
    if (!leadingZeros) {
      for (digitNum = 0 ; digitNum < (numDigits - 1 - decPlaces) ; digitNum++) {
        if (digits[digitNum] == 0) {
          digits[digitNum] = BLANK_IDX;
        }
        // Exit once the first non-zero number is encountered
        else if (digits[digitNum] <= 9) {
          break;
        }
      }
    }

  }
}


// setDigitCodes
/******************************************************************************/
// Sets the 'digitCodes' that are required to display the input numbers

void SevSeg::setDigitCodes(byte digits[], char decPlaces) {

  // Set the digitCode for each digit in the display
  for (byte digitNum = 0 ; digitNum < numDigits ; digitNum++) {
    digitCodes[digitNum] = digitCodeMap[digits[digitNum]];
    // Set the decimal place segment
    if (decPlaces >= 0) {
      if (digitNum == numDigits - 1 - decPlaces) {
        digitCodes[digitNum] |= B10000000;
      }
    }
  }
}

/// END ///
