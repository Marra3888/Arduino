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
// 0 – ПРАВАЯ ячейка, 15 – ЛЕВАЯ
const uint16_t CELL_MAP[NUM_CELLS][SEG19_COUNT] = {
  // 0 (правая)
  {  45, 33, 34, 35, 36, 44, 52, 51, 50, 53, 55, 42, 47, 41, 46, 54, 43, 48, 56 },
  // 1
  {  57, 61, 62, 63, 64, 72, 80, 79, 78, 65, 67, 70, 59, 69, 58, 66, 71, 60, 68 },
  // 2
  {  85, 73, 74, 75, 76, 84, 92, 91, 90, 93, 95, 82, 87, 81, 86, 94, 83, 88, 96 },
  // 3
  {  97,101,102,103,104,  0,112,111,110,113,115,  0, 99,  0, 98,114,  0,100,116 },
  // 4
  { 117,121,122,123,124,132,140,139,138,125,127,130,119,129,118,126,131,120,128 },
  // 5
  { 145,133,134,135,136,144,156,151,150,149,154,142,147,141,146,153,143,148,155 },
  // 6
  { 157,161,162,163,164,172,180,179,178,165,167,170,159,169,158,166,171,160,168 },
  // 7
  { 185,173,174,175,176,184,192,191,190,193,195,182,187,181,186,194,183,188,196 },
  // 8
  { 197,201,202,203,204,212,220,219,218,205,207,210,199,209,198,206,211,200,208 },
  // 9
  { 225,213,214,215,216,224,232,231,230,233,235,222,227,221,226,234,223,228,236 },
  // 10
  { 237,241,242,243,244,252,260,259,258,245,247,250,239,249,238,246,251,240,248 },
  // 11
  { 265,253,254,255,256,264,272,271,270,273,275,262,267,261,266,274,263,268,276 },
  // 12
  { 277,281,282,283,284,292,300,299,298,285,287,290,279,289,278,286,291,280,288 },
  // 13
  { 305,293,294,295,296,304,312,311,310,313,315,302,307,301,306,314,303,308,316 },
  // 14
  { 318,322,323,324,325,333,341,340,339,326,328,331,320,330,319,327,332,321,329 },
  // 15 (левая)
  { 346,334,335,336,337,345,353,352,351,354,356,343,348,342,347,355,344,349,357 }
};

// ===== Класс драйвера =====
class LC75847 {
public:
  // Хвосты: [14] = 0x00, [15] = DD (00/01/10/11)
  byte Display_Data_0[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00, 0x00}; // DD=00
  byte Display_Data_1[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00, 0x01}; // DD=01
  byte Display_Data_2[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00, 0x02}; // DD=10
  byte Display_Data_3[16] = {0,0,0,0,0,0,0,0,0,0,0,0b11111111,0b11111111,0b11111111, 0x00, 0x03}; // DD=11

  void begin(int chipEnabledPin, int address) {
    _chipEnabledPin = chipEnabledPin;
    _address = address;

    pinMode(_chipEnabledPin, OUTPUT);
    digitalWrite(_chipEnabledPin, HIGH); // CE HIGH в простое

    SPI.begin();
    SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE_SEL));

    // clear();
  }

  void clear() {
    memset(Display_Data_0, 0, 14);
    memset(Display_Data_1, 0, 15);
    memset(Display_Data_2, 0, 15);
    memset(Display_Data_3, 0, 15);
    _print();
  }

  void display() { _print(); }

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
    if (bi > 13) return;

    if (on)  blk[bi] |=  (uint8_t)(1 << bb);
    else     blk[bi] &= (uint8_t)~(1 << bb);
  }

  // Логический 0..15 -> физический 15..0, как у тебя
  void setCellMask(uint8_t logicalCell, uint32_t mask19) {
    if (logicalCell >= NUM_CELLS) return;
    uint8_t phys = (uint8_t)(NUM_CELLS - 1 - logicalCell);
    const uint16_t* row = CELL_MAP[phys];
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

  void _print() {
    // Блок 0
    digitalWrite(_chipEnabledPin, LOW);
    SPI.transfer((uint8_t)_address);
    digitalWrite(_chipEnabledPin, HIGH);
    for (uint8_t i = 0; i < 16; ++i) 
    {
      SPI.transfer(Display_Data_0[i]);
        //     Serial.print(Display_Data_0[i], BIN);
        // Serial.print("    i = ");
        //  Serial.println(i, DEC);
    }

    // Блок 1
    digitalWrite(_chipEnabledPin, LOW);
    SPI.transfer((uint8_t)_address);
    digitalWrite(_chipEnabledPin, HIGH);
    for (uint8_t i = 0; i < 16; ++i) 
    {
      SPI.transfer(Display_Data_1[i]);
        //     Serial.print(Display_Data_1[i], BIN);
        // Serial.print("    i = ");
        //  Serial.println(i, DEC);
    }

    // Блок 2
    digitalWrite(_chipEnabledPin, LOW);
    SPI.transfer((uint8_t)_address);
    digitalWrite(_chipEnabledPin, HIGH);
    for (uint8_t i = 0; i < 16; ++i) 
    {
      SPI.transfer(Display_Data_2[i]);
        //     Serial.print(Display_Data_2[i], BIN);
        // Serial.print("    i = ");
        //  Serial.println(i, DEC);
    }

    // Блок 3
    digitalWrite(_chipEnabledPin, LOW);
    SPI.transfer((uint8_t)_address);
    digitalWrite(_chipEnabledPin, HIGH);
    for (uint8_t i = 0; i < 16; ++i) 
    {
      SPI.transfer(Display_Data_3[i]);
      //  Serial.print(Display_Data_3[i], BIN);
      //   Serial.print("    i = ");
      //    Serial.println(i, DEC);
    }
 digitalWrite(_chipEnabledPin, LOW);
    delay(_textSpeed);
  }
};

// ===== Глобальный объект =====
LC75847 lcd;

void setup() {
  Serial.begin(115200);
  lcd.begin(CE_PIN, LC_ADDRESS);

  // 1. Чистим
  lcd.clear();
  delay(2000);
    // byte Display_Data_3[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0b11111111, 0x00, 0x03}; // DD=11


  // 2. Маска "все 19 сегментов"
  uint32_t fullMask = 0;
  for (uint8_t i = 0; i < SEG19_COUNT; ++i) {
    fullMask |= SEG_BIT(i);
  }

  // Инвертированная маска в пределах 19 бит
  // (~fullMask) инвертирует все 32 бита, поэтому маску режем до 19:
  uint32_t invMask = (~fullMask) & ((1UL << SEG19_COUNT) - 1);

  // 3. Зажечь ВСЕ сегменты в двух КРАЙНИХ ЛЕВЫХ логических ячейках:
  // логическая 0 -> phys 15 (левая)
  // логическая 1 -> phys 14
  lcd.setCellMask(0, fullMask);
  // lcd.setCellMask(1, fullMask);
  // lcd.setCellMask(0, invMask);
  // lcd.setCellMask(1, invMask);
  // lcd.setCellMask(2, fullMask);
  lcd.display();

  Serial.println("DONE: leftmost 2 cells should be FULL ON");

  while (true) {
    // держим картинку
  }
}

void loop() {}