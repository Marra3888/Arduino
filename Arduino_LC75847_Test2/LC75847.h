#ifndef LC75847_h
#define LC75847_h

#include <Arduino.h>

// Поменяй на 0, если у тебя CE активный LOW.
#define CE_ACTIVE_HIGH 1

// Кол-во 19‑сегм. ячеек (по твоей карте)
#define NUM_CELLS 17

// Логические сегменты одной ячейки
enum Seg19 : uint8_t {
  SA, SB1, SB2, SC1, SC2, SD, SE1, SE2, SF1, SF2,
  SG1, SG2, SG3, SH1, SH2, SH3, SI1, SI2, SI3, SEG19_COUNT
};
#define SEG_BIT(s) (1UL << (s))

// Карта D‑номеров (реализация в .cpp)
extern const uint16_t CELL_MAP[NUM_CELLS][SEG19_COUNT];

class LC75847 {
public:
  // 128 бит на блок. [14] = 0x00, [15] = DD (00/01/10/11).
  byte Display_Data_0[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00, 0x00 }; // DD=00
  byte Display_Data_1[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00, 0x01 }; // DD=01
  byte Display_Data_2[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00, 0x02 }; // DD=10
  byte Display_Data_3[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00, 0x03 }; // DD=11

  LC75847();
  void begin(int chipEnabledPin, int address);
  void clear();     // блок0: 13.5; блоки1..3: 14
  void display();
  void _print();

  void setD(uint16_t Dn, bool on);

  // Вывод текста
  void setCellMask(uint8_t cell, uint32_t mask19);
  void putChar(uint8_t cell, char ch);
  void printText(uint8_t firstCell, const char* s, bool clearTail = true);
  void clearCells(uint8_t firstCell, uint8_t count);

  // Тест
  void lightAll(bool on);

private:
  int _chipEnabledPin = 10;
  int _address = 0xA1;
  int _textSpeed = 2;

  inline void _ceSelect();
  inline void _ceDeselect();

  static uint8_t  font7(char ch);
  static uint32_t map7to19(uint8_t m7);
  static uint32_t glyph19(char ch);
};

#endif