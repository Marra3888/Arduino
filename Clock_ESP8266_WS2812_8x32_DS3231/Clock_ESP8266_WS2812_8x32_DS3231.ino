//Date 02.03.2024  Marchenko Vadim
//Для создания шрифта использовать программу AdafruitGFXFontTool

#include <Wire.h>
#include <string.h>
// #include <MD_DS3231.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include "Fonts/font6x7GFXFONT.h"
#include <RTClib.h> 
#include <ESP8266WiFi.h>  // Для ESP8266, для ESP32 используйте <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Timezone.h> // Библиотека для управления часовыми поясами

// Определение правил для зимнего (UTC+2) и летнего (UTC+3) времени
TimeChangeRule DSTRule = {"EEST", Last, Sun, Mar, 2, 180}; // Летнее время: UTC+3, последнее воскресенье марта, 02:00
TimeChangeRule StdRule = {"EET", Last, Sun, Oct, 2, 120};  // Зимнее время: UTC+2, последнее воскресенье октября, 02:00
Timezone myTZ(DSTRule, StdRule); // Создаем объект часового пояса

RTC_DS3231 RTC;

#define PIN D5

// #define SDA_PIN D2
// #define SCL_PIN D1

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, 4, 1, PIN, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_TILE_COLUMNS + NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800);

// int h, m, sec, month, day, day_week;
// int year;
byte culoare = 0;
int myDay, myMonth, myYear, myDayWeek, myHour, myMinute, mySecond;

bool f = true;
bool z = false;
char buff[16];
char buff1[130];
const char* Day__week[7] = {"Bockrecenze", "Ponedelznik", "Btornik", "Creda", "Qetverg", "Pytnisa", "Cubbota"};
// const char *Day__week[7] = {"Воскресенье", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота"};

// const long timezoneOffset = 2; // ? hours
const char *ntpServer  = "pool.ntp.org"; // change it to local NTP server if needed

// Wi-Fi настройки
const char* ssid = "TP-Link_22DC";
const char* password = "Mar4enko2704";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer);

// Определение перечисления
enum ColorName 
{
  Red,
  Green,
  Blue,
  White,
  Yellow,
  Cyan,
  Magenta,
  Bleu,
  Fiolet,
  LightGreen,
  COLOR_COUNT // Для подсчёта количества цветов
};

// Макросы для цветов
#define COLOR_RED     matrix.Color(255, 0, 0)
#define COLOR_GREEN   matrix.Color(0, 255, 0)
#define COLOR_BLUE    matrix.Color(0, 0, 255)
#define COLOR_WHITE   matrix.Color(255, 255, 255)
#define COLOR_YELLOW  matrix.Color(255, 255, 0)
#define COLOR_CYAN    matrix.Color(0, 255, 255)
#define COLOR_MAGENTA matrix.Color(255, 0, 255)
#define COLOR_Bleu    matrix.Color(51, 153, 255)  // bleu (light blue)
#define COLOR_Fiolet    matrix.Color(102, 102, 153)
#define COLOR_LightGreen    matrix.Color(153, 255, 153)

// Функция для получения цвета по имени
uint16_t getColor(ColorName color) 
{
  if (color == Red)     return COLOR_RED;
  if (color == Green)   return COLOR_GREEN;
  if (color == Blue)    return COLOR_BLUE;
  if (color == White)   return COLOR_WHITE;
  if (color == Yellow)  return COLOR_YELLOW;
  if (color == Cyan)    return COLOR_CYAN;
  if (color == Magenta) return COLOR_MAGENTA;
  if (color == Bleu)    return COLOR_Bleu;
  if (color == Fiolet)    return COLOR_Fiolet;
  if (color == LightGreen)    return COLOR_LightGreen;
  return matrix.Color(0, 0, 0); // Чёрный по умолчанию
}

void getTime()
              {
                // RTC.readTime();
                // // s = RTC.s;
                // m = RTC.m;
                // h = RTC.h;
                // day = RTC.dd;
                // month = RTC.mm;
                // year = RTC.yyyy;
                // dayOfWeek = RTC.dow;
                // m = 36;
                // h = 12;
                  DateTime now = RTC.now();
                  myYear = now.year();       // присваиваем переменным значения года
                  myMonth = now.month();   // месяца
                  myDay = now.day();      // дня
                  myDayWeek = now.dayOfTheWeek();
                  // day_week = 6;
                  myHour = now.hour();      // часа
                  myMinute = now.minute();  // минуты
                  mySecond = now.second(); // и секунды
                  // sec = 50;
              } 
              
