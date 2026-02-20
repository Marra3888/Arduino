#include <Arduino.h>
#include <Wire.h>
#include <TM1637Display.h>
#include "ds3231_simple.h"   // твоя простая библиотека DS3231

// Пины (WeMos D1 mini / NodeMCU)
#define I2C_SDA  D2
#define I2C_SCL  D1
#define TM_CLK   D5
#define TM_DIO   D6

TM1637Display display(TM_CLK, TM_DIO);
DS3231_Simple rtc;

// Таблица сегментов для цифр 0..9 (abcdefg), dp = 0x80
const uint8_t SEG_DIGIT[10] = {
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F  // 9
};

// Биты сегментов (совместимы с TM1637Display)
#define SA 0x01  // a
#define SB 0x02  // b
#define SC 0x04  // c
#define SD 0x08  // d
#define SE 0x10  // e
#define SF 0x20  // f
#define SG 0x40  // g
#define SDP 0x80 // двоеточие/точка (используем для ":" у 2-го разряда)

// Порядок «бегущей полоски» по сегментам (по кругу)
const uint8_t RUN_SEQ[] = { SA, SB, SC, SD, SE, SF, SG };
const uint8_t RUN_LEN   = sizeof(RUN_SEQ);

// Состояние анимации
// struct Anim {
//   bool     active = false;     // идёт анимация?
//   uint8_t  step   = 0;         // 0..9 (10 кадров)
//   uint32_t tPrev  = 0;         // время последнего шага
//   uint16_t period = 10;        // мс на кадр (10*60 = 600 мс)
//   uint8_t  mask   = 0;         // битовая маска разрядов, которые нужно анимировать (bit0..bit3)
//   uint8_t  target[4];          // целевые цифры для каждого разряда (0..9)
// } anim;

struct Anim 
{
  enum Mode { SEG_RUN /*бегущая полоска*/, DIGIT_SCROLL /*если захочешь вернуть*/ } mode = SEG_RUN;
  bool     active   = false;  // идёт анимация?
  uint8_t  step     = 0;      // номер кадра
  uint8_t  frames   = 0;      // сколько кадров уже показано
  uint8_t  framesTotal = 10;  // 10×60мс = 600мс
  uint16_t periodMs = 60;     // длительность кадра
  uint32_t tPrev    = 0;      // время последнего кадра
  uint8_t  mask     = 0;      // какие разряды анимируем (bit0..bit3)
  uint8_t  target[4];         // целевые цифры разрядов 0..9
} anim;

// Текущие и предыдущие цифры на экране (только числа HH:MM)
uint8_t lastDigits[4] = { 0xFF, 0xFF, 0xFF, 0xFF }; // 0xFF => первый показ без анимации
// uint8_t curDigits [4] = { 0, 0, 0, 0 };

// Хелпер: отрисовать 4 разряда, с учётом двоеточия
void renderDigits(const uint8_t digits[4], bool colon)
{
  uint8_t segs[4];
  for (int i=0; i<4; i++) segs[i] = SEG_DIGIT[digits[i]];
  if (colon) segs[1] |= 0x80;         // двоеточие — точка у второго разряда
  display.setSegments(segs);
}

// Запуск анимации прокрутки для изменившихся разрядов
void startAnim(uint8_t changedMask, const uint8_t newDigits[4])
{
  anim.active = true;
  anim.step   = 0;
  anim.tPrev  = millis();
  anim.mask   = changedMask;
  for (int i=0; i<4; i++) anim.target[i] = newDigits[i];
}

// Кадр анимации: показать для изменившихся разрядов «anim.step», остальные — целевые
void renderAnimFrame(bool colon)
{
  uint8_t segs[4];
  for (int i=0; i<4; i++) {
    if (anim.mask & (1<<i)) {
      // этот разряд крутим 0..9
      segs[i] = SEG_DIGIT[anim.step % 10];
    } else {
      // этот разряд уже на целевой цифре
      segs[i] = SEG_DIGIT[anim.target[i]];
    }
  }
  if (colon) segs[1] |= 0x80;
  display.setSegments(segs);
}

