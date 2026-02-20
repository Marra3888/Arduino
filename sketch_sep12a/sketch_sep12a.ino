#include <RTClib.h>
#include <Wire.h>
#include <Ticker.h>

// RTC DS3231
RTC_DS3231 rtc;

// MAX7219 пины
#define DIN_PIN D8
#define CS_PIN  D6
#define CLK_PIN D7
#define NUM_MAX 4 // 4 модуля 8x8 = 32x8

#define Russian // Включить русские шрифты
// #define ROTATE 90 // Раскомментируйте, если матрица повёрнута

// Буфер для дисплея
byte scr[NUM_MAX * 8 + 8]; // +8 для прокрутки

// MAX7219 команды
#define CMD_NOOP   0
#define CMD_DIGIT0 1
#define CMD_DIGIT1 2
#define CMD_DIGIT2 3
#define CMD_DIGIT3 4
#define CMD_DIGIT4 5
#define CMD_DIGIT5 6
#define CMD_DIGIT6 7
#define CMD_DIGIT7 8
#define CMD_DECODEMODE  9
#define CMD_INTENSITY   10
#define CMD_SCANLIMIT   11
#define CMD_SHUTDOWN    12
#define CMD_DISPLAYTEST 15

// Переменные для времени
struct MyDateTime {
  unsigned char sek1, sek2, min1, min2, hour1, hour2, tag1, tag2, mon1, mon2, year1, year2, WT;
} MEZ;

byte sec1_position = 27; // Позиции символов на дисплее
byte sec2_position = 23;
byte min1_position = 18;
byte min2_position = 13;
byte hour1_position = 4;
byte hour2_position = 1;
byte colon_position = 11;

unsigned char maxPosX = NUM_MAX * 8 - 1; // 31
unsigned char z_PosX = 0, d_PosX = -8;
bool f_tckr1s = false, f_tckr50ms = false, f_scroll_x = false;
static uint16_t len = 0;

// Тикеры для периодических обновлений
Ticker tckr1s, tckr50ms;

// Русские месяцы и дни недели
const uint8_t M_rus[13][9] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0}, // " ",
  {11, 23, 14, 12, 26, 32, 0xff, 0xff, 0xff}, // "Январь"
  {9, 17, 14, 26, 12, 21, 32, 0xff, 0xff}, // "Февраль"
  {4, 12, 26, 28, 12, 0xff, 0xff, 0xff, 0xff}, // "Март"
  {0, 25, 26, 17, 21, 32, 0xff, 0xff, 0xff}, // "Апрель"
  {4, 12, 32, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // "Май"
  {3, 33, 23, 32, 0xff, 0xff, 0xff, 0xff, 0xff}, // "Июнь"
  {3, 33, 21, 32, 0xff, 0xff, 0xff, 0xff, 0xff}, // "Июль"
  {0, 14, 15, 29, 27, 28, 12, 0xff, 0xff}, // "Август"
  {8, 17, 23, 28, 32, 13, 26, 32, 0xff}, // "Сентябрь"
  {6, 20, 28, 32, 13, 26, 32, 0xff, 0xff}, // "Октябрь"
  {5, 24, 32, 13, 26, 32, 0xff, 0xff, 0xff}, // "Ноябрь"
  {2, 17, 26, 12, 13, 26, 32, 0xff, 0xff} // "Декабрь"
};