void adjust_time_date()
{
              // timeClient.setTimeOffset((timezoneOffset)*3600);
              timeClient.begin();

              if (timeClient.update()) 
              {
                        // unsigned long epochTime = timeClient.getEpochTime();
                        // // Преобразование epoch в дату
                        // time_t rawtime = epochTime;
                        // struct tm * ti;
                        // ti = localtime(&rawtime);
                        // day = ti->tm_mday;
                        // month = ti->tm_mon + 1;
                        // year = ti->tm_year + 1900;
                        // // dt.weekday = ti->tm_wday;
                        // day_week = timeClient.getDay();   
                        // h = timeClient.getHours();
                        // m = timeClient.getMinutes();
                        // sec = timeClient.getSeconds();
                        // RTC.adjust(DateTime(timeClient.getEpochTime()));
                        
                        // Получаем Epoch Time в UTC
                        unsigned long epochTimeUTC = timeClient.getEpochTime();
                        
                        // Преобразуем UTC в локальное время с учетом часового пояса и DST
                        time_t localTime = myTZ.toLocal(epochTimeUTC);
                        
                        // Преобразование локального времени в структуру tm
                        struct tm *ti;
                        ti = localtime(&localTime);
                        
                        // Извлекаем компоненты даты и времени
                        myDay = ti->tm_mday;
                        myMonth = ti->tm_mon + 1;    // Месяцы начинаются с 0, добавляем 1
                        myYear = ti->tm_year + 1900; // Годы отсчитываются от 1900
                        myDayWeek = ti->tm_wday;    // День недели (0 - воскресенье, 6 - суббота)
                        myHour = ti->tm_hour;
                        myMinute = ti->tm_min;
                        mySecond = ti->tm_sec;
                        
                        // Синхронизация RTC с локальным временем
                        RTC.adjust(DateTime(localTime));
              }
}

void setup() 
            {
              // Serial.begin(19200);
              // Wire.begin(SDA_PIN, SCL_PIN);  // new syntax: join i2c bus (address required for slave)
              matrix.begin();
              matrix.setTextWrap(false);
              matrix.setBrightness(30);
              // matrix.setTextColor(matrix.Color(255, 255, 255));
              // Установка цвета текста (красный)
              matrix.setTextColor(getColor(Red));
              matrix.setFont(&Font6X7);
            // RTC.begin();
            // RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
              // RTC.control(DS3231_12H, DS3231_OFF);  
              Wire.begin();
              RTC.begin();

               matrix.fillScreen(0);
               matrix.setCursor(0, 0);
              WiFi.begin(ssid, password);
              for(byte i = 0; i < 15; i++)
              {
                  if (WiFi.status() != WL_CONNECTED) 
                  {
                    delay(500);
                    matrix.print(F("."));
                    matrix.show();
                    // Serial.print(".");
                  }
                  else
                  {
                  matrix.fillScreen(0);
                  matrix.setCursor(0, 0);
                  // Serial.println("\nConnected to WiFi");
                  matrix.print(F("Ok!"));
                  matrix.show();
                  delay(2000);
                  adjust_time_date();
                  return; // Выходим из setup() после подключения
                  }
              }
             

              
            }

int y = matrix.width();

void loop() 
            {
              static int x = 0;
              
              if(x++ > 2) {f = !f; x = 0;}

              getTime();
              
              if(mySecond == 50)
                  {
                    z = false;
                    
                while(!z)
                    {
                        // matrix.setTextColor(matrix.Color(204, 204, 0));
                        matrix.setTextColor(getColor(Yellow));
                        matrix.fillScreen(0);
                        matrix.setCursor(y, 0);
                        // sprintf(buff1, "%02i-%02i-%04i  %s", day, month, year, Day__week[day_week]);
                        sprintf(buff1, "%02i-%02i-%04i ", myDay, myMonth, myYear);
                        // sprintf(buffer, "%u-%02d-%02d", 2000U + yOff, m, d);
                        // sprintf(buffer, "%u-%02d-%02dT%02d:%02d:%02d", 2000U + yOff, m, d, hh, mm, ss);
                        matrix.print(buff1);
                        // matrix.setTextColor(matrix.Color(0, 204, 255));
                        matrix.setTextColor(getColor(Bleu));
                        matrix.printf("%s", Day__week[myDayWeek]);
                        // if(--y < -152) 
                        // if (--y < -strlen(Day__week[day_week]) + 132)
                        // // if (--y < -(strlen(buff1) + strlen(Day__week[day_week]) + c))
                        // // if (--y < -(67 + (strlen(Day__week[day_week]) * 5)))
                        // {
                        //   y = matrix.width();
                        //   z = true;
                        // }
                        // int c = matrix.width();
                          int16_t x1, y1;
                          uint16_t w, h;
                          matrix.getTextBounds(buff1, 0, 0, &x1, &y1, &w, &h);
                          int buff1Width = w;
                          matrix.getTextBounds(Day__week[myDayWeek], 0, 0, &x1, &y1, &w, &h);
                          int dayWeekWidth = w;
                          int totalWidth = buff1Width + dayWeekWidth + 1;

                          if (--y < -totalWidth) 
                          {
                              y = matrix.width();
                              z = true;
                              culoare++;
                              if (culoare >= COLOR_COUNT) culoare = 0;
                          }
                        matrix.show();
                        delay(50);
                    }
                  
                
                  }
              else
                  {
                    // matrix.setTextColor(matrix.Color(255, 255, 255));
                    matrix.setTextColor(getColor(static_cast<ColorName>(culoare)));
                    matrix.fillScreen(0);
                    matrix.setCursor(1, 0);
                    sprintf(buff, "%02d%c%02d", myHour, (f ? ':' : ' '), myMinute);
                    // sprintf(buffer, "%02d:%02d:%02d", hh, mm, ss);
                    matrix.print(buff);
                  
                  // matrix.print(F("11Vadim Mihaylovich 21 April 1976 Year"));
                    matrix.show();
                  }

              delay(100);

            }
