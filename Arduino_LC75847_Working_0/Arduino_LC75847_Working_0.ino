#include <SPI.h>
#include <ctype.h>

// ===== Интерфейс =====
#define CE_PIN       10
#define LC_ADDRESS   0xA1
#define SPI_MODE_SEL SPI_MODE3
#define SPI_SPEED    200000

// ===== Поле =====
#define NUM_CELLS 16

enum Seg19 : uint8_t {
  SA, SB1, SB2, SC1, SC2, SD, SE1, SE2, SF1, SF2,
  SG1, SG2, SG3, SH1, SH2, SH3, SI1, SI2, SI3, SEG19_COUNT
};
#define SEG_BIT(s) (1UL << (s))

// ===== Карта D-номеров =====
// Порядок строк: логическая 0 = самая левая (15),
// логическая 15 = самая правая (0)
const uint16_t CELL_MAP[NUM_CELLS][SEG19_COUNT] = {
  // Порядок: A ,B1, B2, C1, C2, D,  E1, E2, F1, F2, G1, G2, G3, H1, H2, H3, I1, I2, I3
  {  45, 33, 34, 35, 36, 44, 52, 51, 50, 53, 55, 42, 47, 41, 46, 54, 43, 48, 56 },  // 15 (левая)
  {  57, 61, 62, 63, 64, 72, 80, 79, 78, 65, 67, 70, 59, 69, 58, 66, 71, 60, 68 },  // 14
  {  85, 73, 74, 75, 76, 84, 92, 91, 90, 93, 95, 82, 87, 81, 86, 94, 83, 88, 96 },  // 13
  {  97,101,102,103,104,  0,112,111,110,113,115,  0, 99,  0, 98,114,  0,100,116 },  // 12
  { 117,121,122,123,124,132,140,139,138,125,127,130,119,129,118,126,131,120,128 },  // 11
  { 145,133,134,135,136,144,156,151,150,149,154,142,147,141,146,153,143,148,155 },  // 10
  { 157,161,162,163,164,172,180,179,178,165,167,170,159,169,158,166,171,160,168 },  // 9
  { 185,173,174,175,176,184,192,191,190,193,195,182,187,181,186,194,183,188,196 },  // 8
  { 197,201,202,203,204,212,220,219,218,205,207,210,199,209,198,206,211,200,208 },  // 7
  { 225,213,214,215,216,224,232,231,230,233,235,222,227,221,226,234,223,228,236 },  // 6
  { 237,241,242,243,244,252,260,259,258,245,247,250,239,249,238,246,251,240,248 },  // 5
  { 265,253,254,255,256,264,272,271,270,273,275,262,267,261,266,274,263,268,276 },  // 4
  { 277,281,282,283,284,292,300,299,298,285,287,290,279,289,278,286,291,280,288 },  // 3
  { 305,293,294,295,296,304,312,311,310,313,315,302,307,301,306,314,303,308,316 },  // 2
  { 317,321,322,323,324,332,340,339,338,325,327,330,319,329,318,326,331,320,328 },  // 1
  { 345,333,334,335,336,344,352,351,350,353,355,342,347,341,346,354,343,348,356 }   // 0 (правая)
};

// ===== Класс драйвера =====
class LC75847 {
public:
  // Хвосты: [14] = 0x00, [15] = DD (00/01/10/11)
  byte Display_Data_0[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00, 0x00}; // DD=00
  byte Display_Data_1[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00, 0x01}; // DD=01
  byte Display_Data_2[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00, 0x02}; // DD=10
  byte Display_Data_3[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00, 0x03}; // DD=11

