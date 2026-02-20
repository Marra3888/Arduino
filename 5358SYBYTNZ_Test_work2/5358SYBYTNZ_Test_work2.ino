#include <Arduino.h>

// ---------- Пины (8‑бит 6800) ----------
const uint8_t DB[8] = {2,3,4,5,6,7,8,9};  // CN10: 9..16 = D0..D7
#define PIN_RS   10    // CN10-4 A0/RS
#define PIN_E    12    // CN10-5 E/WR
#define PIN_RST  A2    // CN10-17 RST
#define PIN_CS   A1    // CN10-18 CS
#define CS_ACTIVE_LOW  1 // если CS активный высоким — поставьте 0

// ---------- Контраст / ориентация ----------
#define BIAS_CMD   0xA3    // 0xA2 (1/9) или 0xA3 (1/7)
#define RATIO_CMD  0x26    // 0x24..0x26
#define EV_VALUE   0x32    // 0x00..0x3F

#define ADC_NORMAL 1       // 1: 0xA0 (нормаль), 0: 0xA1 (зеркало по X)
#define SHL_NORMAL 1       // 1: 0xC0 (нормаль), 0: 0xC8 (переворот по Y)

// ---------- Низкий уровень ----------
static inline void setCS_level(bool active){
  if (CS_ACTIVE_LOW) digitalWrite(PIN_CS, active ? LOW  : HIGH);
  else               digitalWrite(PIN_CS, active ? HIGH : LOW);
}
static inline void pulseE(){
  digitalWrite(PIN_E, HIGH);  delayMicroseconds(6);
  digitalWrite(PIN_E, LOW);   delayMicroseconds(6);
}
static inline void busWrite(uint8_t v){
  for(uint8_t i=0;i<8;i++) digitalWrite(DB[i], (v>>i)&1);
}
static inline void cmd_hold(uint8_t c){
  digitalWrite(PIN_RS, LOW);
  busWrite(c);
  pulseE();
  delayMicroseconds(120);
}
static inline void setContrast(uint8_t bias, uint8_t ratio, uint8_t ev){
  cmd_hold(bias);                 // 0xA2/0xA3
  cmd_hold(ratio);                // 0x24..0x26
  cmd_hold(0x81); cmd_hold(ev & 0x3F); // EV 0..63
}

// Один проход инициализации — выполняем его дважды с разными уровнями CS
static void init_pass(bool cs_active){
  setCS_level(cs_active);

  cmd_hold(0xAE);            // Display OFF
  cmd_hold(0xE2);            // Software Reset
  cmd_hold(0x2C); delay(10); // Booster ON
  cmd_hold(0x2E); delay(10); // Regulator ON
  cmd_hold(0x2F); delay(30); // Follower ON

  cmd_hold(ADC_NORMAL ? 0xA0 : 0xA1); // ориентация SEG
  cmd_hold(SHL_NORMAL ? 0xC0 : 0xC8); // ориентация COM

  cmd_hold(0xAC); cmd_hold(0x00);     // Static indicator OFF

  setContrast(BIAS_CMD, RATIO_CMD, EV_VALUE);
  delay(20);

  cmd_hold(0x2F); delay(20);          // стабилизация питания ЖК

  cmd_hold(0xA6);                     // Normal (не inverse)
  cmd_hold(0xA4);                     // Resume (не All-Points-ON)
  cmd_hold(0x40);                     // Start line = 0
  cmd_hold(0xAF); delay(30);          // Display ON
}

// ---------- Инициализация БЕЗ вывода ----------
void lcd_init(){
  // Пины
  for(uint8_t i=0;i<8;i++){ pinMode(DB[i],OUTPUT); digitalWrite(DB[i],LOW); }
  pinMode(PIN_RS,OUTPUT);
  pinMode(PIN_E, OUTPUT);
  pinMode(PIN_RST,OUTPUT);
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_E,LOW);

  // Жёсткий сброс
  digitalWrite(PIN_RST,LOW); delay(50);
  digitalWrite(PIN_RST,HIGH); delay(50);

  // Инициализируем обе «половины» панели
  init_pass(true);   // CS в активном уровне
  delay(10);
  init_pass(false);  // CS в противоположном уровне

  // НИКАКИХ A5/A4 и записей в GDRAM — только init.
}

void setup(){
  // ВАЖНО: CN10‑8 (R/W) — на GND аппаратно. Подсветка — на VCC (не управляем).
  lcd_init();  // только инициализация
}

void loop(){
  // ничего не рисуем
}