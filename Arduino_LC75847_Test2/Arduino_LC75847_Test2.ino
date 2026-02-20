#include "LC75847.h"

const byte CCB_address = 0xA1;
#define SS_PIN 10   // CE

LC75847 lcd;

void setup() {
  Serial.begin(115200);
  pinMode(3, INPUT_PULLUP);

  lcd.begin(SS_PIN, CCB_address);

  // Тест: все сегменты ON 0.5с, затем OFF
  lcd.lightAll(true);
  delay(500);
  lcd.lightAll(false);
  delay(100);

  lcd.printText(0, "HELLO 2025");
}

void loop() {
  static bool flip = false;
  if (digitalRead(3) == LOW) {
    delay(200);
    flip = !flip;
    if (flip) lcd.printText(0, "VOLUME 12");
    else      lcd.printText(0, "TRACK 01 ");
  }
}