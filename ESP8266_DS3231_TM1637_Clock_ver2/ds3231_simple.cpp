#include "ds3231_simple.h"

bool DS3231_Simple::begin(int sda, int scl, uint32_t i2cClock) {
  if (sda >= 0 && scl >= 0) {
    Wire.begin(sda, scl);
  } // иначе считаем, что Wire уже инициализирован
  Wire.setClock(i2cClock);
  return ping();
}

bool DS3231_Simple::ping() {
  Wire.beginTransmission(I2C_ADDR);
  return (Wire.endTransmission() == 0);
}

bool DS3231_Simple::readBytes(uint8_t reg, uint8_t *buf, uint8_t len) {
  if (!buf || !len) return false;

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0)  // repeated START
    return false;

  uint8_t n = Wire.requestFrom((uint8_t)I2C_ADDR, (uint8_t)len, (uint8_t)true);
  if (n < len) return false;

  uint32_t t0 = millis();
  for (uint8_t i=0; i<len; ) {
    if (Wire.available()) {
      buf[i++] = Wire.read();
    } else {
      if (millis() - t0 > 20) return false;
      yield();
    }
  }
  return true;
}

bool DS3231_Simple::writeBytes(uint8_t reg, const uint8_t *buf, uint8_t len) {
  if (!buf || !len) return false;
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  Wire.write(buf, len);
  return (Wire.endTransmission(true) == 0);
}

bool DS3231_Simple::readTime(DateTime &t) {
  uint8_t b[7];
  if (!readBytes(REG_SEC, b, 7)) return false;

  uint8_t sec = b[0] & 0x7F;
  uint8_t min = b[1] & 0x7F;
  uint8_t hr  = b[2];

  // 12/24 разбор
  if (hr & 0x40) {
    // 12-hour mode
    uint8_t h12 = bcd2bin(hr & 0x1F); // 1..12
    bool pm = (hr & 0x20) != 0;
    if (h12 == 12) h12 = 0;           // 12AM -> 0
    if (pm) h12 += 12;                // PM -> +12 (12PM уже 12)
    t.hour = h12;
  } else {
    // 24-hour
    t.hour = bcd2bin(hr & 0x3F);
  }

  t.second = bcd2bin(sec);
  t.minute = bcd2bin(min);

  uint8_t mon = b[5];
  uint16_t y  = 2000 + bcd2bin(b[6]);
  if (mon & 0x80) y += 100;           // century

  t.year  = y;
  t.month = bcd2bin(mon & 0x1F);
  t.day   = bcd2bin(b[4] & 0x3F);

  return true;
}

bool DS3231_Simple::writeTime(const DateTime &t) {
  if (t.month < 1 || t.month > 12) return false;
  if (t.day   < 1 || t.day   > 31) return false;
  if (t.hour  > 23 || t.minute > 59 || t.second > 59) return false;

  uint8_t b[7];
  b[0] = bin2bcd(t.second);
  b[1] = bin2bcd(t.minute);
  b[2] = bin2bcd(t.hour) & 0x3F;   // 24h
  b[3] = 0x01;                     // DOW (не используем, пусть 1)
  b[4] = bin2bcd(t.day);
  uint8_t mon = bin2bcd(t.month);
  uint16_t y  = t.year;
  if (y >= 2100) { mon |= 0x80; y -= 100; } // century bit
  b[5] = mon;
  b[6] = bin2bcd((uint8_t)(y - 2000));

  return writeBytes(REG_SEC, b, 7);
}

float DS3231_Simple::readTemperature() {
  uint8_t b[2];
  if (!readBytes(REG_TEMP_MSB, b, 2)) return NAN;
  int8_t msb = (int8_t)b[0];
  float frac = (b[1] >> 6) * 0.25f;
  return (float)msb + frac;
}

bool DS3231_Simple::setClockHalt(bool halt) {
  uint8_t c;
  if (!readBytes(REG_CONTROL, &c, 1)) return false;
  if (halt) c |= 0x80; else c &= ~0x80; // EOSC
  return writeBytes(REG_CONTROL, &c, 1);
}

bool DS3231_Simple::clearOSF() {
  uint8_t s;
  if (!readBytes(REG_STATUS, &s, 1)) return false;
  s &= ~0x80; // OSF=0
  return writeBytes(REG_STATUS, &s, 1);
}

bool DS3231_Simple::isOSFSet(bool &set) {
  uint8_t s;
  if (!readBytes(REG_STATUS, &s, 1)) return false;
  set = (s & 0x80) != 0;
  return true;
}

bool DS3231_Simple::set24h() {
  // Считываем час, снимаем бит 6 (12h), корректируем, если надо
  uint8_t hr;
  if (!readBytes(REG_HOUR, &hr, 1)) return false;
  if (hr & 0x40) {
    uint8_t h12 = bcd2bin(hr & 0x1F); // 1..12
    bool pm = (hr & 0x20) != 0;
    if (h12 == 12) h12 = 0;
    if (pm) h12 += 12;
    hr = bin2bcd(h12) & 0x3F;         // 24h формат
    return writeBytes(REG_HOUR, &hr, 1);
  }
  return true; // уже 24ч
}