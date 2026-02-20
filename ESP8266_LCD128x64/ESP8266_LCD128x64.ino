#include <Arduino.h>

// УКАЖИТЕ ваши 5 пинов ESP8266, на которые уже заведены 5 ЛИНИЙ ПАНЕЛИ (в ЛЮБОМ порядке):
// РЕКОМЕНДУЕМЫЕ «безопасные» GPIO: D1, D2, D5, D6, D7
uint8_t wires[5] = { D1, D2, D5, D6, D7 };

// Время удержания «all pixels ON» и скорость «битбанга»
const uint16_t HOLD_MS = 700;  // 0.7 c на окно
const uint8_t  TCLK_US = 3;    // полупериод «часов» (увеличьте при необходимости)

// Текущее назначение ролей (меняются при переборе)
uint8_t PIN_SCK, PIN_MOSI, PIN_CS, PIN_DC, PIN_RST;
bool csActiveLow, rstActiveLow;
uint8_t spiMode;   // 0 или 3

inline void CS_H() { digitalWrite(PIN_CS, csActiveLow ? LOW  : HIGH); }
inline void CS_L() { digitalWrite(PIN_CS, csActiveLow ? HIGH : LOW ); }
inline void RST_H(){ digitalWrite(PIN_RST, rstActiveLow ? LOW  : HIGH); }
inline void RST_L(){ digitalWrite(PIN_RST, rstActiveLow ? HIGH : LOW ); }

// Битбанг SPI (mode 0 или 3). На ESP обязательно yield() (watchdog).
void spiWrite(uint8_t b){
  bool cpol = (spiMode==3);
  digitalWrite(PIN_SCK, cpol ? HIGH : LOW);
  for (uint8_t i=0;i<8;i++){
    digitalWrite(PIN_MOSI, (b & 0x80) ? HIGH : LOW);
    if (!cpol){ // mode 0
      digitalWrite(PIN_SCK, HIGH); delayMicroseconds(TCLK_US);
      digitalWrite(PIN_SCK, LOW ); delayMicroseconds(TCLK_US);
    }else{      // mode 3
      digitalWrite(PIN_SCK, LOW ); delayMicroseconds(TCLK_US);
      digitalWrite(PIN_SCK, HIGH); delayMicroseconds(TCLK_US);
    }
    b <<= 1;
  }
  yield();
}

inline void wrCmd(uint8_t c){ CS_L(); digitalWrite(PIN_DC, LOW ); spiWrite(c); CS_H(); }
inline void wrDat(uint8_t d){ CS_L(); digitalWrite(PIN_DC, HIGH); spiWrite(d); CS_H(); }

void rstPulse(){ RST_L(); delay(60); RST_H(); delay(15); yield(); }

// ST7565 минимальная инициализация + ALL ON
void st7565_all_on(bool seg_rev, bool com_rev, uint8_t contrast){
  wrCmd(0xAE);
  wrCmd(0xA2);
  wrCmd(seg_rev ? 0xA1 : 0xA0);
  wrCmd(com_rev ? 0xC8 : 0xC0);
  wrCmd(0x2F);
  wrCmd(0x27);
  wrCmd(0x81); wrCmd(contrast);   // 0x10..0x3F
  wrCmd(0xAF);
  wrCmd(0xA5);
  delay(HOLD_MS);
  wrCmd(0xA4);
}

// UC1701 минимальная инициализация + ALL ON
void uc1701_all_on(bool seg_rev, bool com_rev, uint8_t contrast){
  wrCmd(0xE2);
  wrCmd(0x2C); delay(20);
  wrCmd(0x2E); delay(20);
  wrCmd(0x2F); delay(20);
  wrCmd(0x24);
  wrCmd(0x81); wrCmd(contrast);   // 0x10..0x30
  wrCmd(seg_rev ? 0xA1 : 0xA0);
  wrCmd(com_rev ? 0xC8 : 0xC0);
  wrCmd(0xA2);
  wrCmd(0xAF);
  wrCmd(0xA5);
  delay(HOLD_MS);
  wrCmd(0xA4);
}

void assignPins(const uint8_t p[5]){
  PIN_SCK  = p[0];
  PIN_MOSI = p[1];
  PIN_CS   = p[2];
  PIN_DC   = p[3];
  PIN_RST  = p[4];

  pinMode(PIN_SCK,  OUTPUT);
  pinMode(PIN_MOSI, OUTPUT);
  pinMode(PIN_CS,   OUTPUT);
  pinMode(PIN_DC,   OUTPUT);
  pinMode(PIN_RST,  OUTPUT);

  // исходные уровни
  CS_H();
  digitalWrite(PIN_DC, LOW);
  digitalWrite(PIN_SCK, (spiMode==3) ? HIGH : LOW);
  digitalWrite(PIN_MOSI, LOW);
  RST_H();
  delay(10);
  yield();
}

// next_permutation
bool next_perm(uint8_t *a, uint8_t n){
  int i=n-2; while(i>=0 && a[i]>=a[i+1]) i--; if(i<0) return false;
  int j=n-1; while(a[j]<=a[i]) j--; uint8_t t=a[i]; a[i]=a[j]; a[j]=t;
  int l=i+1,r=n-1; while(l<r){ t=a[l]; a[l]=a[r]; a[r]=t; l++; r--; }
  return true;
}

void setup(){
  Serial.begin(115200);
  Serial.println(F("\nESP8266 GLCD brute-force (5 wires): roles+polarities+SPI modes+controllers"));

  uint8_t idx[5]={0,1,2,3,4};
  uint32_t attempt=0;

  do{
    // Назначение ролей: [SCK, MOSI, CS, DC, RST]
    uint8_t p[5] = { wires[idx[0]], wires[idx[1]], wires[idx[2]], wires[idx[3]], wires[idx[4]] };

    for (uint8_t csPol=0; csPol<2; csPol++)
    for (uint8_t rstPol=0; rstPol<2; rstPol++)
    for (uint8_t mode=0; mode<2; mode++){            // SPI mode 0/3
      csActiveLow  = (csPol==0);
      rstActiveLow = (rstPol==0);
      spiMode      = (mode==0) ? 0 : 3;

      assignPins(p);
      attempt++;
      Serial.printf("Try #%lu  SCK=%d MOSI=%d CS=%d DC=%d RST=%d  CS=%s RST=%s MODE=%u\n",
        attempt, PIN_SCK, PIN_MOSI, PIN_CS, PIN_DC, PIN_RST,
        csActiveLow?"LOW":"HIGH", rstActiveLow?"LOW":"HIGH", spiMode);

      // — ST7565: 4 ориентации + 2 контраста
      for (uint8_t seg=0; seg<2; seg++)
        for (uint8_t com=0; com<2; com++){
          rstPulse(); st7565_all_on(seg, com, 0x18);
          rstPulse(); st7565_all_on(seg, com, 0x24);
        }

      // — UC1701: 4 ориентации + 2 контраста
      for (uint8_t seg=0; seg<2; seg++)
        for (uint8_t com=0; com<2; com++){
          rstPulse(); uc1701_all_on(seg, com, 0x18);
          rstPulse(); uc1701_all_on(seg, com, 0x24);
        }
    }
    yield();

  } while (next_perm(idx,5));

  Serial.println(F("Brute-force finished. If no reaction, re-check wiring/power/lines."));
}

void loop(){}