#include <Arduino.h>
#include <avr/pgmspace.h>

// ---------- Настройка железа ----------
#define LCD_W        150    // ширина
#define LCD_H        16     // высота
#define LCD_PAGES    (LCD_H/8) // 2
#define CHIP0_COLS   75     // кол-во колонок у левого чипа
#define CHIP1_COLS   75     // у правого чипа
#define COL_OFFSET   0      // если картинка сдвинута, попробуйте 2 или 4

// Наличие отдельных линий CS1/CS2
#define HAS_CS1         1   // 1 = есть CS1 и CS2; 0 = есть только CS2
#define CS_ACTIVE_LOW   1   // 1 = активный LOW; 0 = активный HIGH

// Пины Arduino Nano (меняйте при необходимости)
const uint8_t LCD_DB[8] = {2,3,4,5,6,7,8,9}; // D0..D7
#define PIN_RS   10    // A0/RS
#define PIN_E    12    // E/WR (Enable)
#define PIN_RST  A2    // RST
#define PIN_CS1  A0    // CS1 (если нет — HAS_CS1=0)
#define PIN_CS2  A1    // CS2

// ---------- Низкоуровневые функции ----------
static inline void pulseE(){
  digitalWrite(PIN_E, HIGH);  delayMicroseconds(2);
  digitalWrite(PIN_E, LOW);   delayMicroseconds(2);
}
static inline void busWrite(uint8_t v){
  for (uint8_t b=0;b<8;b++) digitalWrite(LCD_DB[b], (v>>b)&1);
}
static inline void wrCmd(uint8_t c){
  digitalWrite(PIN_RS, LOW);
  busWrite(c);
  pulseE();
  delayMicroseconds(40);
}
static inline void wrData(uint8_t d){
  digitalWrite(PIN_RS, HIGH);
  busWrite(d);
  pulseE();
  delayMicroseconds(40);
}

static inline void cs_none(){
#if HAS_CS1
  digitalWrite(PIN_CS1, CS_ACTIVE_LOW ? HIGH : LOW);
#endif
  digitalWrite(PIN_CS2, CS_ACTIVE_LOW ? HIGH : LOW);
}
static inline void cs_left(){
#if HAS_CS1
  // Выбрать левый чип (CS1)
  digitalWrite(PIN_CS1, CS_ACTIVE_LOW ? LOW : HIGH);
  digitalWrite(PIN_CS2, CS_ACTIVE_LOW ? HIGH: LOW);
#else
  // Нет CS1: левая половина = CS2 неактивен
  digitalWrite(PIN_CS2, CS_ACTIVE_LOW ? HIGH : LOW);
#endif
}
static inline void cs_right(){
#if HAS_CS1
  // Выбрать правый чип (CS2)
  digitalWrite(PIN_CS1, CS_ACTIVE_LOW ? HIGH: LOW);
  digitalWrite(PIN_CS2, CS_ACTIVE_LOW ? LOW : HIGH);
#else
  // Нет CS1: правая половина = CS2 активен
  digitalWrite(PIN_CS2, CS_ACTIVE_LOW ? LOW : HIGH);
#endif
}

// Команды ST7565/UC1701‑совм.
static inline void setPage(uint8_t page){
  wrCmd(0xB0 | (page & 0x0F));
}
static inline void setColumn(uint16_t col){
  col += COL_OFFSET;
  wrCmd(0x10 | ((col>>4) & 0x0F));
  wrCmd(0x00 | (col & 0x0F));
}

// ---------- Инициализация ----------
void lcd_init(){
  for (uint8_t i=0;i<8;i++) pinMode(LCD_DB[i], OUTPUT);
  pinMode(PIN_RS, OUTPUT);
  pinMode(PIN_E,  OUTPUT);
  pinMode(PIN_RST,OUTPUT);
#if HAS_CS1
  pinMode(PIN_CS1,OUTPUT);
#endif
  pinMode(PIN_CS2,OUTPUT);

  digitalWrite(PIN_E,LOW);
  cs_none();

  // Аппаратный сброс
  digitalWrite(PIN_RST, LOW); delay(20);
  digitalWrite(PIN_RST, HIGH); delay(20);

  // Включаем оба чипа по очереди
  auto init_chip = [](){
    wrCmd(0xAE);        // OFF
    wrCmd(0xE2);        // SW reset
    wrCmd(0x2C); delay(2); // Booster ON
    wrCmd(0x2E); delay(2); // Regulator ON
    wrCmd(0x2F); delay(8); // Follower ON
    // Ориентация (если зеркалит — поменяйте на 0xA1/0xC8)
    wrCmd(0xA0);        // ADC normal (SEG++)
    wrCmd(0xC0);        // SHL normal (COM++)
    // Контраст/делитель (подбирается)
    wrCmd(0xA2);        // Bias 1/9 (или 0xA3)
    wrCmd(0x25);        // Resistor ratio (0x24..0x26)
    wrCmd(0x81); wrCmd(0x1E);  // Контраст 0x00..0x3F
    wrCmd(0x40);        // Start line = 0
    wrCmd(0xAF);        // ON
  };

  cs_left();  init_chip();  cs_none();
  cs_right(); init_chip();  cs_none();
}

