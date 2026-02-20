#include <Arduino.h>

// -------------------- ТВОИ ПИНЫ --------------------
// 8 физических проводов данных (CN10-9..16) — впиши СВОИ пины Arduino в том порядке, как подключено
const uint8_t PIN_DB[8] = {2,3,4,5,6,7,8,9};  // не меняем проводку — меняем карту MAP ниже

// Управляющие:
#define PIN_RS   10   // CN10-4  (RS/A0)
#define PIN_E1   12   // CN10-5  (левая половина)  ← если у тебя E1=4, просто поставь 4
#define PIN_E2   A1   // CN10-18 (правая половина) ← если у тебя E2=8, поставь 8
#define PIN_RST  A2   // CN10-17
#define PIN_RW   A3   // CN10-8  (LOW = запись)

// Строб активен НИЗКИМ (по твоим тестам именно так)
#define E_ACTIVE_LOW 1

// Геометрия
uint8_t CHIP_W = 64;     // начни с 64; если всё ок — попробуй 67 для 134 пикселей
const uint8_t PAGES = 2; // 16px по Y => 2 страницы

// -------------------- Карты соответствия битов --------------------
// MAP[k][b] = индекс в PIN_DB, на который подать бит b (0..7) исходного байта
const uint8_t MAPS[4][8] = {
  // MAP0: прямой порядок
  {0,1,2,3,4,5,6,7},
  // MAP1: полный реверс (часто так и бывает на "зеркальных" шлейфах)
  {7,6,5,4,3,2,1,0},
  // MAP2: обмен полубайтами (D0..D3 ↔ D4..D7)
  {4,5,6,7,0,1,2,3},
  // MAP3: реверс внутри каждого полубайта
  {3,2,1,0,7,6,5,4}
};
uint8_t mapIdx = 1;  // стартуем с MAP1 (реверс) — чаще всего он «заходит»

// -------------------- Низкий уровень --------------------
enum Sel { LEFT=1, RIGHT=2, BOTH=3 };

static inline void eIdle(){
  digitalWrite(PIN_E1, E_ACTIVE_LOW ? HIGH : LOW);
  digitalWrite(PIN_E2, E_ACTIVE_LOW ? HIGH : LOW);
}
static inline void pulseE(uint8_t pin){
  if (E_ACTIVE_LOW){ digitalWrite(pin, LOW); delayMicroseconds(8); digitalWrite(pin, HIGH); }
  else             { digitalWrite(pin, HIGH); delayMicroseconds(8); digitalWrite(pin, LOW ); }
  delayMicroseconds(4);
}
// Пишем байт с учётом текущей карты mapIdx
static inline void busWriteMapped(uint8_t v){
  const uint8_t* M = MAPS[mapIdx];
  for (uint8_t b=0; b<8; b++){
    uint8_t phys = M[b]; // какой провод из PIN_DB несёт бит b
    digitalWrite(PIN_DB[phys], (v >> b) & 1);
  }
}
static inline void wr(uint8_t val, bool isData, Sel s){
  digitalWrite(PIN_RS, isData ? HIGH : LOW);
  busWriteMapped(val);
  if (s & LEFT)  pulseE(PIN_E1);
  if (s & RIGHT) pulseE(PIN_E2);
  delayMicroseconds(isData ? 70 : 80);
}
static inline void cmd (uint8_t c, Sel s){ wr(c,false,s); }
static inline void data(uint8_t d, Sel s){ wr(d,true, s);  }
static inline void setPage(uint8_t p, Sel s){ cmd(0xB0 | (p & 0x0F), s); }
static inline void setCol (uint8_t c, Sel s){
  cmd(0x10 | ((c>>4)&0x0F), s);
  cmd(0x00 | (c     &0x0F), s);
}

