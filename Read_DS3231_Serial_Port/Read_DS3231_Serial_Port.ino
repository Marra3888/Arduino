#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    // while (1);
  }
  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    // Установка времени при потере питания
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  DateTime now = rtc.now();
  
  // Вывод даты
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  
  // Вывод времени
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  
  // Вывод температуры
  Serial.print(" Temp: ");
  Serial.print(rtc.getTemperature());
  Serial.print(" ");
  Serial.print(now.dayOfTheWeek(), DEC);
  // Serial.println(" C");
  
  delay(1000); // Обновление каждую секунду
}