const uint8_t WT_rus[8][12] = {
  {1, 24, 27, 20, 26, 17, 27, 17, 23, 31, 17, 0xff}, // "Воскресенье"
  {7, 24, 23, 17, 16, 17, 21, 31, 23, 18, 20, 0xff}, // "Понедельник"
  {1, 28, 24, 26, 23, 18, 20, 0xff, 0xff, 0xff, 0xff, 0xff}, // "Вторник"
  {8, 26, 17, 16, 12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // "Среда"
  {10, 17, 28, 14, 17, 26, 15, 0xff, 0xff, 0xff, 0xff, 0xff}, // "Четверг"
  {7, 32, 28, 23, 18, 30, 12, 0xff, 0xff, 0xff, 0xff, 0xff}, // "Пятница"
  {8, 29, 13, 13, 24, 28, 12, 0xff, 0xff, 0xff, 0xff, 0xff} // "Суббота"
};

// Шрифт 5x8 для ASCII и русских символов
const unsigned char font1[96][9] = {
  {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space
  {0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00}, // !
  {0x07, 0x09, 0x09, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00}, // "
  // ... (остальные ASCII символы из вашего кода, для краткости опущены)
  {0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00}, // :
  // ... (продолжение шрифта)
  {0x07, 0x0F, 0x09, 0x09, 0x09, 0x09, 0x09, 0x0F, 0x00}, // 0
  {0x07, 0x02, 0x06, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00}, // 1
  {0x07, 0x0F, 0x01, 0x01, 0x0F, 0x08, 0x08, 0x0F, 0x00}, // 2
  {0x07, 0x0F, 0x01, 0x01, 0x07, 0x01, 0x01, 0x0F, 0x00}, // 3
  {0x07, 0x09, 0x09, 0x09, 0x0F, 0x01, 0x01, 0x01, 0x00}, // 4
  {0x07, 0x0F, 0x08, 0x08, 0x0F, 0x01, 0x01, 0x0F, 0x00}, // 5
  {0x07, 0x0F, 0x08, 0x08, 0x0F, 0x09, 0x09, 0x0F, 0x00}, // 6
  {0x07, 0x0F, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00}, // 7
  {0x07, 0x0F, 0x09, 0x09, 0x0F, 0x09, 0x09, 0x0F, 0x00}, // 8
  {0x07, 0x0F, 0x09, 0x09, 0x0F, 0x01, 0x01, 0x0F, 0x00}, // 9
  // Русские символы (пример, замените на свои, если нужно)
  {0x07, 0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11, 0x00}, // А
  {0x07, 0x1E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E, 0x00}, // Б
  // ... (остальные русские символы по вашим массивам)
};

// Функции MAX7219
void sendCmd(int addr, byte cmd, byte data) {
  digitalWrite(CS_PIN, LOW);
  for (int i = NUM_MAX - 1; i >= 0; i--) {
    shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, i == addr ? cmd : 0);
    shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, i == addr ? data : 0);
  }
  digitalWrite(CS_PIN, HIGH);
}

void sendCmdAll(byte cmd, byte data) {
  digitalWrite(CS_PIN, LOW);
  for (int i = NUM_MAX - 1; i >= 0; i--) {
    shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, cmd);
    shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, data);
  }
  digitalWrite(CS_PIN, HIGH);
}

void refreshAll() {
#ifdef ROTATE
  #if ROTATE == 270
    byte mask = 0x01;
    for (int c = 0; c < 8; c++) {
      digitalWrite(CS_PIN, LOW);
      for (int i = 0; i < NUM_MAX; i++) {
        byte bt = 0;
        for (int b = 0; b < 8; b++) {
          bt <<= 1;
          if (scr[i * 8 + b] & mask) bt |= 0x01;
        }
        shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, CMD_DIGIT0 + c);
        shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, bt);
      }
      digitalWrite(CS_PIN, HIGH);
      mask <<= 1;
    }
  #elif ROTATE == 90
    byte mask = 0x80;
    for (int c = 0; c < 8; c++) {
      digitalWrite(CS_PIN, LOW);
      for (int i = NUM_MAX - 1; i >= 0; i--) {
        byte bt = 0;
        for (int b = 0; b < 8; b++) {
          bt >>= 1;
          if (scr[i * 8 + b] & mask) bt |= 0x80;
        }
        shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, CMD_DIGIT0 + c);
        shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, bt);
      }
      digitalWrite(CS_PIN, HIGH);
      mask >>= 1;
    }
  #endif
#else
  for (int c = 0; c < 8; c++) {
    digitalWrite(CS_PIN, LOW);
    for (int i = NUM_MAX - 1; i >= 0; i--) {
      shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, CMD_DIGIT0 + c);
      shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, scr[i * 8 + c]);
    }
    digitalWrite(CS_PIN, HIGH);
  }
#endif
}

void clr() {
  for (byte i = 0; i < NUM_MAX * 8; i++) scr[i] = 0;
}

