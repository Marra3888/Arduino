#include <Arduino.h>

// ВПИШИТЕ сюда 5 пинов Arduino, к которым УЖЕ подключены 5 линий ЖК (порядок ЛЮБОЙ):
uint8_t wires[5] = {8, 9, 10, 11, 13};

// Настройки
const uint16_t HOLD_MS = 1000;  // время показа ALL ON
const uint8_t  TCLK_US = 3;     // полупериод «битбанга»

// Текущие пины (назначаются при переборе)
uint8_t PIN_SCK, PIN_MOSI, PIN_CS, PIN_DC, PIN_RST;
bool csActiveLow, rstActiveLow;
uint8_t spiMode;  // 0 или 3

// Обёртки для CS/RST с полярностью
inline void CS_H() { digitalWrite(PIN_CS, csActiveLow ? LOW  : HIGH); }
inline void CS_L() { digitalWrite(PIN_CS, csActiveLow ? HIGH : LOW ); }
inline void RST_H(){ digitalWrite(PIN_RST, rstActiveLow ? LOW  : HIGH); }
inline void RST_L(){ digitalWrite(PIN_RST, rstActiveLow ? HIGH : LOW ); }

// Битбанг SPI (mode 0 или 3)
void spiWrite(uint8_t b){
  bool cpol = (spiMode==3);
  digitalWrite(PIN_SCK, cpol ? HIGH : LOW);
  for (uint8_t i=0;i<8;i++){
    digitalWrite(PIN_MOSI, (b & 0x80) ? HIGH : LOW);
    if (!cpol){ // mode 0: данные читаются по фронту ↑
      digitalWrite(PIN_SCK, HIGH); delayMicroseconds(TCLK_US);
      digitalWrite(PIN_SCK, LOW ); delayMicroseconds(TCLK_US);
    }else{      // mode 3: данные читаются по фронту ↓
      digitalWrite(PIN_SCK, LOW ); delayMicroseconds(TCLK_US);
      digitalWrite(PIN_SCK, HIGH); delayMicroseconds(TCLK_US);
    }
    b <<= 1;
  }
}

inline void wrCmd(uint8_t c){
  CS_L(); digitalWrite(PIN_DC, LOW);  spiWrite(c); CS_H();
}
inline void wrDat(uint8_t d){
  CS_L(); digitalWrite(PIN_DC, HIGH); spiWrite(d); CS_H();
}

void rstPulse(){
  RST_L(); delay(50);
  RST_H(); delay(10);
}

// ——— Мини‑инициалы для ST7565 ———
void st7565_all_on(bool seg_rev, bool com_rev, uint8_t contrast){
  wrCmd(0xAE);                      // OFF
  wrCmd(0xA2);                      // bias 1/9
  wrCmd(seg_rev ? 0xA1 : 0xA0);     // SEG
  wrCmd(com_rev ? 0xC8 : 0xC0);     // COM
  wrCmd(0x2F);                      // booster+reg+follower
  wrCmd(0x27);                      // resistor ratio
  wrCmd(0x81); wrCmd(contrast);     // контраст
  wrCmd(0xAF);                      // ON
  wrCmd(0xA5);                      // ALL ON
  delay(HOLD_MS);
  wrCmd(0xA4);
}

// ——— Мини‑инициалы для UC1701 ———
void uc1701_all_on(bool seg_rev, bool com_rev, uint8_t contrast){
  wrCmd(0xE2);                      // reset
  wrCmd(0x2C); delay(30);
  wrCmd(0x2E); delay(30);
  wrCmd(0x2F); delay(30);
  wrCmd(0x24);                      // regulator ratio
  wrCmd(0x81); wrCmd(contrast);
  wrCmd(seg_rev ? 0xA1 : 0xA0);     // SEG
  wrCmd(com_rev ? 0xC8 : 0xC0);     // COM
  wrCmd(0xA2);                      // bias 1/9
  wrCmd(0xAF);                      // ON
  wrCmd(0xA5);                      // ALL ON
  delay(HOLD_MS);
  wrCmd(0xA4);
}

