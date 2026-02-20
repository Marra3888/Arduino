#include "LC75847.h"
#include <SPI.h>

LC75847::LC75847() {}

void LC75847::begin(int chipEnabledPin, int address) {
  _chipEnabledPin = chipEnabledPin;
  _address = address;
  pinMode(_chipEnabledPin, OUTPUT);
  digitalWrite(_chipEnabledPin, HIGH); // CE=HIGH — неактивно
  SPI.begin();
  clear(); // отправляем пустой кадр
}

void LC75847::clear() {
  // memset(Display_Data_0, 0, sizeof(Display_Data_0));
  // memset(Display_Data_1, 0, sizeof(Display_Data_1));
  // memset(Display_Data_2, 0, sizeof(Display_Data_2));
  // memset(Display_Data_3, 0, sizeof(Display_Data_3));

  memset(Display_Data_0, 0, 14);
  memset(Display_Data_1, 0, 14);
  memset(Display_Data_2, 0, 14);
  memset(Display_Data_3, 0, 14);

  // Устанавливаем коды блока (DD)
  // Display_Data_0[15] = 0b00000000; // DD=00
  // Display_Data_1[15] = 0b00000001; // DD=01
  // Display_Data_2[15] = 0b00000010; // DD=10
  // Display_Data_3[15] = 0b00000011; // DD=11

  _print(); // отправляем пустой кадр
}

void LC75847::_print() {
  // Блок 0
  digitalWrite(_chipEnabledPin, LOW);
  SPI.transfer(_address);
  digitalWrite(_chipEnabledPin, HIGH);
  for (byte i = 0; i < 16; i++) {
    SPI.transfer(Display_Data_0[i]);
  }

  // Блок 1
  digitalWrite(_chipEnabledPin, LOW);
  SPI.transfer(_address);
  digitalWrite(_chipEnabledPin, HIGH);
  for (byte i = 0; i < 16; i++) {
    SPI.transfer(Display_Data_1[i]);
  }

  // Блок 2
  digitalWrite(_chipEnabledPin, LOW);
  SPI.transfer(_address);
  digitalWrite(_chipEnabledPin, HIGH);
  for (byte i = 0; i < 16; i++) {
    SPI.transfer(Display_Data_2[i]);
  }

  // Блок 3
  digitalWrite(_chipEnabledPin, LOW);
  SPI.transfer(_address);
  digitalWrite(_chipEnabledPin, HIGH);
  for (byte i = 0; i < 16; i++) {
    SPI.transfer(Display_Data_3[i]);
  }
  digitalWrite(_chipEnabledPin, LOW);
  delay(_textSpeed);
}

void LC75847::display() {
  _print();
}

void LC75847::setD(uint16_t Dn, bool on) {
  if (Dn < 1 || Dn > 420) return;

  uint8_t* blk;
  uint16_t startD;
  int pos;

  if (Dn <= 108) 
  {
    blk = Display_Data_0;
    startD = 1;
  } 
  else if (Dn <= 212) 
  {
    blk = Display_Data_1;
    startD = 109;
  } 
  else if (Dn <= 316) 
  {
    blk = Display_Data_2;
    startD = 213;
  } 
  else 
  {
    blk = Display_Data_3;
    startD = 317;
  }

  // Расчёт позиции (простой, без проверки)
  pos = Dn - startD;

  // Установка бита (оставляет другие)
  int bi = pos >> 3; // байт
  int bb = pos & 7; // бит в байте (LSB-first внутри байта)
  if (on) {
    blk[bi] |= (1 << bb); // Включаем бит, оставляем другие
  } else {
    blk[bi] &= ~(1 << bb); // Выключаем бит, оставляем другие
  }
}