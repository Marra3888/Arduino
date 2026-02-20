#include <Arduino.h>
#include <string.h>

// Пины HD44100
#define PIN_DL1  11   // Data
#define PIN_CL2  13   // один из тактов (shift или latch)
#define PIN_CL1  12   // второй такт (latch или shift)
#define PIN_M    10   // переменная полярность (AC)

// Геометрия
#define HAVE_DP            1     // 1 — есть точки, 0 — нет
const uint16_t NSEG = HAVE_DP ? 80 : 70;  // 10*(7+DP) или 10*7
const uint8_t  NCOM = 4;         // чаще 4 (если у вас 8 — поставьте 8)

// Быстрые настройки (подбирайте)
#define CL2_IS_SHIFT       1     // 1: CL2=shift, CL1=latch; 0: наоборот
#define BIT_LSB_FIRST      1     // 1: отправляем биты младшим вперёд, 0: старшим вперёд
#define REVERSE_DIGIT_ORDER 1    // 1: порядок разрядов 9..0, 0: 0..9
#define M_PER_LINE         1     // 1: инвертировать M на каждой строке, 0: на кадре
#define M_INVERT           0     // 1: инвертировать уровень M целиком
#define LINE_HOLD_US       60    // «время строки», подберите (30..150 мкс)
#define TCLK_US            2     // короткая задержка между фронтами, мкс

// Шрифт A..G (+DP в бите7)
const uint8_t seg7_font[10] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111  // 9
};

// ===== карта: digit(0..9) × seg(0=A..6=G,7=DP) → индекс бита (0..NSEG-1)
int16_t mapBit[10][8];

// Побитовый буфер строк
uint8_t lineBits[8][(NSEG+7)/8];  // хватит до 8 строк

inline void setBit(uint8_t com, uint16_t bitIndex, bool val){
  uint16_t byteIdx = bitIndex >> 3;
  uint8_t  mask    = 1 << (bitIndex & 7);
  if(val) lineBits[com][byteIdx] |= mask;
  else    lineBits[com][byteIdx] &= ~mask;
}

inline void pulse(uint8_t pin){
  digitalWrite(pin, HIGH); delayMicroseconds(TCLK_US);
  digitalWrite(pin, LOW ); delayMicroseconds(TCLK_US);
}

// Сдвиг одной строки из lineBits[com]
void shiftLineFromBuf(uint8_t com){
  uint8_t pinShift = CL2_IS_SHIFT ? PIN_CL2 : PIN_CL1;
  uint8_t pinLatch = CL2_IS_SHIFT ? PIN_CL1 : PIN_CL2;

  uint16_t cnt = 0;
  uint16_t nbytes = (NSEG + 7) >> 1 >> 2; // (NSEG+7)/8
  for(uint16_t i=0; cnt < NSEG; i++){
    uint8_t b = lineBits[com][i];
    if (!BIT_LSB_FIRST) { // MSB-first (редко)
      for (uint8_t k=0; k<8 && cnt<NSEG; k++, cnt++){
        uint8_t bit = (b & 0x80) ? HIGH : LOW;
        digitalWrite(PIN_DL1, bit);
        pulse(pinShift);
        b <<= 1;
      }
    } else {              // LSB-first (чаще)
      for (uint8_t k=0; k<8 && cnt<NSEG; k++, cnt++){
        uint8_t bit = (b & 0x01) ? HIGH : LOW;
        digitalWrite(PIN_DL1, bit);
        pulse(pinShift);
        b >>= 1;
      }
    }
  }
  pulse(pinLatch);
  delayMicroseconds(LINE_HOLD_US);
}

// Построить lineBits под массив 10 цифр
void buildFrame(const int8_t digits[10], uint16_t dpMask){
  // очистить
  for(uint8_t com=0; com<NCOM; com++)
    memset(lineBits[com], 0, (NSEG+7)/8);

  for(uint8_t d=0; d<10; d++){
    int8_t val = digits[d];
    if(val < 0 || val > 9) continue;
    uint8_t pat = seg7_font[val];
#if HAVE_DP
    if (dpMask & (1U<<d)) pat |= 0x80;
#endif

    // для каждого сегмента A..G, DP
    for(uint8_t s=0; s<(HAVE_DP ? 8 : 7); s++){
      if (!(pat & (1<<s))) continue;
      int16_t bitIndex = mapBit[d][s];
      if (bitIndex < 0) continue;

      // В 1/4 duty сегмент активен на одном COM.
      // Если ваша карта даёт один bitIndex на конкретный COM — просто зажгите на всех COM для наглядности,
      // либо (предпочтительнее) храните по COM. Пока включим на всех — для проверки.
      for(uint8_t com=0; com<NCOM; com++)
        setBit(com, (uint16_t)bitIndex, true);
    }
  }
}

// Сформировать «линейную» карту: на каждый разряд свой блок из 8 бит
void fillLinearMap() {
  for (uint8_t d=0; d<10; d++){
    uint16_t base = (REVERSE_DIGIT_ORDER ? (9 - d) : d) * (HAVE_DP ? 8 : 7);
    for (uint8_t s=0; s<8; s++){
#if HAVE_DP
      mapBit[d][s] = base + s;       // A..G,DP подряд
#else
      mapBit[d][s] = (s<7) ? (base + s) : -1; // DP нет
#endif
    }
  }
}

void setup(){
  pinMode(PIN_DL1, OUTPUT);
  pinMode(PIN_CL1, OUTPUT);
  pinMode(PIN_CL2, OUTPUT);
  pinMode(PIN_M,   OUTPUT);
  digitalWrite(PIN_DL1, LOW);
  digitalWrite(PIN_CL1, LOW);
  digitalWrite(PIN_CL2, LOW);
  digitalWrite(PIN_M,   LOW);

  fillLinearMap();  // <-- главное отличие от вашей версии
}

void loop(){
  // Пример: выводим 0123456789, точки на 2 и 6 разряде
  int8_t digits[10] = {0,1,2,3,4,5,6,7,8,9};
#if HAVE_DP
  uint16_t dpMask = (1<<1) | (1<<5);
#else
  uint16_t dpMask = 0;
#endif

  buildFrame(digits, dpMask);

  // Скан кадра с AC на M
  static bool mFrame = false;
  bool m = (M_INVERT ? !mFrame : mFrame);
  for(uint8_t com=0; com<NCOM; com++){
    digitalWrite(PIN_M, m);
    shiftLineFromBuf(com);
    if (M_PER_LINE) m = !m;
  }
  if (!M_PER_LINE) mFrame = !mFrame;
}