void assignPins(const uint8_t p[5]){
  PIN_SCK  = p[0];
  PIN_MOSI = p[1];
  PIN_CS   = p[2];
  PIN_DC   = p[3];
  PIN_RST  = p[4];

  pinMode(PIN_SCK, OUTPUT);
  pinMode(PIN_MOSI,OUTPUT);
  pinMode(PIN_CS,  OUTPUT);
  pinMode(PIN_DC,  OUTPUT);
  pinMode(PIN_RST, OUTPUT);

  // исходные уровни
  digitalWrite(PIN_CS, csActiveLow ? HIGH : LOW);  // CS неактивен
  digitalWrite(PIN_DC, LOW);
  digitalWrite(PIN_SCK, (spiMode==3) ? HIGH : LOW);
  digitalWrite(PIN_MOSI, LOW);
  RST_H();
  delay(10);
}

bool next_perm(uint8_t *a, uint8_t n){
  int i=n-2; while(i>=0 && a[i]>=a[i+1]) i--; if(i<0) return false;
  int j=n-1; while(a[j]<=a[i]) j--; uint8_t t=a[i]; a[i]=a[j]; a[j]=t;
  int l=i+1,r=n-1; while(l<r){ t=a[l]; a[l]=a[r]; a[r]=t; l++; r--; }
  return true;
}

void setup(){
  Serial.begin(115200);
  Serial.println(F("\nBRUTE 5 wires: all roles + polarities + SPI modes + 2 controllers"));

  uint8_t idx[5]={0,1,2,3,4};
  uint32_t k=0;

  do{
    // Текущее назначение ролей: [SCK, MOSI, CS, DC, RST]
    uint8_t p[5] = { wires[idx[0]], wires[idx[1]], wires[idx[2]], wires[idx[3]], wires[idx[4]] };

    for (uint8_t csPol=0; csPol<2; csPol++)
    for (uint8_t rstPol=0; rstPol<2; rstPol++)
    for (uint8_t mode=0; mode<2; mode++){        // 0 или 3
      csActiveLow  = (csPol==0);
      rstActiveLow = (rstPol==0);
      spiMode      = (mode==0) ? 0 : 3;

      assignPins(p);
      k++;
      Serial.print(F("Try #")); Serial.print(k);
      Serial.print(F("  SCK="));  Serial.print(PIN_SCK);
      Serial.print(F(" MOSI="));  Serial.print(PIN_MOSI);
      Serial.print(F(" CS="));    Serial.print(PIN_CS);
      Serial.print(F(" DC="));    Serial.print(PIN_DC);
      Serial.print(F(" RST="));   Serial.print(PIN_RST);
      Serial.print(F("  CS="));   Serial.print(csActiveLow?"LOW":"HIGH");
      Serial.print(F(" RST="));   Serial.print(rstActiveLow?"LOW":"HIGH");
      Serial.print(F(" MODE="));  Serial.println(spiMode);

      // Сброс
      RST_L(); delay(50); RST_H(); delay(10);

      // ST7565: 4 ориентации и 2 контраста
      for(uint8_t seg=0; seg<2; seg++)
        for(uint8_t com=0; com<2; com++){
          RST_L(); delay(50); RST_H(); delay(10);
          st7565_all_on(seg, com, 0x18);
          RST_L(); delay(50); RST_H(); delay(10);
          st7565_all_on(seg, com, 0x24);
        }

      // UC1701: 4 ориентации и 2 контраста
      for(uint8_t seg=0; seg<2; seg++)
        for(uint8_t com=0; com<2; com++){
          RST_L(); delay(50); RST_H(); delay(10);
          uc1701_all_on(seg, com, 0x18);
          RST_L(); delay(50); RST_H(); delay(10);
          uc1701_all_on(seg, com, 0x24);
        }
    }

  } while(next_perm(idx,5));

  Serial.println(F("Done. No reaction? Check wiring/power/lines again."));
}

void loop(){}