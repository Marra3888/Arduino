#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

#define NUM_MAX 4
#define DIN_PIN D8 // 15
#define CS_PIN  D6 // 12
#define CLK_PIN D7 // 13
#define SCROLL_END_POS 183

#define CMD_NOOP        0
#define CMD_DIGIT0      1
#define CMD_DECODEMODE  9
#define CMD_INTENSITY   10
#define CMD_SCANLIMIT   11
#define CMD_SHUTDOWN    12
#define CMD_DISPLAYTEST 15

#define REVERSE_VERTICAL

struct DateTimeStruct {
  uint8_t seconds, minutes, hours;
  uint8_t day, month, weekday;
  uint16_t year;
};

void displayInit();
void displayClear();
void displaySetBrightness(uint8_t level);
void displayChar(uint8_t ch, int8_t posX, int8_t posY, bool useFont2 = false);
void displayCharRus(uint8_t ch, int8_t posX, int8_t posY);
void displayRefresh();
void renderDisplay(const DateTimeStruct& time, bool scroll, uint8_t scrollPos, char timeSource, bool blinkState);
void updateDisplayTime(const DateTimeStruct& time);
void sendCmdAll(uint8_t cmd, uint8_t data);

extern uint8_t ledArray[NUM_MAX][8];
extern const uint8_t font1[96][9];
extern const uint8_t font2[96][9];
extern const uint8_t rusFont[34][9];
extern const uint8_t monthsRus[13][9];
extern const uint8_t weekdaysRus[8][12];

extern volatile bool tickerFlag50ms;
extern volatile bool tickerFlag1s;
extern bool scrollEnabled;
extern uint8_t scrollPosX;
extern char timeSource;
extern bool blinkState;

#endif