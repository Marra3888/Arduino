#include "gps_time.h"

extern WiFiUDP ntpUDP; // Объявляем внешний объект из main.ino
extern NTPClient timeClient;
extern Timezone tz; // Объявляем как extern

void timeInit() {
  // Инициализация начального времени, если нужно
}

// Вспомогательная функция для вычисления дня недели (0 = Воскресенье, 6 = Суббота)
uint8_t calculateDayOfWeek(uint16_t year, uint8_t month, uint8_t day) {
  if (month < 3) {
    month += 12;
    year--;
  }
  uint16_t k = day;
  uint16_t m = month;
  uint16_t D = year % 100;
  uint16_t C = year / 100;
  uint16_t f = k + ((13 * (m + 1)) / 5) + D + (D / 4) + (C / 4) - (2 * C);
  return (f + 6) % 7; // Сдвиг, чтобы 0 = Воскресенье
}

bool updateTimeFromGPS(TinyGPSPlus& gps, SoftwareSerial& gpsSerial, DateTimeStruct& time) {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.time.isValid() && gps.date.isValid()) {
        time.hours = gps.time.hour();
        time.minutes = gps.time.minute();
        time.seconds = gps.time.second();
        time.day = gps.date.day();
        time.month = gps.date.month();
        time.year = gps.date.year();
        time.weekday = calculateDayOfWeek(time.year, time.month, time.day); // Вычисляем день недели
        return true;
      }
    }
  }
  return false;
}

bool updateTimeFromNTP(DateTimeStruct& time) {
  if (!wifiConnected) {
    Serial.println("No WiFi connection for NTP");
    return false;
  }

  Serial.println("Attempting NTP sync...");
  if (!timeClient.update()) {
    Serial.println("NTP update failed");
    return false;
  }

  unsigned long epochTime = timeClient.getEpochTime();
  if (epochTime < 946684800) { // Проверка на валидность (позже 2000 года)
    Serial.println("Invalid NTP time received");
    return false;
  }

  time_t rawtime = (time_t)epochTime;
  struct tm* ti = localtime(&rawtime);
  if (ti == nullptr) {
    Serial.println("Failed to convert NTP time");
    return false;
  }

  time.hours = ti->tm_hour;
  time.minutes = ti->tm_min;
  time.seconds = ti->tm_sec;
  time.day = ti->tm_mday;
  time.month = ti->tm_mon + 1;
  time.year = ti->tm_year + 1900;
  time.weekday = ti->tm_wday;

  Serial.print("NTP sync successful: ");
  Serial.print(time.hours); Serial.print(":");
  Serial.print(time.minutes); Serial.print(":");
  Serial.println(time.seconds);
  return true;
}

void incrementTime(DateTimeStruct& time) {
  time.seconds++;
  if (time.seconds >= 60) {
    time.seconds = 0;
    time.minutes++;
    if (time.minutes >= 60) {
      time.minutes = 0;
      time.hours++;
      if (time.hours >= 24) {
        time.hours = 0;
        time.day++;
        time.weekday = (time.weekday + 1) % 7;
        // Упрощенная логика для месяца/года
        if (time.day > 31) {
          time.day = 1;
          time.month++;
          if (time.month > 12) {
            time.month = 1;
            time.year++;
          }
        }
      }
    }
  }
}