  void begin(int chipEnabledPin, int address) {
    _chipEnabledPin = chipEnabledPin;
    _address = address;

    pinMode(_chipEnabledPin, OUTPUT);
    digitalWrite(_chipEnabledPin, HIGH); // CE HIGH в простое

    SPI.begin();
    SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE_SEL));

    clear();
  }

  // Очистка только D‑бит (не хвостов)
  void clear() {
    memset(Display_Data_0, 0, 14);
    memset(Display_Data_1, 0, 15);
    memset(Display_Data_2, 0, 15);
    memset(Display_Data_3, 0, 15);
    _print();
  }

  void display() { _print(); }

  // ===== Печать текста (16 ячеек) =====
  // firstCell — логический индекс 0..15 (0 — крайняя левая)
  void printText(uint8_t firstCell, const char* s, bool clearTail = true) {
    if (!s) return;
    uint8_t logical = firstCell;
    while (*s && logical < NUM_CELLS) {
      putChar(logical++, *s++);
    }
    if (clearTail && logical < NUM_CELLS)
      clearCells(logical, NUM_CELLS - logical);
    display();
  }

  void putChar(uint8_t logicalCell, char ch) {
    setCellMask(logicalCell, glyph19(ch));
  }

  void clearCells(uint8_t firstLogical, uint8_t count) {
    for (uint8_t i = 0; i < count && (firstLogical + i) < NUM_CELLS; ++i)
      setCellMask(firstLogical + i, 0);
  }

  // Тест: все 16 ячеек по карте
  void lightAll(bool on) {
    for (uint8_t phys = 0; phys < NUM_CELLS; ++phys) {
      const uint16_t* row = CELL_MAP[phys];
      for (uint8_t s = 0; s < SEG19_COUNT; ++s) {
        uint16_t dn = row[s];
        if (!dn) continue;
        setD(dn, on);
      }
    }
    display();
  }

  // Низкоуровневый тест по D1..D420
  void lightAllD(bool on) {
    for (uint16_t d = 1; d <= 420; ++d) setD(d, on);
    display();
  }

  void setD(uint16_t Dn, bool on) {
    if (Dn < 1 || Dn > 420) return;

    uint8_t* blk;
    uint16_t startD;
    if (Dn <= 108)      { blk = Display_Data_0; startD = 1;   }
    else if (Dn <= 212) { blk = Display_Data_1; startD = 109; }
    else if (Dn <= 316) { blk = Display_Data_2; startD = 213; }
    else                { blk = Display_Data_3; startD = 317; }

    uint16_t pos = (uint16_t)(Dn - startD);
    uint8_t  bi  = pos >> 3;
    uint8_t  bb  = pos & 7;
    if (bi > 13) return;   // только байты 0..13

    if (on)  blk[bi] |=  (uint8_t)(1 << bb);
    else     blk[bi] &= (uint8_t)~(1 << bb);
  }

  // Логический индекс 0..15 -> строка CELL_MAP с таким же индексом
  // 0 (крайняя левая) -> CELL_MAP[0]
  void setCellMask(uint8_t logicalCell, uint32_t mask19) {
    if (logicalCell >= NUM_CELLS) return;
    const uint16_t* row = CELL_MAP[logicalCell];  // БЕЗ инверсии
    for (uint8_t s = 0; s < SEG19_COUNT; ++s) {
      uint16_t dn = row[s];
      if (!dn) continue;
      setD(dn, (mask19 & SEG_BIT(s)) != 0);
    }
  }

private:
  int _chipEnabledPin = CE_PIN;
  int _address = LC_ADDRESS;
  int _textSpeed = 2;

  // Передача блоков: CE=LOW -> адрес; CE=HIGH -> 16 байт
  void _print() {
    // Блок 0
    digitalWrite(_chipEnabledPin, LOW);
    SPI.transfer((uint8_t)_address);
    digitalWrite(_chipEnabledPin, HIGH);
    for (uint8_t i = 0; i < 16; ++i) SPI.transfer(Display_Data_0[i]);

    // Блок 1
    digitalWrite(_chipEnabledPin, LOW);
    SPI.transfer((uint8_t)_address);
    digitalWrite(_chipEnabledPin, HIGH);
    for (uint8_t i = 0; i < 16; ++i) SPI.transfer(Display_Data_1[i]);

    // Блок 2
    digitalWrite(_chipEnabledPin, LOW);
    SPI.transfer((uint8_t)_address);
    digitalWrite(_chipEnabledPin, HIGH);
    for (uint8_t i = 0; i < 16; ++i) SPI.transfer(Display_Data_2[i]);

    // Блок 3
    digitalWrite(_chipEnabledPin, LOW);
    SPI.transfer((uint8_t)_address);
    digitalWrite(_chipEnabledPin, HIGH);
    for (uint8_t i = 0; i < 16; ++i) SPI.transfer(Display_Data_3[i]);

    digitalWrite(_chipEnabledPin, LOW); // завершить кадр
    delay(_textSpeed);
  }

  // 7‑seg -> 19‑seg
  static inline uint8_t B7(uint8_t a,uint8_t b,uint8_t c,uint8_t d,uint8_t e,uint8_t f,uint8_t g){
    return (a<<6)|(b<<5)|(c<<4)|(d<<3)|(e<<2)|(f<<1)|g;
  }
  static uint8_t font7(char ch) {
    ch = toupper((unsigned char)ch);
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
  static uint32_t map7to19(uint8_t m7) {
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
  static uint32_t glyph19(char ch) {
    ch = toupper((unsigned char)ch);
    switch (ch) {
      case 'K': return SEG_BIT(SF1)|SEG_BIT(SF2)|SEG_BIT(SB1)|SEG_BIT(SB2)|
                       SEG_BIT(SH1)|SEG_BIT(SH2)|SEG_BIT(SI2);
      case 'V': return SEG_BIT(SF1)|SEG_BIT(SF2)|SEG_BIT(SB1)|SEG_BIT(SB2)|
                       SEG_BIT(SI2)|SEG_BIT(SI3);
      default:  return map7to19(font7(ch));
    }
  }
};

// ===== Глобальный объект =====
LC75847 lcd;

void setup() {
  Serial.begin(115200);
  lcd.begin(CE_PIN, LC_ADDRESS);

  // Пример: вывести цифры 0..F по всей строке слева направо
  lcd.printText(0, "0123456789ABCDEF");
}

void loop() {
}