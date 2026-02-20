#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);
  // while (!Serial) {
  //   delay(10); // ждем инициализации Serial
  // }
  
  // Инициализация I2C
  Wire.begin();
  
  // // Инициализация RTC
  // if (!rtc.begin()) {
  //   Serial.println("Не удалось найти DS3231!");
  //   while (1);
  // }
  
  // // Проверяем, есть ли часы (если нет, устанавливаем текущее время)
  // if (rtc.lostPower()) {
  //   Serial.println("Часы сброшены! Устанавливаем текущее время.");
  //   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // }
  
  Serial.println("DS3231 RTC - Система обновления времени запущена");
  Serial.println("============================================");
}

void loop() {
  DateTime now = rtc.now();
  
  // Выводим текущее время
  Serial.print("Текущее время: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  if (now.minute() < 10) Serial.print('0');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  if (now.second() < 10) Serial.print('0');
  Serial.print(now.second(), DEC);
  Serial.println();
  
  // Выводим температуру от датчика DS3231
  Serial.print("Температура: ");
  Serial.print(rtc.getTemperature());
  Serial.println("°C");
  
  // Выводим день недели
  Serial.print("День недели: ");
  switch(now.dayOfTheWeek()) {
    case 0: Serial.println("Воскресенье"); break;
    case 1: Serial.println("Понедельник"); break;
    case 2: Serial.println("Вторник"); break;
    case 3: Serial.println("Среда"); break;
    case 4: Serial.println("Четверг"); break;
    case 5: Serial.println("Пятница"); break;
    case 6: Serial.println("Суббота"); break;
    default: Serial.println("Неизвестно"); break;
  }
  
  Serial.println("-----------------------------------");
  
  // Увеличиваем время на 1 час
  DateTime newTime = now + TimeSpan(1, 0, 0, 0); // +1 час, 0 мин, 0 сек
  
  // Устанавливаем новое время в RTC
  rtc.adjust(newTime);
  
  Serial.print("Новое время установлено: ");
  Serial.print(newTime.year(), DEC);
  Serial.print('/');
  Serial.print(newTime.month(), DEC);
  Serial.print('/');
  Serial.print(newTime.day(), DEC);
  Serial.print(" ");
  Serial.print(newTime.hour(), DEC);
  Serial.print(':');
  if (newTime.minute() < 10) Serial.print('0');
  Serial.print(newTime.minute(), DEC);
  Serial.print(':');
  if (newTime.second() < 10) Serial.print('0');
  Serial.print(newTime.second(), DEC);
  Serial.println();
  Serial.println("============================================");
  
  // Ждем 1 секунду
  delay(1000);
}