// Очистка экрана
void lcd_clear(){
  // Левая половина
  cs_left();
  for (uint8_t p=0;p<LCD_PAGES;p++){
    setPage(p); setColumn(0);
    for (uint16_t x=0; x<CHIP0_COLS; x++) wrData(0x00);
  }
  cs_none();
  // Правая половина
  cs_right();
  for (uint8_t p=0;p<LCD_PAGES;p++){
    setPage(p); setColumn(0);
    for (uint16_t x=0; x<CHIP1_COLS; x++) wrData(0x00);
  }
  cs_none();
}

// Записать один вертикальный байт в абсолютную колонку (0..LCD_W-1) и страницу (0/1)
void lcd_writeByteAbs(uint16_t col, uint8_t page, uint8_t b){
  if (page >= LCD_PAGES || col >= LCD_W) return;
  if (col < CHIP0_COLS){
    cs_left();
    setPage(page);
    setColumn(col);
    wrData(b);
    cs_none();
  } else {
    uint16_t c = col - CHIP0_COLS;
    if (c >= CHIP1_COLS) return;
    cs_right();
    setPage(page);
    setColumn(c);
    wrData(b);
    cs_none();
  }
}

// ---------- Мини-шрифт 5x7 (цифры, пробел, A‑Z) ----------
const uint8_t glyph5x7[][5] PROGMEM = {
  // 0..9
  {0x3E,0x45,0x49,0x51,0x3E},{0x00,0x21,0x7F,0x01,0x00},{0x21,0x43,0x45,0x49,0x31},
  {0x22,0x41,0x49,0x49,0x36},{0x0C,0x14,0x24,0x7F,0x04},{0x72,0x51,0x51,0x51,0x4E},
  {0x1E,0x29,0x49,0x49,0x06},{0x40,0x47,0x48,0x50,0x60},{0x36,0x49,0x49,0x49,0x36},
  {0x30,0x49,0x49,0x4A,0x3C},
  // space
  {0x00,0x00,0x00,0x00,0x00},
  // A‑Z
  {0x7E,0x11,0x11,0x11,0x7E},{0x7F,0x49,0x49,0x49,0x36},{0x3E,0x41,0x41,0x41,0x22},
  {0x7F,0x41,0x41,0x22,0x1C},{0x7F,0x49,0x49,0x49,0x41},{0x7F,0x09,0x09,0x09,0x01},
  {0x3E,0x41,0x49,0x49,0x7A},{0x7F,0x08,0x08,0x08,0x7F},{0x00,0x41,0x7F,0x41,0x00},
  {0x20,0x40,0x41,0x3F,0x01},{0x7F,0x08,0x14,0x22,0x41},{0x7F,0x40,0x40,0x40,0x40},
  {0x7F,0x02,0x0C,0x02,0x7F},{0x7F,0x04,0x08,0x10,0x7F},{0x3E,0x41,0x41,0x41,0x3E},
  {0x7F,0x09,0x09,0x09,0x06},{0x3E,0x41,0x51,0x21,0x5E},{0x7F,0x09,0x19,0x29,0x46},
  {0x46,0x49,0x49,0x49,0x31},{0x01,0x01,0x7F,0x01,0x01},{0x3F,0x40,0x40,0x40,0x3F},
  {0x1F,0x20,0x40,0x20,0x1F},{0x7F,0x20,0x18,0x20,0x7F},{0x63,0x14,0x08,0x14,0x63},
  {0x07,0x08,0x70,0x08,0x07},{0x61,0x51,0x49,0x45,0x43}
};
int glyphIndex(char c){
  if (c>='0'&&c<='9') return c-'0';
  if (c==' ') return 10;
  if (c>='A'&&c<='Z') return 11+(c-'A');
  if (c>='a'&&c<='z') return 11+(c-'a');
  return 10;
}

void drawChar(uint16_t x, uint8_t yPage, char c){
  if (yPage >= LCD_PAGES) return;
  int gi = glyphIndex(c);
  for (uint8_t col=0; col<5; col++){
    uint8_t bits = pgm_read_byte(&glyph5x7[gi][col]); // 7 px по вертикали
    lcd_writeByteAbs(x+col, yPage, bits);
  }
}
void drawText(uint16_t x, uint8_t yPage, const char* s){
  while (*s && x+5 < LCD_W){
    drawChar(x, yPage, *s++);
    x += 6; // 5 колонки + 1 пробел
  }
}

void setup(){
  lcd_init();
  lcd_clear();

  // ОБЯЗАТЕЛЬНО: крутите подстроечник VO на пине 3 до чёткой картинки.

  drawText(4, 0, "HELLO 150x16");
  drawText(4, 1, "TWO-CHIP LCD");
}

void loop(){}