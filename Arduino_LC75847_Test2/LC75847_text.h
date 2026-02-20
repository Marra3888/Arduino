// #pragma once
// #include "LC75847.h"

// // Кол-во 19-сегментных ячеек в твоём поле (поменяй под свой дисплей)
// #define NUM_CELLS 14

// // Нумерация логических сегментов внутри одной ячейки
// // 0..18: A, B1, B2, C1, C2, D, E1, E2, F1, F2, G1, G2, G3, H1, H2, H3, I1, I2, I3
// enum Seg19 : uint8_t {
//   SA, SB1, SB2, SC1, SC2, SD, SE1, SE2, SF1, SF2, SG1, SG2, SG3, SH1, SH2, SH3, SI1, SI2, SI3, SEG19_COUNT
// };

// #define SEG_BIT(s) (1UL << (s))

// // Карта D‑номеров для каждой ячейки и каждого из 19 сегментов.
// // Заполни сам! 0 = сегмент отсутствует/не задействован.
// extern const uint16_t CELL_MAP[NUM_CELLS][SEG19_COUNT];

// class LC75847 {
// public:
//   // уже есть: begin(), clear(), display(), setD()

//   // Выставить набор сегментов ячейки одним 19-битным маском
//   void setCellMask(uint8_t cell, uint32_t mask19);

//   // Напечатать 1 символ в указанной ячейке
//   void putChar(uint8_t cell, char ch);

//   // Напечатать строку начиная с firstCell. Если clearTail=true — очистит
//   // оставшиеся ячейки справа.
//   void printText(uint8_t firstCell, const char* s, bool clearTail = true);

//   // Очистить диапазон ячеек
//   void clearCells(uint8_t firstCell, uint8_t count);

// private:
//   // 7‑сегментный шрифт (a,b,c,d,e,f,g) -> битовая маска 7 бит
//   static uint8_t font7(char ch);

//   // Преобразование 7‑сегментной маски в 19‑сегментную
//   static uint32_t map7to19(uint8_t m7);

//   // При желании можно сделать «родной» 19‑сегментный глиф для некоторых букв
//   static uint32_t glyph19(char ch); // сначала пытается 19‑seg, иначе fallback в 7‑seg
// };