void initMAX7219() {
  pinMode(DIN_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  sendCmdAll(CMD_DISPLAYTEST, 0); // Отключить тестовый режим
  sendCmdAll(CMD_SCANLIMIT, 7);   // Сканировать все 8 строк
  sendCmdAll(CMD_DECODEMODE, 0);  // Отключить декодирование
  sendCmdAll(CMD_INTENSITY, 7);   // Средняя яркость (0-15)
  sendCmdAll(CMD_SHUTDOWN, 1);    // Включить нормальный режим
  clr();
  refreshAll();
}

// Отрисовка символа
void char2Arr(char c, int pos, int y) {
  if (pos < -8 || pos >= NUM_MAX * 8) return;
  int idx = c - 32; // Смещение для font1 (ASCII с 32)
  if (idx < 0 || idx >= 96) return;
  for (int i = 0; i < 8; i++) {
    int p = pos + i;
    if (p >= 0 && p < NUM_MAX * 8) {
      scr[p] = (y == 0) ? font1[idx][i + 1] : 0;
    }
  }
}

void char2ArrRus(uint8_t c, int pos, int y) {
  if (c == 0xff || pos < -8 || pos >= NUM_MAX * 8) return;
  int idx = c - 32; // Предполагаем, что русские символы мапятся так же
  if (idx < 0 || idx >= 96) return;
  for (int i = 0; i < 8; i++) {
    int p = pos + i;
    if (p >= 0 && p < NUM_MAX * 8) {
      scr[p] = (y == 0) ? font1[idx][i + 1] : 0;
    }
  }
}

void refresh_display() {
  clr();
  refreshAll();
}

// Чтение времени из DS3231
void ReadTime() {
  DateTime now = rtc.now();
  MEZ.hour2 = now.hour() / 10;
  MEZ.hour1 = now.hour() % 10;
  MEZ.min2 = now.minute() / 10;
  MEZ.min1 = now.minute() % 10;
  MEZ.sek2 = now.second() / 10;
  MEZ.sek1 = now.second() % 10;
  MEZ.tag2 = now.day() / 10;
  MEZ.tag1 = now.day() % 10;
  MEZ.mon2 = now.month() / 10;
  MEZ.mon1 = now.month() % 10;
  MEZ.year2 = (now.year() % 100) / 10;
  MEZ.year1 = now.year() % 10;
  MEZ.WT = now.dayOfTheWeek(); // 0 = воскресенье, 1 = понедельник, ...
}

void ticker1s() { f_tckr1s = true; }
void ticker50ms() { f_tckr50ms = true; }

void setup() {
  Serial.begin(115200);
  Wire.begin();
  initMAX7219();
  if (!rtc.begin()) {
    Serial.println("RTC не найден!");
    // while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC потерял питание, устанавливаем время!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Установить время компиляции
  }
  tckr1s.attach(1.0, ticker1s);
  tckr50ms.attach_ms(50, ticker50ms);
}

void loop() {
  static uint8_t sek1 = 0, sek2 = 0, min1 = 0, min2 = 0, hour1 = 0, hour2 = 0;
  static uint8_t sek11 = 0, sek12 = 0, sek21 = 0, sek22 = 0;
  static uint8_t min11 = 0, min12 = 0, min21 = 0, min22 = 0;
  static uint8_t hour11 = 0, hour12 = 0, hour21 = 0, hour22 = 0;
  static int y = 0, y_1 = -8, y2 = 8, y3 = 0;
  static bool updown = true, f_scrollend_y = false;
  static uint8_t nomber1 = 0, nomber2 = 0, nomber3 = 0, nomber4 = 0, nomber5 = 0, nomber6 = 0;
  static int jj = 0;

  z_PosX = maxPosX; // 31
  d_PosX = -8;

  if (f_tckr1s) {
    ReadTime();
    sek1 = MEZ.sek1;
    sek2 = MEZ.sek2;
    min1 = MEZ.min1;
    min2 = MEZ.min2;
    hour1 = MEZ.hour1;
    hour2 = MEZ.hour2;
    y = y2;
    nomber1 = 1;
    sek1++;
    if (sek1 == 10) {
      nomber2 = 1;
      sek2++;
      sek1 = 0;
    }
    if (sek2 == 6) {
      min1++;
      sek2 = 0;
      nomber3 = 1;
    }
    if (min1 == 10) {
      min2++;
      min1 = 0;
      nomber4 = 1;
    }
    if (min2 == 6) {
      hour1++;
      min2 = 0;
      nomber5 = 1;
    }
    if (hour1 == 10) {
      hour2++;
      hour1 = 0;
      nomber6 = 1;
    }
    if (hour2 == 2 && hour1 == 4) {
      hour1 = 0;
      hour2 = 0;
      nomber6 = 1;
    }
    sek11 = sek12; sek12 = sek1;
    sek21 = sek22; sek22 = sek2;
    min11 = min12; min12 = min1;
    min21 = min22; min22 = min2;
    hour11 = hour12; hour12 = hour1;
    hour21 = hour22; hour22 = hour2;
    if (MEZ.sek1 == 0) f_scroll_x = true; // Прокрутка каждую минуту
    f_tckr1s = false;
  }

  if (f_tckr50ms) {
    f_tckr50ms = false;
    if (f_scroll_x) {
      z_PosX++;
      d_PosX++;
      if (d_PosX >= 183) z_PosX = 0;
      if (z_PosX == maxPosX) {
        f_scroll_x = false;
        d_PosX = -8;
      }
    }

    clr();
    // Отрисовка времени
    if (nomber1) {
      if (updown) y--;
      else y++;
      y3 = y;
      if (y3 > 0) y3 = 0;
      char2Arr(48 + sek12, z_PosX - sec1_position, y3);
      char2Arr(48 + sek11, z_PosX - sec1_position, y + y_1);
      if (y == 0) {
        nomber1 = 0;
        f_scrollend_y = true;
      }
    } else {
      char2Arr(48 + sek1, z_PosX - sec1_position, 0);
    }

    if (nomber2) {
      char2Arr(48 + sek22, z_PosX - sec2_position, y3);
      char2Arr(48 + sek21, z_PosX - sec2_position, y + y_1);
      if (y == 0) nomber2 = 0;
    } else {
      char2Arr(48 + sek2, z_PosX - sec2_position, 0);
    }

    if (nomber3) {
      char2Arr(48 + min12, z_PosX - min1_position, y);
      char2Arr(48 + min11, z_PosX - min1_position, y + y_1);
      if (y == 0) nomber3 = 0;
    } else {
      char2Arr(48 + min1, z_PosX - min1_position, 0);
    }

    if (nomber4) {
      char2Arr(48 + min22, z_PosX - min2_position, y);
      char2Arr(48 + min21, z_PosX - min2_position, y + y_1);
      if (y == 0) nomber4 = 0;
    } else {
      char2Arr(48 + min2, z_PosX - min2_position, 0);
    }

    if (nomber5) {
      char2Arr(48 + hour12, z_PosX - hour1_position, y);
      char2Arr(48 + hour11, z_PosX - hour1_position, y + y_1);
      if (y == 0) nomber5 = 0;
    } else {
      char2Arr(48 + hour1, z_PosX - hour1_position, 0);
    }

    if (nomber6) {
      char2Arr(48 + hour22, z_PosX + hour2_position, y);
      char2Arr(48 + hour21, z_PosX + hour2_position, y + y_1);
      if (y == 0) nomber6 = 0;
    } else {
      char2Arr(48 + hour2, z_PosX + hour2_position, 0);
    }

    char2Arr(':', z_PosX - colon_position, 0);

    // Отрисовка даты и дня недели
    #ifdef Russian
    len = 0;
    for (int i = 0; i < 12 && WT_rus[MEZ.WT][i] != 0xff; i++) {
      char2ArrRus(WT_rus[MEZ.WT][i], d_PosX - len, 0);
      len += 6;
    }
    char2Arr(48 + MEZ.tag2, d_PosX - len, 0);
    len += 6;
    char2Arr(48 + MEZ.tag1, d_PosX - len, 0);
    len += 6;
    char2Arr(' ', d_PosX - len, 0);
    len += 3;
    for (int i = 0; i < 9 && M_rus[MEZ.mon1 + MEZ.mon2 * 10][i] != 0xff; i++) {
      char2ArrRus(M_rus[MEZ.mon1 + MEZ.mon2 * 10][i], d_PosX - len, 0);
      len += 6;
    }
    char2Arr('2', d_PosX - len, 0);
    len += 6;
    char2Arr('0', d_PosX - len, 0);
    len += 6;
    char2Arr(48 + MEZ.year2, d_PosX - len, 0);
    len += 6;
    char2Arr(48 + MEZ.year1, d_PosX - len, 0);
    #endif

    refreshAll();
    if (f_scrollend_y) f_scrollend_y = false;
  }

  ESP.wdtFeed(); // Сбрасываем WDT
}