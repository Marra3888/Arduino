#include "LC75847.h"
#include <SPI.h>
#include <ctype.h>

// Твоя карта D-номеров
const uint16_t CELL_MAP[NUM_CELLS][SEG19_COUNT] = {
  {  45, 33, 34, 35, 36, 44, 52, 51, 50, 53, 55, 42, 47, 41, 46, 54, 43, 48, 56 },
  {  57, 61, 62, 63, 64, 72, 80, 79, 78, 65, 67, 70, 59, 69, 58, 66, 71, 60, 68 },
  {  85, 73, 74, 75, 76, 84, 92, 91, 90, 93, 95, 82, 87, 81, 86, 94, 83, 88, 96 },
  {  97,101,102,103,104,  0,112,111,110,113,115,  0, 99,  0, 98,114,  0,100,116 },
  { 117,121,122,123,124,132,140,139,138,125,127,130,119,129,118,126,131,120,128 },
  { 145,133,134,135,136,144,156,151,150,149,154,142,147,141,146,153,143,148,155 },
  { 157,161,162,163,164,172,180,179,178,165,167,170,159,169,158,166,171,160,168 },
  { 185,173,174,175,176,184,192,191,190,193,195,182,187,181,186,194,183,188,196 },
  { 197,201,202,203,204,212,220,219,218,205,207,210,199,209,198,206,211,200,208 },
  { 225,213,214,215,216,224,232,231,230,233,235,222,227,221,226,234,223,228,236 },
  { 237,241,242,243,244,252,260,259,258,245,247,250,239,249,238,246,251,240,248 },
  { 265,253,254,255,256,264,272,271,270,273,275,262,267,261,266,274,263,268,276 },
  { 277,281,282,283,284,292,300,299,298,285,287,290,279,289,278,286,291,280,288 },
  { 305,293,294,295,296,304,312,311,310,313,315,302,307,301,306,314,303,308,316 },
  { 318,322,323,324,325,333,341,340,339,326,328,331,320,330,319,327,332,321,329 },
  { 346,334,335,336,337,345,353,352,351,354,356,343,348,342,347,355,344,349,357 },
  {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }
};

LC75847::LC75847() {}

void LC75847::begin(int chipEnabledPin, int address) {
  _chipEnabledPin = chipEnabledPin;
  _address = address;

  pinMode(_chipEnabledPin, OUTPUT);
#if CE_ACTIVE_HIGH
  digitalWrite(_chipEnabledPin, LOW);   // неактивен (активный HIGH)
#else
  digitalWrite(_chipEnabledPin, HIGH);  // неактивен (активный LOW)
#endif

  SPI.begin();
  // Ставим режим SPI максимально совместимый с CCB (обычно MODE0, MSB first)
  SPI.beginTransaction(SPISettings(200000, MSBFIRST, SPI_MODE0));

  clear(); // очистим и отрисуем
}

inline void LC75847::_ceSelect() {
#if CE_ACTIVE_HIGH
  digitalWrite(_chipEnabledPin, HIGH);
#else
  digitalWrite(_chipEnabledPin, LOW);
#endif
}
inline void LC75847::_ceDeselect() {
#if CE_ACTIVE_HIGH
  digitalWrite(_chipEnabledPin, LOW);
#else
  digitalWrite(_chipEnabledPin, HIGH);
#endif
}

void LC75847::clear() {
  // Блок 0: 13,5 — 13 байт полностью и младшая тетрада byte[13]
  memset(Display_Data_0, 0, 13);  // 0..12
  Display_Data_0[13] &= 0xF0;     // очистить D105..D108 (младшие 4 бита)

  // Блоки 1..3: 14 байт
  memset(Display_Data_1, 0, 14);
  memset(Display_Data_2, 0, 14);
  memset(Display_Data_3, 0, 14);

  _print();
}

void LC75847::_print() {
  uint8_t* blk[4] = { Display_Data_0, Display_Data_1, Display_Data_2, Display_Data_3 };

  for (uint8_t b = 0; b < 4; ++b) {
    _ceSelect();                               // CE активен — приём
    SPI.transfer((uint8_t)_address);           // адрес (0xA1)
    for (uint8_t i = 0; i < 16; ++i) {
      SPI.transfer(blk[b][i]);                 // 128 бит: D-данные + хвост
    }
    _ceDeselect();                             // защёлкнуть блок
  }
  delay(_textSpeed);
}

void LC75847::display() { _print(); }

