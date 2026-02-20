#ifndef GPS_TIME_H
#define GPS_TIME_H

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Timezone.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "display.h" // Подключаем для DateTimeStruct

void timeInit();
bool updateTimeFromGPS(TinyGPSPlus& gps, SoftwareSerial& gpsSerial, DateTimeStruct& time);
bool updateTimeFromNTP(DateTimeStruct& time);
void incrementTime(DateTimeStruct& time);
uint8_t calculateDayOfWeek(uint16_t year, uint8_t month, uint8_t day); // Добавляем прототип

extern Timezone tz;
extern bool wifiConnected;

#endif