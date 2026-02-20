#include <Wire.h>
#include <RTClib.h>
#include "TM1628.h"
#include <Ticker.h>
#include <ESP8266WiFi.h>   // для ESP8266 WiFi
#include <NTPClient.h>
#include <WiFiUdp.h>

// TM1628: DIO=D6, CLK=D5, STB=D7  (ESP8266)
TM1628 dvdLED(D6, D5, D7, 5);
RTC_DS3231 rtc;

// ========== ШРИФТ ==========
const uint8_t PROGMEM NUMBER_FONT[] = {
  0b01111110, // 0
  0b01010000, // 1
  0b10111100, // 2
  0b11110100, // 3
  0b11010010, // 4
  0b11100110, // 5
  0b11101110, // 6
  0b01010100, // 7
  0b11111110, // 8
  0b11110110, // 9
  0b11011110, // A
  0b11101010, // b
  0b00101110, // C
  0b11111000, // d
  0b10101110, // E
  0b10001110  // F
};





inline uint8_t segPattern(uint8_t d){ return pgm_read_byte(&NUMBER_FONT[d & 0x0F]); }

// ========== МАСКИ СЕГМЕНТОВ ==========
const uint8_t SEG_A = (1 << 2);
const uint8_t SEG_B = (1 << 4);
const uint8_t SEG_C = (1 << 6);
const uint8_t SEG_D = (1 << 5);
const uint8_t SEG_E = (1 << 3);
const uint8_t SEG_F = (1 << 1);
const uint8_t SEG_G = (1 << 7);
inline uint8_t withDP(uint8_t p){ return (uint8_t)(p | 0x01); } // bit0=DP

// Индексы (без конфликтов с avr/io.h)
enum { IDX_A=0, IDX_B, IDX_C, IDX_D, IDX_E, IDX_F, IDX_G };
const uint8_t SEG_MASK_BY_IDX[7] = { SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G };

// ========== МАРШРУТЫ ЦИФР ==========
const uint8_t PROGMEM DIGIT_ROUTE[10][7] = 
{
  { IDX_A, IDX_B, IDX_C, IDX_D, IDX_E, IDX_F, 0xFF },        // 0: a b c d e f
  { IDX_B, IDX_C, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },            // 1: b c
  { IDX_A, IDX_B, IDX_G, IDX_E, IDX_D, 0xFF, 0xFF },         // 2: a b g e d
  { IDX_A, IDX_B, IDX_G, IDX_C, IDX_D, 0xFF, 0xFF },         // 3: a b g c d
  { IDX_F, IDX_G, IDX_B, IDX_C, 0xFF, 0xFF, 0xFF },          // 4: f g b c
  { IDX_A, IDX_F, IDX_G, IDX_C, IDX_D, 0xFF, 0xFF },         // 5: a f g c d
  { IDX_A, IDX_F, IDX_G, IDX_C, IDX_D, IDX_E, 0xFF },        // 6: a f g c d e
  { IDX_A, IDX_B, IDX_C, 0xFF, 0xFF, 0xFF, 0xFF },           // 7: a b c
  { IDX_A, IDX_B, IDX_C, IDX_D, IDX_E, IDX_F, IDX_G },       // 8: a b c d e f g
  { IDX_D, IDX_C, IDX_B, IDX_A, IDX_F, IDX_G, 0xFF }         // 9: d c b a f g
};

// ==== паттерны под вашу раскладку битов ====
const uint8_t PAT_G = /* a f g e c d */ (SEG_A | SEG_F | SEG_G | SEG_E | SEG_C | SEG_D); // похоже на "G"
const uint8_t PAT_o = /* c d e g     */ (SEG_C | SEG_D | SEG_E | SEG_G);                 // маленькая "o"
const uint8_t PAT_d = /* b c d e g   */ (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G);         // маленькая "d"

// один сегмент g на указанной позиции
inline void showGat(uint8_t pos)
{
  blankAllDigits();
  dvdLED.setSegments(SEG_G, pos);
}