void LC75847::setD(uint16_t Dn, bool on) {
  if (Dn < 1 || Dn > 420) return;

  uint8_t* blk;
  uint16_t startD;
  if (Dn <= 108)      { blk = Display_Data_0; startD = 1;   }
  else if (Dn <= 212) { blk = Display_Data_1; startD = 109; }
  else if (Dn <= 316) { blk = Display_Data_2; startD = 213; }
  else                { blk = Display_Data_3; startD = 317; }

  uint16_t pos = (uint16_t)(Dn - startD); // 0..107
  uint8_t  bi  = pos >> 3;                // 0..13
  uint8_t  bb  = pos & 7;                 // 0..7 (LSB в байте = Dn с меньшим номером)

  if (bi > 13) return;                    // защита
  if (on)  blk[bi] |=  (uint8_t)(1 << bb);
  else     blk[bi] &= (uint8_t)~(1 << bb);
}

void LC75847::lightAll(bool on) {
  for (uint16_t d = 1; d <= 420; ++d) setD(d, on);
  display();
}

/************ Текст ************/
#define B7(a,b,c,d,e,f,g) ((uint8_t)((a<<6)|(b<<5)|(c<<4)|(d<<3)|(e<<2)|(f<<1)|g))

static uint8_t font7map(char ch) {
  switch (ch) {
    case '0': return B7(1,1,1,1,1,1,0);
    case '1': return B7(0,1,1,0,0,0,0);
    case '2': return B7(1,1,0,1,1,0,1);
    case '3': return B7(1,1,1,1,0,0,1);
    case '4': return B7(0,1,1,0,0,1,1);
    case '5': return B7(1,0,1,1,0,1,1);
    case '6': return B7(1,0,1,1,1,1,1);
    case '7': return B7(1,1,1,0,0,0,0);
    case '8': return B7(1,1,1,1,1,1,1);
    case '9': return B7(1,1,1,1,0,1,1);
    case 'A': return B7(1,1,1,0,1,1,1);
    case 'B': case 'b': return B7(0,0,1,1,1,1,1);
    case 'C': return B7(1,0,0,1,1,1,0);
    case 'D': case 'd': return B7(0,1,1,1,1,0,1);
    case 'E': return B7(1,0,0,1,1,1,1);
    case 'F': return B7(1,0,0,0,1,1,1);
    case 'H': return B7(0,1,1,0,1,1,1);
    case 'J': return B7(0,1,1,1,1,0,0);
    case 'L': return B7(0,0,0,1,1,1,0);
    case 'N': return B7(0,1,1,0,1,1,0);
    case 'O': return B7(1,1,1,1,1,1,0);
    case 'P': return B7(1,1,0,0,1,1,1);
    case 'R': return B7(1,1,0,0,1,1,1);
    case 'S': return B7(1,0,1,1,0,1,1);
    case 'T': return B7(0,0,0,1,0,0,1);
    case 'U': return B7(0,1,1,1,1,1,0);
    case 'Y': return B7(0,1,1,1,0,1,1);
    case '-': return B7(0,0,0,0,0,0,1);
    case ' ': return 0;
    default : return 0;
  }
}

uint8_t LC75847::font7(char ch) { return font7map(toupper((unsigned char)ch)); }

// 7->19: a→A; b→B1|B2; c→C1|C2; d→D; e→E1|E2; f→F1|F2; g→G2
uint32_t LC75847::map7to19(uint8_t m7) {
  uint32_t m = 0;
  if (m7 & (1<<6)) m |= SEG_BIT(SA);
  if (m7 & (1<<5)) m |= SEG_BIT(SB1)|SEG_BIT(SB2);
  if (m7 & (1<<4)) m |= SEG_BIT(SC1)|SEG_BIT(SC2);
  if (m7 & (1<<3)) m |= SEG_BIT(SD);
  if (m7 & (1<<2)) m |= SEG_BIT(SE1)|SEG_BIT(SE2);
  if (m7 & (1<<1)) m |= SEG_BIT(SF1)|SEG_BIT(SF2);
  if (m7 & (1<<0)) m |= SEG_BIT(SG2);
  return m;
}

// Пара примеров 19‑seg букв; остальные — из 7‑seg
uint32_t LC75847::glyph19(char ch) {
  ch = toupper((unsigned char)ch);
  switch (ch) {
    case 'K': return SEG_BIT(SF1)|SEG_BIT(SF2)|SEG_BIT(SB1)|SEG_BIT(SB2)|
                     SEG_BIT(SH1)|SEG_BIT(SH2)|SEG_BIT(SI2);
    case 'V': return SEG_BIT(SF1)|SEG_BIT(SF2)|SEG_BIT(SB1)|SEG_BIT(SB2)|
                     SEG_BIT(SI2)|SEG_BIT(SI3);
    default:  return map7to19(font7(ch));
  }
}