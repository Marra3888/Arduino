// Подключение по SPI программным способом (Software SPI)
#define UC1608_SCLK 9
#define UC1608_SID  10
#define UC1608_CS   2
#define UC1608_A0   5
#define UC1608_RST  4
#define UC1608_LED  8

void uc1608_send(bool isData, uint8_t data) {
  digitalWrite(UC1608_CS, LOW);
  digitalWrite(UC1608_A0, isData); // 0 = команда, 1 = данные
  for (int i = 0; i < 8; i++) {
    digitalWrite(UC1608_SCLK, LOW);
    digitalWrite(UC1608_SID, (data & 0x80) ? HIGH : LOW);
    data <<= 1;
    digitalWrite(UC1608_SCLK, HIGH);
  }
  digitalWrite(UC1608_CS, HIGH);
}

void uc1608_init() {
  digitalWrite(UC1608_RST, LOW);
  delay(50);
  digitalWrite(UC1608_RST, HIGH);
  delay(50);

  // Пример инициализации UC1608
  uc1608_send(0, 0xE2); // Software reset
  uc1608_send(0, 0x2C); // Boost ON
  uc1608_send(0, 0x2E); // Voltage regulator ON
  uc1608_send(0, 0x2F); // Voltage follower ON
  delay(50);
  uc1608_send(0, 0x24); // Set contrast (регулируй под дисплей)
  uc1608_send(0, 0x81); // Set contrast control register
  uc1608_send(0, 0x1F); // Контраст (0x00–0x3F)
  uc1608_send(0, 0xA0); // ADC normal
  uc1608_send(0, 0xC0); // SHL normal
  uc1608_send(0, 0xAF); // Display ON
}

void setup() {
  pinMode(UC1608_SCLK, OUTPUT);
  pinMode(UC1608_SID, OUTPUT);
  pinMode(UC1608_CS, OUTPUT);
  pinMode(UC1608_A0, OUTPUT);
  pinMode(UC1608_RST, OUTPUT);
  pinMode(UC1608_LED, OUTPUT);

  digitalWrite(UC1608_LED, HIGH); // Подсветка

  uc1608_init();
}

void loop() {
  static uint8_t x = 0;

  // Очистить дисплей (только первую страницу 0)
  uc1608_send(0, 0xB0); // Page 0
  uc1608_send(0, 0x10); // Column upper 4 bits = 0
  uc1608_send(0, 0x00); // Column lower 4 bits = 0

  for (int i = 0; i < 134; i++) {
    if (i == x)
      uc1608_send(1, 0xFF); // Пиксель ON (вертикальная линия)
    else
      uc1608_send(1, 0x00); // Пиксель OFF
  }

  x = (x + 1) % 134;
  delay(1000);
}