// бегущий g (анимация поиска)
// durationMs — сколько максимум ждём текущую сеть (например, 10 000 мс)
bool waitWifiWithGRun(uint32_t durationMs)
{
  const uint16_t stepMs = 120; // скорость «бега» g
  uint8_t pos = 0;
  int8_t  dir = +1;
  uint32_t t0 = millis(), tAnim = 0;

  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < durationMs)
  {
    if (millis() - tAnim >= stepMs)
    {
      tAnim = millis();
      showGat(pos);
      pos += dir;
      if (pos >= 3) { pos = 3; dir = -1; }
      if (pos == 0) { dir = +1; }
    }
    // даём время WiFi‑стеку
    delay(1);
    yield();
  }
  return WiFi.status() == WL_CONNECTED;
}

// показать слово Good на 4 позициях
void showGOOD(uint16_t holdMs = 1500)
{
  blankAllDigits();
  dvdLED.setSegments(PAT_G, 0);
  dvdLED.setSegments(PAT_o, 1);
  dvdLED.setSegments(PAT_o, 2);
  dvdLED.setSegments(PAT_d, 3);
  setColon(false);
  delay(holdMs);
}


// ========== ДВОЕТОЧИЕ ЧЕРЕЗ TICKER ==========
Ticker colonTicker;
volatile bool colonState = false;
volatile bool colonDirty = false;

void IRAM_ATTR colonTickISR()
{
  colonState = !colonState;
  colonDirty = true;
}

void applyColonIfDirty()
{
  if (colonDirty){
    colonDirty = false;
    // применять только в режиме TIME; в других режимах состояние задаём вручную
    // но сам вызов безвреден
    dvdLED.setSegments(colonState ? (1<<3) : 0, 4);
  }
}

// ========== УТИЛИТЫ ВЫВОДА ==========
void setColon(bool on){ dvdLED.setSegments(on ? (1 << 3) : 0, 4); } // pos4, bit3
void blankAllDigits(){ for(uint8_t p=0; p<4; ++p) dvdLED.setSegments(0, p); }
void drawDigits(uint8_t hh, uint8_t mm){
  dvdLED.setSegments(segPattern((hh/10)%10), 0);
  dvdLED.setSegments(segPattern(hh%10),      1);
  dvdLED.setSegments(segPattern((mm/10)%10), 2);
  dvdLED.setSegments(segPattern(mm%10),      3);
}

// Прокрутка a..f на одной позиции (для стартовой анимации)
void sweepPosition(uint8_t pos, uint8_t cycles=2, uint16_t stepMs=110){
  static const uint8_t ORDER6[6] = { SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F };
  for(uint8_t k=0; k<cycles; ++k)
    for(uint8_t i=0; i<6; ++i){
      dvdLED.setSegments(ORDER6[i], pos);
      delay(stepMs);
      applyColonIfDirty();
    }
}

// Анимация одной цифры по маршруту (добавляем нужные сегменты)
void animateDigitByRoute(uint8_t pos, uint8_t digit, uint16_t stepMs=110){
  uint8_t finalPat = segPattern(digit), acc = 0;
  for(uint8_t i=0; i<7; ++i){
    uint8_t idx = pgm_read_byte(&DIGIT_ROUTE[digit][i]);
    if(idx == 0xFF) break;
    uint8_t mask = SEG_MASK_BY_IDX[idx];
    if(finalPat & mask){
      acc |= mask;
      dvdLED.setSegments(acc, pos);
      delay(stepMs);
      applyColonIfDirty(); // чтобы ":" обновлялось и во время анимации
    }
  }
  dvdLED.setSegments(finalPat, pos);
}

