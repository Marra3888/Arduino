#include "display.h"
#include "fonts.h"

uint8_t ledArray[NUM_MAX][8];

void displayInit() {
  pinMode(DIN_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);

  digitalWrite(CS_PIN, HIGH);
  sendCmdAll(CMD_SHUTDOWN, 1);    // Включить дисплей
  sendCmdAll(CMD_SCANLIMIT, 7);   // Отображать все 8 строк
  sendCmdAll(CMD_DECODEMODE, 0);  // Без декодирования (raw mode)
  sendCmdAll(CMD_INTENSITY, 15);  // Максимальная яркость
  displayClear();
}

void displayClear() {
  for (uint8_t i = 0; i < NUM_MAX; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      ledArray[i][j] = 0;
    }
  }
  displayRefresh();
}

void displaySetBrightness(uint8_t level) {
  if (level > 15) level = 15;
  sendCmdAll(CMD_INTENSITY, level);
}

void displayChar(uint8_t ch, int8_t posX, int8_t posY, bool useFont2) {
  if (ch < 32 || ch > 127) return; // Проверка на допустимый символ
  uint8_t charIdx = ch - 32;
  const uint8_t* font = useFont2 ? font2[charIdx] : font1[charIdx];

  for (uint8_t i = 0; i < 8; i++) {
    int8_t arrayX = posX / 8;
    int8_t shiftX = posX % 8;
    if (arrayX >= 0 && arrayX < NUM_MAX) {
      // Поворот на 180°: инвертируем строки (7-i) и биты
      uint8_t originalByte = pgm_read_byte(&font[1 + (7 - i)]); // +1 из-за ширины
      uint8_t rotatedByte = 0;
      for (uint8_t k = 0; k < 8; k++) {
        if (originalByte & (1 << k)) {
          rotatedByte |= (1 << (7 - k)); // Инверсия битов
        }
      }
      if (shiftX >= 0) {
        ledArray[arrayX][i] |= (rotatedByte >> shiftX);
      }
      if (shiftX < 0 && arrayX > 0) {
        ledArray[arrayX - 1][i] |= (rotatedByte << -shiftX);
      }
    }
  }
}

void displayCharRus(uint8_t ch, int8_t posX, int8_t posY) {
  uint8_t charIdx = 0;
  if (ch >= 'А' && ch <= 'Я') charIdx = ch - 'А';
  else if (ch >= 'а' && ch <= 'я') charIdx = ch - 'а';
  else return;

  for (uint8_t i = 0; i < 8; i++) {
    int8_t arrayX = posX / 8;
    int8_t shiftX = posX % 8;
    if (arrayX >= 0 && arrayX < NUM_MAX) {
      // Поворот на 180°: инвертируем строки (7-i) и биты
      uint8_t originalByte = pgm_read_byte(&rusFont[charIdx][1 + (7 - i)]);
      uint8_t rotatedByte = 0;
      for (uint8_t k = 0; k < 8; k++) {
        if (originalByte & (1 << k)) {
          rotatedByte |= (1 << (7 - k)); // Инверсия битов
        }
      }
      if (shiftX >= 0) {
        ledArray[arrayX][i] |= (rotatedByte >> shiftX);
      }
      if (shiftX < 0 && arrayX > 0) {
        ledArray[arrayX - 1][i] |= (rotatedByte << -shiftX);
      }
    }
  }
}

void displayRefresh() {
  Serial.println("Refreshing display...");
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(CS_PIN, LOW);
#ifdef REVERSE_VERTICAL
    for (uint8_t j = 0; j < NUM_MAX; j++) { // Слева направо
      shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, CMD_DIGIT0 + i);
      shiftOut(DIN_PIN, CLK_PIN, LSBFIRST, ledArray[j][i]);
      Serial.print("Row "); Serial.print(i); Serial.print(", Module "); Serial.print(j);
      Serial.print(": "); Serial.println(ledArray[j][i], BIN);
    }
#else
    for (uint8_t j = NUM_MAX; j > 0; j--) { // Справа налево
      shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, CMD_DIGIT0 + i);
      shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, ledArray[j - 1][i]);
      Serial.print("Row "); Serial.print(i); Serial.print(", Module "); Serial.print(j - 1);
      Serial.print(": "); Serial.println(ledArray[j - 1][i], BIN);
    }
#endif
    digitalWrite(CS_PIN, HIGH);
  }
  Serial.println("Display refresh completed");
}

void renderDisplay(const DateTimeStruct& time, bool scroll, uint8_t scrollPos, char timeSource, bool blinkState) {
  displayClear();

  Serial.print("Rendering time: ");
  Serial.print(time.hours); Serial.print(":");
  Serial.print(time.minutes); Serial.print(":");
  Serial.println(time.seconds);

  // Отображаем время HH:MM:SS
  displayChar('0' + (time.hours / 10), 0, 0);
  displayChar('0' + (time.hours % 10), 6, 0);
  displayChar(':', 12, 0);
  displayChar('0' + (time.minutes / 10), 18, 0);
  displayChar('0' + (time.minutes % 10), 24, 0);
  displayChar('0' + (time.seconds / 10), 30, 0);
  displayChar('0' + (time.seconds % 10), 36, 0);

  // Отображаем источник времени
  if (!scroll && blinkState) {
    displayChar(timeSource, 42, 0);
  }

  // Прокрутка даты и дня недели
  if (scroll) {
    int16_t datePos = -scrollPos;
    for (uint8_t i = 0; i < 12 && pgm_read_byte(&weekdaysRus[time.weekday][i + 1]) != 0xFF; i++) {
      displayCharRus(pgm_read_byte(&weekdaysRus[time.weekday][i + 1]), datePos, 0);
      datePos -= 6;
    }
    displayChar('0' + (time.day / 10), datePos, 0);
    datePos -= 6;
    displayChar('0' + (time.day % 10), datePos, 0);
    datePos -= 6;
    for (uint8_t i = 0; i < 9 && pgm_read_byte(&monthsRus[time.month][i + 1]) != 0xFF; i++) {
      displayCharRus(pgm_read_byte(&monthsRus[time.month][i + 1]), datePos, 0);
      datePos -= 6;
    }
    String yearStr = String(time.year);
    for (uint8_t i = 0; i < 4; i++) {
      displayChar(yearStr[i], datePos, 0);
      datePos -= 6;
    }
  }

  displayRefresh();
}

void updateDisplayTime(const DateTimeStruct& time) {
  renderDisplay(time, scrollEnabled, scrollPosX, timeSource, blinkState);
}

void sendCmdAll(uint8_t cmd, uint8_t data) {
  digitalWrite(CS_PIN, LOW);
  for (uint8_t i = 0; i < NUM_MAX; i++) {
    shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, cmd);
    shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, data);
  }
  digitalWrite(CS_PIN, HIGH);
}