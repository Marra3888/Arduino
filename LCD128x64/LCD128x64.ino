#include <Arduino.h>
#include <U8g2lib.h>

// Пины Nano:
#define PIN_CS   10   // CN1: линия через AHCT14 + 33 Ом → CS
#define PIN_DC   9    // CN1: линия через AHCT14 + 33 Ом → A0(DC)
#define PIN_RST  8    // CN1: линия через AHCT14 + RC → RST
// SCK = 13 (аппаратный), MOSI = 11 (аппаратный)

// Вариант 1: ST7565 (чаще встречается на 5 В)
// U8G2_ST7565_ERC12864_F_HW_SPI u8g2(U8G2_R0, /*cs=*/PIN_CS, /*dc=*/PIN_DC, /*reset=*/PIN_RST);
// U8G2_ST7565_ERC12864_F_4W_HW_SPI u8g2(U8G2_R0, PIN_CS, PIN_DC, PIN_RST);

// Вариант 2: если не заработал — попробуйте UC1701
// U8G2_UC1701_MINI12864_F_HW_SPI u8g2(U8G2_R0, /*cs=*/PIN_CS, /*dc=*/PIN_DC, /*reset=*/PIN_RST);
U8G2_UC1701_MINI12864_F_4W_HW_SPI u8g2(U8G2_R0, PIN_CS, PIN_DC, PIN_RST);

void setup() {
  u8g2.begin();
  u8g2.setContrast(200);    // подберите 150..240
  // u8g2.setFlipMode(1);   // если ориентация неверная
}

void loop() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 12, "Hello 128x64");
  u8g2.drawBox(0, 16, 60, 16);
  u8g2.sendBuffer();
  delay(500);
}