// Построение времени по позициям: прокрутка + сборка (используется в setup)
void stagedTimePerPosition(uint8_t hh, uint8_t mm, uint16_t stepMs=110){
  uint8_t d[4] = { (uint8_t)((hh/10)%10), (uint8_t)(hh%10), (uint8_t)((mm/10)%10), (uint8_t)(mm%10) };
  blankAllDigits();
  for(uint8_t pos=0; pos<4; ++pos){
    sweepPosition(pos, 2, stepMs);
    animateDigitByRoute(pos, d[pos], stepMs);
  }
}

// ТОЛЬКО “сборка времени по маршруту цифр” (без прокрутки) — вызываем при входе в TIME
void animateTimeByRoute(uint8_t hh, uint8_t mm, uint16_t stepMs=110){
  uint8_t d[4] = { (uint8_t)((hh/10)%10), (uint8_t)(hh%10), (uint8_t)((mm/10)%10), (uint8_t)(mm%10) };
  blankAllDigits();
  for(uint8_t pos=0; pos<4; ++pos){
    animateDigitByRoute(pos, d[pos], stepMs);
  }
}

// Дата и температура
void showDate(uint8_t day, uint8_t mon){
  setColon(true);
  dvdLED.setSegments(segPattern((day/10)%10), 0);
  dvdLED.setSegments(segPattern(day%10),      1);
  dvdLED.setSegments(segPattern((mon/10)%10), 2);
  dvdLED.setSegments(segPattern(mon%10),      3);
}

void showTempC(float tC){
  setColon(false);
  int t10 = (int)roundf(tC * 10.0f);
  bool neg = (t10 < 0);
  if(neg) t10 = -t10;
  int intPart = t10 / 10;
  int frac    = t10 % 10;

  if(!neg){
    if(intPart >= 10){
      dvdLED.setSegments(segPattern((intPart/10)%10), 0);
      dvdLED.setSegments(withDP(segPattern(intPart%10)), 1);
      dvdLED.setSegments(segPattern(frac), 2);
      dvdLED.setSegments(0, 3);
    }else{
      dvdLED.setSegments(0, 0);
      dvdLED.setSegments(withDP(segPattern(intPart)), 1);
      dvdLED.setSegments(segPattern(frac), 2);
      dvdLED.setSegments(0, 3);
    }
  }else{
    if(intPart < 10){
      dvdLED.setSegments(SEG_G, 0); // минус
      dvdLED.setSegments(withDP(segPattern(intPart)), 1);
      dvdLED.setSegments(segPattern(frac), 2);
      dvdLED.setSegments(0, 3);
    }else{
      dvdLED.setSegments(SEG_G, 0);
      dvdLED.setSegments(segPattern((intPart/10)%10), 1);
      dvdLED.setSegments(segPattern(intPart%10),      2);
      dvdLED.setSegments(0, 3);
    }
  }
}

// Ротация экранов
enum Mode : uint8_t { MODE_TIME=0, MODE_DATE=1, MODE_TEMP=2 };
Mode mode = MODE_TIME;

const uint32_t T_TIME_MS = 10000;
const uint32_t T_DATE_MS = 4000;
const uint32_t T_TEMP_MS = 4000;
uint32_t modeT0 = 0;

void startColonTickerForTime(){
  colonTicker.detach();
  colonTicker.attach(0.25, colonTickISR); // 2 Гц: переключение каждые 250 мс
}
void stopColonTickerAndSet(bool on){
  colonTicker.detach();
  setColon(on);
}

void nextMode(){
  mode = (Mode)((mode + 1) % 3);
  modeT0 = millis();

  if(mode == MODE_TIME){
    startColonTickerForTime();
    DateTime now = rtc.now();
    // ПОКАЗ ВРЕМЕНИ ПО МАРШРУТУ ЦИФР (после температуры — именно то, что просили)
    animateTimeByRoute(now.hour(), now.minute(), 110);
  }else if(mode == MODE_DATE){
    stopColonTickerAndSet(true);
    DateTime now = rtc.now();
    showDate(now.day(), now.month());
  }else{ // MODE_TEMP
    stopColonTickerAndSet(false);
    showTempC(rtc.getTemperature());
  }
}