// -------------------- Инициализация UC1608/ST7565-совм. --------------------
void lcd_init(){
  Serial.begin(115200);
  Serial.println("UC1608 init. Пины данных (PIN_DB) и карта MAP переключаемая (нажми 'm').");
  Serial.print("PIN_DB = "); for(int i=0;i<8;i++){ Serial.print(PIN_DB[i]); if(i<7) Serial.print(","); } Serial.println();

  for (uint8_t i=0;i<8;i++){ pinMode(PIN_DB[i], OUTPUT); digitalWrite(PIN_DB[i], 0); }
  pinMode(PIN_RS, OUTPUT); pinMode(PIN_E1, OUTPUT); pinMode(PIN_E2, OUTPUT);
  pinMode(PIN_RST,OUTPUT); pinMode(PIN_RW, OUTPUT);
  digitalWrite(PIN_RW, LOW); // запись
  eIdle();

  digitalWrite(PIN_RST, LOW);  delay(25);
  digitalWrite(PIN_RST, HIGH); delay(60);

  // Базовый init
  cmd(0xAE, BOTH);      // OFF
  cmd(0xE2, BOTH);      // SW reset
  cmd(0xA0, BOTH);      // ADC normal (если зеркалит по X — 0xA1)
  cmd(0xC0, BOTH);      // COM normal (если вверх ногами — 0xC8)
  cmd(0x2F, BOTH); delay(20);  // booster/reg/follower ON
  cmd(0x81, BOTH); data(0x28, BOTH); // контраст (подбери 0x20..0x32)
  cmd(0xA6, BOTH);      // normal
  cmd(0xA4, BOTH);      // RAM display
  cmd(0x40, BOTH);      // start line = 0
  cmd(0xAF, BOTH); delay(10); // ON
}

void lcd_clear(){
  for(uint8_t p=0;p<PAGES;p++){
    setPage(p,LEFT);  setCol(0,LEFT);  for(uint8_t x=0;x<CHIP_W;x++) data(0x00,LEFT);
    setPage(p,RIGHT); setCol(0,RIGHT); for(uint8_t x=0;x<CHIP_W;x++) data(0x00,RIGHT);
  }
}

void stripes(){
  // Ровные вертикальные полосы — признак правильной карты MAP
  for(uint8_t p=0;p<PAGES;p++){
    setPage(p,LEFT);  setCol(0,LEFT);
    for(uint8_t x=0;x<CHIP_W;x++) data( (x&1)?0xFF:0x00, LEFT);
    setPage(p,RIGHT); setCol(0,RIGHT);
    for(uint8_t x=0;x<CHIP_W;x++) data( (x&1)?0x00:0xFF, RIGHT);
  }
}

// 8 горизонтальных линий (0x01,0x02,...,0x80) — проверка порядка битов
void bitLanes(){
  const uint8_t pats[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
  for(uint8_t i=0;i<8;i++){
    for(uint8_t p=0;p<PAGES;p++){
      setPage(p,LEFT);  setCol(0,LEFT);  for(uint8_t x=0;x<CHIP_W;x++) data(pats[i],LEFT);
      setPage(p,RIGHT); setCol(0,RIGHT); for(uint8_t x=0;x<CHIP_W;x++) data(pats[i],RIGHT);
    }
    delay(500);
    lcd_clear();
  }
}

void printMap(){
  Serial.print("MAP"); Serial.print(mapIdx); Serial.print(" = [");
  for(int i=0;i<8;i++){ Serial.print(MAPS[mapIdx][i]); if(i<7) Serial.print(","); }
  Serial.println("]  (Dbit -> index в PIN_DB)");
}

void setup(){
  lcd_init();
  lcd_clear();
  printMap();
  stripes();
  bitLanes();
}

void loop(){
  // В Serial нажми 'm' — переключить карту. 'w' — 64/67.
  if (Serial.available()){
    char c = Serial.read();
    if (c=='m'){
      mapIdx = (mapIdx+1) % 4;
      printMap();
      lcd_clear();
      stripes();
      bitLanes();
    } else if (c=='w'){
      CHIP_W = (CHIP_W==64) ? 67 : 64;
      Serial.print("CHIP_W = "); Serial.println(CHIP_W);
      lcd_clear();
      stripes();
    } else if (c=='o'){
      // Быстро поменять ориентацию, если зеркалит
      static bool ax=0, ay=0;
      ax=!ax; ay=!ay;
      cmd(ax?0xA1:0xA0, BOTH);
      cmd(ay?0xC8:0xC0, BOTH);
      lcd_clear();
      stripes();
    }
  }
}