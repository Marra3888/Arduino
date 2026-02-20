#include <Arduino.h>

// ---------- Пины (8‑бит 6800) ----------
const uint8_t DB[8] = {2,3,4,5,6,7,8,9}; // CN10: 9..16 = D0..D7
#define PIN_RS   10   // CN10-4 A0/RS
#define PIN_E    12   // CN10-5 E/WR
#define PIN_RST  A2   // CN10-17 RST
#define PIN_CS   A1   // CN10-18 CS
#define CS_ACTIVE_LOW  1     // если CS активный высоким — поставьте 0

// ---------- Контраст / ориентация ----------
#define BIAS_CMD   0xA3    // 0xA2 (1/9) или 0xA3 (1/7)
#define RATIO_CMD  0x26    // 0x24..0x26
#define EV_VALUE   0x32    // 0x00..0x3F

#define ADC_NORMAL 1       // 1: 0xA0, 0: 0xA1 (зеркало по X)
#define SHL_NORMAL 1       // 1: 0xC0, 0: 0xC8 (переворот по Y)

// Профиль питания ЖК: 0 = 0x2C→0x2E→0x2F; 1 = сразу 0x2F и повтор после контраста
#define INIT_PROFILE  0

// ---------- Низкий уровень ----------
static inline void cs_on()  { digitalWrite(PIN_CS, CS_ACTIVE_LOW ? LOW : HIGH); }
static inline void cs_off() { digitalWrite(PIN_CS, CS_ACTIVE_LOW ? HIGH: LOW ); }

static inline void pulseE(){ digitalWrite(PIN_E,1); delayMicroseconds(3); digitalWrite(PIN_E,0); delayMicroseconds(3); }
static inline void busWrite(uint8_t v){ for(uint8_t i=0;i<8;i++) digitalWrite(DB[i], (v>>i)&1); }

static inline void cmd_hold(uint8_t c){ digitalWrite(PIN_RS,0); busWrite(c); pulseE(); delayMicroseconds(40); }
static inline void data_hold(uint8_t d){ digitalWrite(PIN_RS,1); busWrite(d); pulseE(); delayMicroseconds(30); }

static inline void cmd(uint8_t c){ cs_on();  cmd_hold(c);  cs_off(); }
static inline void data1(uint8_t d){ cs_on(); data_hold(d); cs_off(); }

static inline void setPage(uint8_t p){ cmd(0xB0 | (p & 0x0F)); }
static inline void setCol(uint16_t c){ cmd(0x10 | ((c>>4)&0x0F)); cmd(0x00 | (c&0x0F)); }
// Для совместимости с чужим кодом
static inline void setColumn(uint16_t c){ setCol(c); }

// ---------- Контраст ----------
static inline void setContrast(uint8_t bias, uint8_t ratio, uint8_t ev){
  cmd(bias);                 // 0xA2/0xA3
  cmd(ratio);                // 0x24..0x26
  cmd(0x81); cmd(ev & 0x3F); // Electronic Volume
}

// ---------- Инициализация (без какого‑либо вывода) ----------
void lcd_init(){
  for(uint8_t i=0;i<8;i++){ pinMode(DB[i],OUTPUT); digitalWrite(DB[i],0); }
  pinMode(PIN_RS,OUTPUT); pinMode(PIN_E,OUTPUT);
  pinMode(PIN_RST,OUTPUT); pinMode(PIN_CS,OUTPUT);
  digitalWrite(PIN_E,0);

  // Жёсткий сброс
  digitalWrite(PIN_RST,0); delay(30);
  digitalWrite(PIN_RST,1); delay(30);

  // Держим CS включённым на весь init (некоторые ревизии этого требуют)
  cs_on();

  // Базовый init
  cmd_hold(0xAE);           // Display OFF
  cmd_hold(0xE2);           // SW Reset
#if (INIT_PROFILE==0)
  cmd_hold(0x2C); delay(5); // Booster
  cmd_hold(0x2E); delay(5); // Regulator
  cmd_hold(0x2F); delay(15);// Follower
#else
  cmd_hold(0x2F); delay(20);// Все биты питания сразу
#endif

  cmd_hold(ADC_NORMAL ? 0xA0 : 0xA1); // Ориентация X
  cmd_hold(SHL_NORMAL ? 0xC0 : 0xC8); // Ориентация Y

  // Статический индикатор — OFF (некоторые чипы стартуют в ON)
  cmd_hold(0xAC); cmd_hold(0x00);     // Static indicator OFF

  // Контраст
  setContrast(BIAS_CMD, RATIO_CMD, EV_VALUE);
  delay(5);

#if (INIT_PROFILE==1)
  // Повтор 0x2F после установки EV — «будит» капризные панели
  cmd_hold(0x2F); delay(10);
#endif

  // Нормальный режим отображения
  cmd_hold(0xA6);            // Normal (не инверсия)
  cmd_hold(0xA4);            // Resume (не all-points-on)
  cmd_hold(0x40);            // Start line = 0

  cmd_hold(0xAF);            // Display ON
  delay(20);

  cs_off();

  
  cmd(0xA5); 
}


void setup(){
  // Важно: R/W (CN10‑8) — на GND аппаратно. Подсветка — у вас на VCC.
  lcd_init();  // только инициализация
}

void loop(){
  // пусто — без вывода чего‑либо
}