// При необходимости — установить время компиляции, если RTC стоял
void setCompileTimeIfNeeded()
{
  bool osf = false;
  if (!rtc.isOSFSet(osf)) return;
  if (!osf) return;

  static const char* months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  char m3[4] = { __DATE__[0], __DATE__[1], __DATE__[2], 0 };
  int mm = (strstr(months, m3) - months) / 3 + 1;

  DS3231_Simple::DateTime t;
  t.year   = atoi(__DATE__ + 7);
  t.month  = (uint8_t)mm;
  t.day    = (uint8_t)atoi(__DATE__ + 4);
  t.hour   = (uint8_t)atoi(__TIME__ + 0);
  t.minute = (uint8_t)atoi(__TIME__ + 3);
  t.second = (uint8_t)atoi(__TIME__ + 6);

  rtc.writeTime(t);
  rtc.clearOSF();
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);

  display.setBrightness(7, true);
  display.clear();

  if (!rtc.begin()) {
    Serial.println(F("DS3231 not found"));
    uint8_t dash=0x40, dashes[4]={dash,dash,dash,dash};
    display.setSegments(dashes);
    while(1) delay(1000);
  }
  rtc.set24h();
  rtc.setClockHalt(false); // включаем осциллятор
  setCompileTimeIfNeeded();
}

void loop()
{
  static DS3231_Simple::DateTime t;
  static uint32_t tPoll = 0;

  // Мигающее двоеточие — всегда по таймеру
  bool colon = ((millis()/500) % 2);

  // Если идёт анимация — отрисовываем кадры и выходим
  if (anim.active) 
  {
    if (millis() - anim.tPrev >= anim.period) 
    {
      anim.tPrev = millis();
      anim.step++;
      if (anim.step >= 10) 
      {
        anim.active = false;
        // после прокрутки — показать целевые и закрепить их как текущие
        renderDigits(anim.target, colon);
        for (int i=0; i<4; i++) lastDigits[i] = anim.target[i];
      } 
      else 
      {
        renderAnimFrame(colon);
      }
    } 
    else 
    {
      // между кадрами — просто поддерживаем двоеточие
      renderAnimFrame(colon);
    }
    delay(10);
    return;
  }

  // Периодически читаем время (5 раз в секунду достаточно)
  if (millis() - tPoll > 200) {
    tPoll = millis();

    if (rtc.readTime(t)) {
      // Сформировать цифры HHMM
      
      uint8_t newDigits[4] = {
                                  // (uint8_t)((t.hour / 10) % 10),
                                  // (uint8_t)( t.hour % 10),
                                  (uint8_t)((t.minute / 10) % 10),
                                  (uint8_t)( t.minute % 10),
                                  (uint8_t)((t.second / 10) % 10),
                                  (uint8_t)( t.second % 10)
                                };

      // Первый показ: просто вывести без анимации
      if (!lastDigits[0] == 0xFF) 
          {
            renderDigits(newDigits, colon);
            for (int i = 0; i < 4; i++) lastDigits[i] = newDigits[i];
          } 
      else 
      {
        // Определить какие разряды изменились
        uint8_t mask = 0;
        for (int i = 0; i < 4; i++) if (newDigits[i] != lastDigits[i]) mask |= (1 << i);

        if (mask) 
          {
            // Запуск анимации одновременно для всех изменившихся разрядов
            startAnim(mask, newDigits);
            // первый кадр отрисуем сразу
            renderAnimFrame(colon);
          } 
        else 
          {
            // изменений нет — просто поддерживаем «:»
            renderDigits(lastDigits, colon);
          }
      }
    } 
    else 
      {
        // Ошибка чтения RTC — поддерживаем «:» на последних цифрах
        renderDigits(lastDigits, colon);
      }
  } 
  else 
    {
      // Между опросами — поддерживаем «:», чтобы он мигал плавно
      renderDigits(lastDigits, colon);
    }

  delay(10);
}