// tzOffsetSec — смещение часового пояса в секундах (пример: Москва = 3*3600)
bool adjustRtcFromNtp(long tzOffsetSec = 2*3600, long dstOffsetSec = 0) 
{
  configTime(tzOffsetSec, dstOffsetSec, "pool.ntp.org", "time.nist.gov", "time.google.com");

  // ждём, пока time() вернёт валидное время (порог ~2021-01-01)
  const time_t VALID_EPOCH = 1609459200;
  uint32_t t0 = millis();
  time_t now = 0;
  while ((now = time(nullptr)) < VALID_EPOCH && (millis() - t0) < 15000) 
  {
    delay(200);
    yield();
  }
  if (now < VALID_EPOCH) return false;

  struct tm tmLocal;
  localtime_r(&now, &tmLocal);
  rtc.adjust(DateTime(
    tmLocal.tm_year + 1900,
    tmLocal.tm_mon + 1,
    tmLocal.tm_mday,
    tmLocal.tm_hour,
    tmLocal.tm_min,
    tmLocal.tm_sec
  ));
  return true;
}


#define ADJUST_AT_BOOT 0

void setup()
{

  Wire.begin();

  setColon(false);
  blankAllDigits();

  if(!rtc.begin()){ drawDigits(88,88); setColon(true); while(1){} }
#if ADJUST_AT_BOOT
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
#endif

  // Стартовая анимация: прокрутка + сборка по маршрутам
  DateTime t = rtc.now();
  stagedTimePerPosition(t.hour(), t.minute(), 110);

  mode = MODE_TIME;
  modeT0 = millis();
  startColonTickerForTime();




  WiFi.mode(WIFI_STA);
  WiFi.persistent(false); // не записывать в flash

  const char* ssid[]     = {"TP-LINK_333856", "TP-Link_22DC"};
  const char* password[] = {"",               "Mar4enko2704"};
  const int numNetworks = 2;

  bool connected = false;

  for (int i = 0; i < numNetworks && !connected; ++i)
  {
    WiFi.disconnect(true);           // чистый старт на каждый SSID
    delay(50);
    WiFi.begin(ssid[i], password[i]);

    // во время ожидания подключения к этой сети «бежит» сегмент g
    connected = waitWifiWithGRun(10000UL); // ждём до 10 секунд на сеть
  }

if (connected)
{
  showGOOD(1500); // показать Good ~1.5 c

  // Синхронизация DS3231 по NTP и запись в RTC
  if (!adjustRtcFromNtp(2 * 3600 /*ваш часовой пояс*/, 1 /*летнее смещение*/)) {
    // Резерв: если NTP не доступен, ставим время компиляции
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}
else
{
  blankAllDigits();
  dvdLED.setSegments(PAT_o, 2);
  dvdLED.setSegments(PAT_o, 3);
  setColon(false);
  delay(1500);
}
}

void loop()
{
  static uint8_t prevH = 255, prevM = 255;

  uint32_t ms = millis();
  uint32_t dwell = (mode==MODE_TIME) ? T_TIME_MS :
                   (mode==MODE_DATE) ? T_DATE_MS : T_TEMP_MS;
  if(ms - modeT0 >= dwell) nextMode();

  // Применяем состояние «:» из Ticker (обновляется даже во время анимаций)
  applyColonIfDirty();

  // Периодическое обновление содержимого
  static uint32_t t250 = 0;
  if(ms - t250 >= 250){
    t250 += 250;

    DateTime now = rtc.now();
    switch(mode){
      case MODE_TIME:
        if(now.hour()!=prevH || now.minute()!=prevM){
          prevH=now.hour(); prevM=now.minute();
          drawDigits(prevH, prevM);
        }
        break;
      case MODE_DATE:
        showDate(now.day(), now.month());
        break;
      case MODE_TEMP:
        showTempC(rtc.getTemperature());
        break;
    }
  }
}