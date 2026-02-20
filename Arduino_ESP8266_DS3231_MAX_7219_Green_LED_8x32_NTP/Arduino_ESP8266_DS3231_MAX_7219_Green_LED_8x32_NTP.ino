#include <Ticker.h>
// #include <TinyGPS++.h>
// #include <SoftwareSerial.h>
#include <Timezone.h>
#include <Time.h>
// #include <RTClib.h> 
// #include <Wire.h>
#include <ESP8266WiFi.h>  // Для ESP8266, для ESP32 используйте <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <MD_DS3231.h>

// const long timezoneOffset = 2; // ? hours
const char *ntpServer  = "pool.ntp.org"; // change it to local NTP server if needed

// Wi-Fi настройки
// Массив с SSID и паролями
const char* ssid[] = {"TP-LINK_333856", "TP-Link_22DC"};
const char* password[] = {"", "Mar4enko2704"};
const int numNetworks = 2;
// const char* ssid = "TP-LINK_333856";
// const char* password = "Mar4enko2704";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer);

// RTC_DS3231 rtc; // Объявление объекта RTC

//#include "fonts.h"

// static const uint8_t RXPin = 4, TXPin = 5; //D2 - RX, D1 - TX
// static const uint32_t GPSBaud = 9600;

// TinyGPSPlus GPS;
// SoftwareSerial SerialGPS(RXPin, TXPin);
TimeChangeRule myDST = {"CEST", Last, Sun, Mar, 2, 180}; // Central European Summer Time; CEST (Центральноевропейское Летнее Время) - одно из общеизвестных названий для UTC+2 часового пояса
TimeChangeRule mySTD = {"CET", Last, Sun, Oct, 2, 120};   // Central European Time(UTC+1)
Timezone myTZ(myDST, mySTD);//DST - правило для начала летнего времени или летнего времени для любого года;
                            //STD - правило начала поясного времени на любой год.
// TimeChangeRule *tcr;

#define DIN_PIN D8//15 //D8
#define CS_PIN  D6//12 //D6
#define CLK_PIN D7//13 //D7

#define NUM_MAX     4
#include "max7219_hr.h"

unsigned char maxPosX = NUM_MAX * 8 - 1;            
unsigned char LEDarr[NUM_MAX][8];                   
unsigned char helpArrMAX[NUM_MAX * 8];              
unsigned char helpArrPos[NUM_MAX * 8];              
unsigned char z_PosX = 0;                            
unsigned char d_PosX = 0;                            
bool f_tckr1s = false;
bool f_tckr50ms = false;


#define MAX_MESG  75
char szMesg[MAX_MESG + 1] = "";
// другие дисплеи -------------------------------------
//#define REVERSE_HORIZONTAL                        // Parola, Generic and IC-Station
#define REVERSE_VERTICAL                          // IC-Station display
//#define ROTATE_90                                 // Generic display
//#define ROTATE_270
#define Russian

/*
   p  A  B  C  D  E  F  G        7  6  5  4  3  2  1  0        G  F  E  D  C  B  A  p        G  F  E  D  C  B  A  p
  ------------------------      ------------------------      ------------------------      ------------------------
0 |o  o  o  o  o  o  o  o|    p |o  o  o  o  o  o  o  o|    0 |o  o  o  o  o  o  o  o|    7 |o  o  o  o  o  o  o  o|
1 |o  o  o  o  o  o  o  o|    A |o  o  o  o  o  o  o  o|    1 |o  o  o  o  o  o  o  o|    6 |o  o  o  o  o  o  o  o|
2 |o  o  o  o  o  o  o  o|    B |o  o  o  o  o  o  o  o|    2 |o  o  o  o  o  o  o  o|    5 |o  o  o  o  o  o  o  o|
3 |o  o              o  o|    C |o  o              o  o|    3 |o  o              o  o|    4 |o  o              o  o|
4 |o  o    FC-16     o  o|    D |o  o   Generic    o  o|    4 |o  o   Parola     o  o|    3 |o  o  IC-Station  o  o|
5 |o  o              o  o|    E |o  o              o  o|    5 |o  o              o  o|    2 |o  o              o  o|
6 |o  o  o  o  o  o  o  o|    F |o  o  o  o  o  o  o  o|    6 |o  o  o  o  o  o  o  o|    1 |o  o  o  o  o  o  o  o|
7 |o  o  o  o  o  o  o  o|    G |o  o  o  o  o  o  o  o|    7 |o  o  o  o  o  o  o  o|    0 |o  o  o  o  o  o  o  o|
  ------------------------      ------------------------      ------------------------      ------------------------
*/
struct MyDateTime 
{
    unsigned char sek1, sek2, sek12, min1, min2, min12, hour1, hour2, hour12;
    unsigned char tag1, tag2, tag12, mon1, mon2, mon12, year1, year2, year12, WT;
} MEZ;

byte Tag, Monat, WoTag, Stunde, Minute, Sekunde;
int Jahr;

// Объект для Ticker
Ticker tckr;
// Экземпляр UDP, позволяющий нам отправлять и получать пакеты по UDP.
//WiFiUDP udp;

byte sec1_position = 27;
byte sec2_position = 23;
byte min1_position = 18;
byte min2_position = 13;
byte hour1_position = 4;
byte hour2_position = 1;
byte colon_position = 11;

//месяцы
char M_arr[12][5] = 
{
    { ' ', 'J', 'A', 'N', ' ' },
    { ' ', 'F', 'E', 'B', ' ' },
    { ' ', 'M', 'A', 'R', ' ' },
    { ' ', 'A', 'P', 'R', ' ' },
    { ' ', 'M', 'A', 'Y', ' ' },
    { ' ', 'J', 'U', 'N', ' ' },
    { ' ', 'J', 'U', 'L', ' ' },
    { ' ', 'A', 'U', 'G', ' ' },
    { ' ', 'S', 'E', 'P', ' ' },
    { ' ', 'O', 'C', 'T', ' ' },
    { ' ', 'N', 'O', 'V', ' ' },
    { ' ', 'D', 'E', 'C', ' ' }
};
//дни
char WT_arr[7][4] = 
{
    { 'S', 'U', 'N', ' ' },
    { 'M', 'O', 'N', ' ' },
    { 'T', 'U', 'E', ' ' },
    { 'W', 'E', 'D', ' ' },
    { 'T', 'H', 'U', ' ' },
    { 'F', 'R', 'I', ' ' },
    { 'S', 'A', 'T', ' ' }
};

const uint8_t M_rus[13][9] = 
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },//" ",
    { 11, 23, 14, 12, 26, 32, 0xff, 0xff, 0xff },//" Январь ",
    { 9, 17, 14, 26, 12, 21, 32, 0xff, 0xff },//" Февраль ",
    { 4, 12, 26, 28, 12, 0xff, 0xff, 0xff, 0xff },//" Март ",
    { 0, 25, 26, 17, 21, 32, 0xff, 0xff,0xff },//" Апрель ",
    { 4, 12, 32, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },//" Май ",
    { 3, 33, 23, 32, 0xff, 0xff, 0xff, 0xff, 0xff },//" Июнь ",
    { 3, 33, 21, 32, 0xff, 0xff, 0xff, 0xff, 0xff },//" Июль ",
    { 0, 14, 15, 29, 27, 28, 12, 0xff, 0xff },//" Август ",
    { 8, 17, 23, 28, 32, 13, 26, 32, 0xff },//" Сентябрь ",
    { 6, 20, 28, 32, 13, 26, 32, 0xff, 0xff },//" Октябрь ",
    { 5, 24, 32, 13, 26, 32, 0xff, 0xff, 0xff },//" Ноябрь ",
    { 2, 17, 26, 12, 13, 26, 32, 0xff, 0xff }//" Декабрь "
};

const uint8_t WT_rus[7][12] = 
{
    { 1, 24, 27, 20, 26, 17, 27, 17, 23, 31, 17, 0xff },//"Воскресенье "
    { 7, 24, 23, 17, 16, 17, 21, 31, 23, 18, 20,0xff },//"Понедельник ",
    { 1, 28, 24, 26, 23, 18, 20, 0xff, 0xff, 0xff, 0xff, 0xff },//"Вторник ",
    { 8, 26, 17, 16, 12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },//"Среда ",
    { 10, 17, 28, 14, 17, 26, 15, 0xff, 0xff, 0xff, 0xff, 0xff },//"Четверг ",
    { 7, 32, 28, 23, 18, 30, 12, 0xff, 0xff, 0xff, 0xff, 0xff },//"Пятница ",
    { 8, 29, 13, 13, 24, 28, 12, 0xff, 0xff, 0xff, 0xff, 0xff }//"Суббота ",
};

// Набор символов 6x8 в матрице 8x8, 0,0 вверху справа
unsigned char const font3[16][8] =
{
    {30, 63, 51, 51, 51, 51, 63, 30}, // 0
    {6, 14, 30, 6, 6, 6, 6, 15},      // 1
    {30, 63, 35, 6, 12, 24, 63, 63},  // 2
    {30, 63, 35, 14, 14, 35, 63, 30}, // 3
    {6, 14, 30, 54, 54, 63, 6, 15},   // 4
    {62, 63, 48, 62, 51, 3, 63, 30},  // 5
    {30, 63, 48, 62, 51, 51, 63, 30}, // 6
    {63, 63, 3, 6, 12, 12, 12, 12},   // 7
    {30, 63, 51, 30, 51, 51, 63, 30}, // 8
    {30, 63, 51, 51, 31, 3, 63, 30},  // 9
    {0, 0, 0, 0, 0, 0, 0, 0},         // blank
    {0, 3, 3, 0, 0, 3, 3, 0},         // dp
    {120, 124, 68, 64, 64, 68, 124, 120}, // C
    {56, 109, 109, 109, 57, 1, 1, 0}, // `
    {62, 63, 51, 62, 51, 51, 63, 62}, // B
    {0, 0, 27, 31, 28, 24, 24, 24}    // r
};

// Набор символов 5x8 в матрице 8x8, 0,0 вверху справа
unsigned char const font1[96][9] = 
{
        { 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x20, Space
        { 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00 },   // 0x21, !
        { 0x07, 0x09, 0x09, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x22, "
        { 0x07, 0x0a, 0x0a, 0x1f, 0x0a, 0x1f, 0x0a, 0x0a, 0x00 },   // 0x23, #
        { 0x07, 0x04, 0x0f, 0x14, 0x0e, 0x05, 0x1e, 0x04, 0x00 },   // 0x24, $
        { 0x07, 0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13, 0x00 },   // 0x25, %
        { 0x07, 0x04, 0x0a, 0x0a, 0x0a, 0x15, 0x12, 0x0d, 0x00 },   // 0x26, &
        { 0x07, 0x04, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x27, '
        { 0x07, 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02, 0x00 },   // 0x28, (
        { 0x07, 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08, 0x00 },   // 0x29, )
        { 0x07, 0x04, 0x15, 0x0e, 0x1f, 0x0e, 0x15, 0x04, 0x00 },   // 0x2a, *
        { 0x07, 0x00, 0x04, 0x04, 0x1f, 0x04, 0x04, 0x00, 0x00 },   // 0x2b, +
        { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02 },   // 0x2c, ,
        { 0x07, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00 },   // 0x2d, -
        { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00 },   // 0x2e, .
        { 0x07, 0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10, 0x00 },   // 0x2f, /
        { 0x07, 0x0F, 0x09, 0x09, 0x09, 0x09, 0x09, 0x0F, 0x00 },   // 0x30, 0
        { 0x07, 0x02, 0x06, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00 },   // 0x31, 1
        { 0x07, 0x0F, 0x01, 0x01, 0x0F, 0x08, 0x08, 0x0F, 0x00 },   // 0x32, 2
        { 0x07, 0x0F, 0x01, 0x01, 0x07, 0x01, 0x01, 0x0F, 0x00 },   // 0x33, 3
        { 0x07, 0x09, 0x09, 0x09, 0x0F, 0x01, 0x01, 0x01, 0x00 },   // 0x34, 4
        { 0x07, 0x0F, 0x08, 0x08, 0x0F, 0x01, 0x01, 0x0F, 0x00 },   // 0x35, 5
        { 0x07, 0x0F, 0x08, 0x08, 0x0F, 0x09, 0x09, 0x0F, 0x00 },   // 0x36, 6
        { 0x07, 0x0F, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00 },   // 0x37, 7
        { 0x07, 0x0F, 0x09, 0x09, 0x0F, 0x09, 0x09, 0x0F, 0x00 },   // 0x38, 8
        { 0x07, 0x0F, 0x09, 0x09, 0x0F, 0x01, 0x01, 0x0F, 0x00 },   // 0x39, 9
//        { 0x04, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00 },   // 0x3a, :  
//        { 0x04, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00 },   // 0x3a, :
        { 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00 },   // 0x3a, :
        { 0x07, 0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x04, 0x08, 0x00 },   // 0x3b, ;
        { 0x07, 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02, 0x00 },   // 0x3c, <
        { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x3d, =
        { 0x07, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x00 },   // 0x3e, >
        { 0x07, 0x0e, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04, 0x00 },   // 0x3f, ?
        { 0x07, 0x0e, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0f, 0x00 },   // 0x40, @
        { 0x07, 0x04, 0x0a, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x00 },   // 0x41, A
        { 0x07, 0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e, 0x00 },   // 0x42, B
        { 0x07, 0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e, 0x00 },   // 0x43, C
        { 0x07, 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E, 0x00 },   // 0x44, D
        { 0x07, 0x1f, 0x10, 0x10, 0x1c, 0x10, 0x10, 0x1f, 0x00 },   // 0x45, E
        { 0x07, 0x1f, 0x10, 0x10, 0x1f, 0x10, 0x10, 0x10, 0x00 },   // 0x46, F
        { 0x07, 0x0e, 0x11, 0x10, 0x10, 0x13, 0x11, 0x0f, 0x00 },   // 0x37, G
        { 0x07, 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11, 0x00 },   // 0x48, H
        { 0x07, 0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e, 0x00 },   // 0x49, I
        { 0x07, 0x1f, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0c, 0x00 },   // 0x4a, J
        { 0x07, 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11, 0x00 },   // 0x4b, K
        { 0x07, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f, 0x00 },   // 0x4c, L
        { 0x07, 0x11, 0x1b, 0x15, 0x11, 0x11, 0x11, 0x11, 0x00 },   // 0x4d, M
        { 0x07, 0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x00 },   // 0x4e, N
        { 0x07, 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e, 0x00 },   // 0x4f, O
        { 0x07, 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10, 0x00 },   // 0x50, P
        { 0x07, 0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d, 0x00 },   // 0x51, Q
        { 0x07, 0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11, 0x00 },   // 0x52, R
        { 0x07, 0x0e, 0x11, 0x10, 0x0e, 0x01, 0x11, 0x0e, 0x00 },   // 0x53, S
        { 0x07, 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00 },   // 0x54, T
        { 0x07, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e, 0x00 },   // 0x55, U
        { 0x07, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04, 0x00 },   // 0x56, V
        { 0x07, 0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11, 0x00 },   // 0x57, W
        { 0x07, 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11, 0x00 },   // 0x58, X
        { 0x07, 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04, 0x00 },   // 0x59, Y
        { 0x07, 0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f, 0x00 },   // 0x5a, Z
        { 0x07, 0x0e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0e, 0x00 },   // 0x5b, [
        { 0x07, 0x10, 0x10, 0x08, 0x04, 0x02, 0x01, 0x01, 0x00 },   // 0x5c, '\'
        { 0x07, 0x0e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0e, 0x00 },   // 0x5d, ]
        { 0x07, 0x04, 0x0a, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x5e, ^
        { 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00 },   // 0x5f, _
        { 0x07, 0x04, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x60, `
        { 0x07, 0x00, 0x0e, 0x01, 0x0d, 0x13, 0x13, 0x0d, 0x00 },   // 0x61, a
        { 0x07, 0x10, 0x10, 0x10, 0x1c, 0x12, 0x12, 0x1c, 0x00 },   // 0x62, b
        { 0x07, 0x00, 0x00, 0x0E, 0x10, 0x10, 0x10, 0x0E, 0x00 },   // 0x63, c
        { 0x07, 0x01, 0x01, 0x01, 0x07, 0x09, 0x09, 0x07, 0x00 },   // 0x64, d
        { 0x07, 0x00, 0x00, 0x0e, 0x11, 0x1f, 0x10, 0x0f, 0x00 },   // 0x65, e
        { 0x07, 0x06, 0x09, 0x08, 0x1c, 0x08, 0x08, 0x08, 0x00 },   // 0x66, f
        { 0x07, 0x00, 0x0e, 0x11, 0x13, 0x0d, 0x01, 0x01, 0x0e },   // 0x67, g
        { 0x07, 0x10, 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x00 },   // 0x68, h
        { 0x05, 0x00, 0x02, 0x00, 0x06, 0x02, 0x02, 0x07, 0x00 },   // 0x69, i
        { 0x07, 0x00, 0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0c },   // 0x6a, j
        { 0x07, 0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12, 0x00 },   // 0x6b, k
        { 0x05, 0x06, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00 },   // 0x6c, l
        { 0x07, 0x00, 0x00, 0x0a, 0x15, 0x15, 0x11, 0x11, 0x00 },   // 0x6d, m
        { 0x07, 0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11, 0x00 },   // 0x6e, n
        { 0x07, 0x00, 0x00, 0x0e, 0x11, 0x11, 0x11, 0x0e, 0x00 },   // 0x6f, o
        { 0x07, 0x00, 0x00, 0x1c, 0x12, 0x12, 0x1c, 0x10, 0x10 },   // 0x70, p
        { 0x07, 0x00, 0x00, 0x07, 0x09, 0x09, 0x07, 0x01, 0x01 },   // 0x71, q
        { 0x07, 0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10, 0x00 },   // 0x72, r
        { 0x07, 0x00, 0x00, 0x0f, 0x10, 0x0e, 0x01, 0x1e, 0x00 },   // 0x73, s
        { 0x07, 0x08, 0x08, 0x1c, 0x08, 0x08, 0x09, 0x06, 0x00 },   // 0x74, t
        { 0x07, 0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0d, 0x00 },   // 0x75, u
        { 0x07, 0x00, 0x00, 0x11, 0x11, 0x11, 0x0a, 0x04, 0x00 },   // 0x76, v
        { 0x07, 0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0a, 0x00 },   // 0x77, w
        { 0x07, 0x00, 0x00, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x00 },   // 0x78, x
        { 0x07, 0x00, 0x00, 0x11, 0x11, 0x0f, 0x01, 0x11, 0x0e },   // 0x79, y
        { 0x07, 0x00, 0x00, 0x1f, 0x02, 0x04, 0x08, 0x1f, 0x00 },   // 0x7a, z
        { 0x07, 0x06, 0x08, 0x08, 0x10, 0x08, 0x08, 0x06, 0x00 },   // 0x7b, {
        { 0x07, 0x04, 0x04, 0x04, 0x00, 0x04, 0x04, 0x04, 0x00 },   // 0x7c, |
        { 0x07, 0x0c, 0x02, 0x02, 0x01, 0x02, 0x02, 0x0c, 0x00 },   // 0x7d, }
        { 0x07, 0x08, 0x15, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x7e, ~
        { 0x07, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00 }    // 0x7f, DEL
};

// Набор символов 4x5 в матрице 8x8, 0,0 вверху справа
unsigned char const font2[96][9] = 
{
        { 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x20, Space
        { 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00 },   // 0x21, !
        { 0x07, 0x09, 0x09, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x22, "
        { 0x07, 0x0a, 0x0a, 0x1f, 0x0a, 0x1f, 0x0a, 0x0a, 0x00 },   // 0x23, #
        { 0x07, 0x04, 0x0f, 0x14, 0x0e, 0x05, 0x1e, 0x04, 0x00 },   // 0x24, $
        { 0x07, 0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13, 0x00 },   // 0x25, %
        { 0x07, 0x04, 0x0a, 0x0a, 0x0a, 0x15, 0x12, 0x0d, 0x00 },   // 0x26, &
        { 0x07, 0x04, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x27, '
        { 0x07, 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02, 0x00 },   // 0x28, (
        { 0x07, 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08, 0x00 },   // 0x29, )
        { 0x07, 0x04, 0x15, 0x0e, 0x1f, 0x0e, 0x15, 0x04, 0x00 },   // 0x2a, *
        { 0x07, 0x00, 0x04, 0x04, 0x1f, 0x04, 0x04, 0x00, 0x00 },   // 0x2b, +
        { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02 },   // 0x2c, ,
        { 0x07, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00 },   // 0x2d, -
        { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00 },   // 0x2e, .
        { 0x07, 0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10, 0x00 },   // 0x2f, /
        { 0x07, 0x00, 0x00, 0x07, 0x05, 0x05, 0x05, 0x07, 0x00 },   // 0x30, 0
        { 0x07, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00 },   // 0x31, 1
        { 0x07, 0x00, 0x00, 0x07, 0x01, 0x07, 0x04, 0x07, 0x00 },   // 0x32, 2
        { 0x07, 0x00, 0x00, 0x07, 0x01, 0x07, 0x01, 0x07, 0x00 },   // 0x33, 3
        { 0x07, 0x00, 0x00, 0x05, 0x05, 0x07, 0x01, 0x01, 0x00 },   // 0x34, 4
        { 0x07, 0x00, 0x00, 0x07, 0x04, 0x07, 0x01, 0x07, 0x00 },   // 0x35, 5
        { 0x07, 0x00, 0x00, 0x07, 0x04, 0x07, 0x05, 0x07, 0x00 },   // 0x36, 6
        { 0x07, 0x00, 0x00, 0x07, 0x01, 0x01, 0x01, 0x01, 0x00 },   // 0x37, 7
        { 0x07, 0x00, 0x00, 0x07, 0x05, 0x07, 0x05, 0x07, 0x00 },   // 0x38, 8
        { 0x07, 0x00, 0x00, 0x07, 0x05, 0x07, 0x01, 0x07, 0x00 },   // 0x39, 9
        { 0x04, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00 },   // 0x3a, :
        { 0x07, 0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x04, 0x08, 0x00 },   // 0x3b, ;
        { 0x07, 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02, 0x00 },   // 0x3c, <
        { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x3d, =
        { 0x07, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x00 },   // 0x3e, >
        { 0x07, 0x0e, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04, 0x00 },   // 0x3f, ?
        { 0x07, 0x0e, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0f, 0x00 },   // 0x40, @
        { 0x07, 0x04, 0x0a, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x00 },   // 0x41, A
        { 0x07, 0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e, 0x00 },   // 0x42, B
        { 0x07, 0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e, 0x00 },   // 0x43, C
        { 0x07, 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E, 0x00 },   // 0x44, D
        { 0x07, 0x1f, 0x10, 0x10, 0x1c, 0x10, 0x10, 0x1f, 0x00 },   // 0x45, E
        { 0x07, 0x1f, 0x10, 0x10, 0x1f, 0x10, 0x10, 0x10, 0x00 },   // 0x46, F
        { 0x07, 0x0e, 0x11, 0x10, 0x10, 0x13, 0x11, 0x0f, 0x00 },   // 0x37, G
        { 0x07, 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11, 0x00 },   // 0x48, H
        { 0x07, 0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e, 0x00 },   // 0x49, I
        { 0x07, 0x1f, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0c, 0x00 },   // 0x4a, J
        { 0x07, 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11, 0x00 },   // 0x4b, K
        { 0x07, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f, 0x00 },   // 0x4c, L
        { 0x07, 0x11, 0x1b, 0x15, 0x11, 0x11, 0x11, 0x11, 0x00 },   // 0x4d, M
        { 0x07, 0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x00 },   // 0x4e, N
        { 0x07, 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e, 0x00 },   // 0x4f, O
        { 0x07, 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10, 0x00 },   // 0x50, P
        { 0x07, 0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d, 0x00 },   // 0x51, Q
        { 0x07, 0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11, 0x00 },   // 0x52, R
        { 0x07, 0x0e, 0x11, 0x10, 0x0e, 0x01, 0x11, 0x0e, 0x00 },   // 0x53, S
        { 0x07, 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00 },   // 0x54, T
        { 0x07, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e, 0x00 },   // 0x55, U
        { 0x07, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04, 0x00 },   // 0x56, V
        { 0x07, 0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11, 0x00 },   // 0x57, W
        { 0x07, 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11, 0x00 },   // 0x58, X
        { 0x07, 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04, 0x00 },   // 0x59, Y
        { 0x07, 0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f, 0x00 },   // 0x5a, Z
        { 0x07, 0x0e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0e, 0x00 },   // 0x5b, [
        { 0x07, 0x10, 0x10, 0x08, 0x04, 0x02, 0x01, 0x01, 0x00 },   // 0x5c, '\'
        { 0x07, 0x0e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0e, 0x00 },   // 0x5d, ]
        { 0x07, 0x04, 0x0a, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x5e, ^
        { 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00 },   // 0x5f, _
        { 0x07, 0x04, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x60, `
        { 0x07, 0x00, 0x0e, 0x01, 0x0d, 0x13, 0x13, 0x0d, 0x00 },   // 0x61, a
        { 0x07, 0x10, 0x10, 0x10, 0x1c, 0x12, 0x12, 0x1c, 0x00 },   // 0x62, b
        { 0x07, 0x00, 0x00, 0x0E, 0x10, 0x10, 0x10, 0x0E, 0x00 },   // 0x63, c
        { 0x07, 0x01, 0x01, 0x01, 0x07, 0x09, 0x09, 0x07, 0x00 },   // 0x64, d
        { 0x07, 0x00, 0x00, 0x0e, 0x11, 0x1f, 0x10, 0x0f, 0x00 },   // 0x65, e
        { 0x07, 0x06, 0x09, 0x08, 0x1c, 0x08, 0x08, 0x08, 0x00 },   // 0x66, f
        { 0x07, 0x00, 0x0e, 0x11, 0x13, 0x0d, 0x01, 0x01, 0x0e },   // 0x67, g
        { 0x07, 0x10, 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x00 },   // 0x68, h
        { 0x05, 0x00, 0x02, 0x00, 0x06, 0x02, 0x02, 0x07, 0x00 },   // 0x69, i
        { 0x07, 0x00, 0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0c },   // 0x6a, j
        { 0x07, 0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12, 0x00 },   // 0x6b, k
        { 0x05, 0x06, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00 },   // 0x6c, l
        { 0x07, 0x00, 0x00, 0x0a, 0x15, 0x15, 0x11, 0x11, 0x00 },   // 0x6d, m
        { 0x07, 0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11, 0x00 },   // 0x6e, n
        { 0x07, 0x00, 0x00, 0x0e, 0x11, 0x11, 0x11, 0x0e, 0x00 },   // 0x6f, o
        { 0x07, 0x00, 0x00, 0x1c, 0x12, 0x12, 0x1c, 0x10, 0x10 },   // 0x70, p
        { 0x07, 0x00, 0x00, 0x07, 0x09, 0x09, 0x07, 0x01, 0x01 },   // 0x71, q
        { 0x07, 0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10, 0x00 },   // 0x72, r
        { 0x07, 0x00, 0x00, 0x0f, 0x10, 0x0e, 0x01, 0x1e, 0x00 },   // 0x73, s
        { 0x07, 0x08, 0x08, 0x1c, 0x08, 0x08, 0x09, 0x06, 0x00 },   // 0x74, t
        { 0x07, 0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0d, 0x00 },   // 0x75, u
        { 0x07, 0x00, 0x00, 0x11, 0x11, 0x11, 0x0a, 0x04, 0x00 },   // 0x76, v
        { 0x07, 0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0a, 0x00 },   // 0x77, w
        { 0x07, 0x00, 0x00, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x00 },   // 0x78, x
        { 0x07, 0x00, 0x00, 0x11, 0x11, 0x0f, 0x01, 0x11, 0x0e },   // 0x79, y
        { 0x07, 0x00, 0x00, 0x1f, 0x02, 0x04, 0x08, 0x1f, 0x00 },   // 0x7a, z
        { 0x07, 0x06, 0x08, 0x08, 0x10, 0x08, 0x08, 0x06, 0x00 },   // 0x7b, {
        { 0x07, 0x04, 0x04, 0x04, 0x00, 0x04, 0x04, 0x04, 0x00 },   // 0x7c, |
        { 0x07, 0x0c, 0x02, 0x02, 0x01, 0x02, 0x02, 0x0c, 0x00 },   // 0x7d, }
        { 0x07, 0x08, 0x15, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 },   // 0x7e, ~
        { 0x07, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00 }    // 0x7f, DEL
};

unsigned char const Rus[34][9] =
{
  { 0x07, 0x04, 0x0a, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x00 },   // 0, A
  { 0x07, 0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e, 0x00 },   // 1, B
  { 0x07, 0x06, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x1f, 0x11 },   // 2, Д
  { 0x07, 0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x00 },   // 3, И
  { 0x07, 0x11, 0x1b, 0x15, 0x15, 0x15, 0x11, 0x11, 0x00 },   // 4, М
  { 0x07, 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11, 0x00 },   // 5, Н
  { 0x07, 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e, 0x00 },   // 6, О
  { 0x07, 0x1f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00 },   // 7, П
  { 0x07, 0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e, 0x00 },   // 8, С
  { 0x07, 0x04, 0x0e, 0x15, 0x15, 0x15, 0x0e, 0x04, 0x00 },   // 9, Ф
  { 0x07, 0x09, 0x09, 0x09, 0x07, 0x01, 0x01, 0x01, 0x00 },   // 10, Ч
  { 0x07, 0x0f, 0x11, 0x11, 0x0f, 0x05, 0x09, 0x11, 0x00 },   // 11, Я
  { 0x07, 0x00, 0x0e, 0x01, 0x0d, 0x13, 0x13, 0x0d, 0x00 },   // 12, a
  { 0x07, 0x00, 0x0e, 0x10, 0x1e, 0x11, 0x11, 0x1e, 0x00 },   // 13, б
  { 0x07, 0x00, 0x00, 0x1e, 0x11, 0x1e, 0x11, 0x1e, 0x00 },   // 14, в
  { 0x07, 0x00, 0x00, 0x1e, 0x10, 0x10, 0x10, 0x10, 0x00 },   // 15, г
  { 0x07, 0x00, 0x00, 0x04, 0x0a, 0x0a, 0x0a, 0x1f, 0x11 },   // 16, д
  { 0x07, 0x00, 0x00, 0x0e, 0x11, 0x1f, 0x10, 0x0f, 0x00 },   // 17, е
  { 0x07, 0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0d, 0x00 },   // 18, и
  { 0x07, 0x0e, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0d, 0x00 },   // 19, й
  { 0x07, 0x00, 0x00, 0x12, 0x14, 0x18, 0x14, 0x12, 0x00 },   // 20, к
  { 0x07, 0x00, 0x00, 0x0f, 0x09, 0x09, 0x09, 0x11, 0x00 },   // 21, л
  { 0x07, 0x00, 0x00, 0x0a, 0x15, 0x15, 0x11, 0x11, 0x00 },   // 22, м
  { 0x07, 0x00, 0x00, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x00 },   // 23, н
  { 0x07, 0x00, 0x00, 0x0e, 0x11, 0x11, 0x11, 0x0e, 0x00 },   // 24, о
  { 0x07, 0x00, 0x00, 0x1f, 0x11, 0x11, 0x11, 0x11, 0x00 },   // 25, п
  { 0x07, 0x00, 0x00, 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10 },   // 26, р
  { 0x07, 0x00, 0x00, 0x0f, 0x10, 0x10, 0x10, 0x0f, 0x00 },   // 27, с
  { 0x07, 0x00, 0x00, 0x1f, 0x04, 0x04, 0x04, 0x04, 0x00 },   // 28, т
  { 0x07, 0x00, 0x00, 0x11, 0x11, 0x0f, 0x01, 0x11, 0x0e },   // 29, у
  { 0x07, 0x00, 0x00, 0x12, 0x12, 0x12, 0x12, 0x1f, 0x01 },   // 30, ц
  { 0x07, 0x00, 0x00, 0x10, 0x10, 0x1e, 0x11, 0x1e, 0x00 },   // 31, ь
  { 0x07, 0x00, 0x00, 0x0f, 0x11, 0x0f, 0x09, 0x11, 0x00 },   // 32, я
  { 0x08, 0x00, 0x00, 0x26, 0x29, 0x39, 0x29, 0x26, 0x00 },   // 33, ю

};
//**************************************************************************************************
void Read__Time()
{
    // DateTime RTC = rtc.now(); // Читаем время из RTC
    // time_t utc = now.unixtime();
    // time_t localTime = myTZ.toLocal(utc); // Преобразуем в локальное время с учетом часового пояса

    // Tag = day(localTime); // Получаем локальный день
    // Monat = month(localTime); // Получаем локальный месяц
    // Jahr = year(localTime); // Получаем локальный год
    // WoTag = weekday(localTime) - 1; // weekday() возвращает 1-7 (Вс-Сб), корректируем на 0-6
    // Stunde = hour(localTime); // Получаем локальные часы
    // Minute = minute(localTime); // Получаем локальные минуты
    // Sekunde = second(localTime); // Получаем локальные секунды

    // Tag = now.day(); // Получаем локальный день
    // Monat = now.month(); // Получаем локальный месяц
    // Jahr = now.year(); // Получаем локальный год
    // WoTag = now.dayOfTheWeek(); // weekday() возвращает 1-7 (Вс-Сб), корректируем на 0-6
    // Stunde = now.hour(); // Получаем локальные часы
    // Minute = now.minute(); // Получаем локальные минуты
    // Sekunde = now.second(); // Получаем локальные секунды
    // Serial.println(Tag);
    // Serial.println(Monat);
    // Serial.println(Jahr);
    // Serial.println(WoTag);
    // Serial.println(Stunde);
    // Serial.println(Minute);
    // Serial.println(Sekunde);
    // Serial.println(" ");
    if (RTC.readTime()) 
        {
            Tag = RTC.dd; // Получаем локальный день
            Monat = RTC.mm; // Получаем локальный месяц
            Jahr = RTC.yyyy; // Получаем локальный год
            WoTag = RTC.dow; // weekday() возвращает 1-7 (Вс-Сб), корректируем на 0-6
            Stunde = RTC.h; // Получаем локальные часы
            Minute = RTC.m; // Получаем локальные минуты
            Sekunde = RTC.s; // Получаем локальные секунды
        }
    else 
        {
                Tag = 1; // Получаем локальный день
                Monat = 1; // Получаем локальный месяц
                Jahr = 25; // Получаем локальный год
                WoTag = 1; // weekday() возвращает 1-7 (Вс-Сб), корректируем на 0-6
                Stunde = 20; // Получаем локальные часы
                Minute = 10; // Получаем локальные минуты
                Sekunde = 0; // Получаем локальные секунды     
        }
    // Serial.println(Sekunde);
    // Serial.println(WoTag);

}
//*********************************************************************************************************
// void adjust_time_date()
// {
//               // timeClient.setTimeOffset((timezoneOffset)*3600);
//               timeClient.begin();

//               if (timeClient.update()) 
//               {
//                         // unsigned long epochTime = timeClient.getEpochTime();
//                         // // Преобразование epoch в дату
//                         // time_t rawtime = epochTime;
//                         // struct tm * ti;
//                         // ti = localtime(&rawtime);
//                         // day = ti->tm_mday;
//                         // month = ti->tm_mon + 1;
//                         // year = ti->tm_year + 1900;
//                         // // dt.weekday = ti->tm_wday;
//                         // day_week = timeClient.getDay();   
//                         // h = timeClient.getHours();
//                         // m = timeClient.getMinutes();
//                         // sec = timeClient.getSeconds();
//                         // RTC.adjust(DateTime(timeClient.getEpochTime()));
                        
//                         // Получаем Epoch Time в UTC
//                         unsigned long epochTimeUTC = timeClient.getEpochTime();
                        
//                         // Преобразуем UTC в локальное время с учетом часового пояса и DST
//                         time_t localTime = myTZ.toLocal(epochTimeUTC);
                        
//                         // Преобразование локального времени в структуру tm
//                         struct tm *ti;
//                         ti = localtime(&localTime);
                        
//                         // Извлекаем компоненты даты и времени
//                         Tag = ti->tm_mday;
//                         Monat = ti->tm_mon + 1;    // Месяцы начинаются с 0, добавляем 1
//                         Jahr = ti->tm_year + 1900; // Годы отсчитываются от 1900
//                         WoTag = ti->tm_wday;    // День недели (0 - воскресенье, 6 - суббота)
//                         Stunde = ti->tm_hour;
//                         Minute = ti->tm_min;
//                         Sekunde = ti->tm_sec;
                        
//                         // Синхронизация RTC с локальным временем
//                         rtc.adjust(DateTime(localTime));
//               }
// }

void adjust_time_date() 
{
    timeClient.begin();
    if (timeClient.update()) 
      { 
            // Обновляем время с NTP
        // unsigned long epochTime = timeClient.getEpochTime();//timeClient.getEpochTime() возвращает время в формате Unix (секунды с 1 января 1970 года).
        //Получаем Epoch Time в UTC
        unsigned long epochTimeUTC = timeClient.getEpochTime();

        // Преобразуем UTC в локальное время с учетом часового пояса и DST
        time_t localTime = myTZ.toLocal(epochTimeUTC);

        // Преобразуем время Unix в компоненты
        // time_t rawtime = epochTime;
        // struct tm * ti = localtime(&rawtime);//Функция localtime из стандартной библиотеки C (<time.h>) преобразует время Unix в структуру tm, содержащую компоненты времени.

        // Преобразование локального времени в структуру tm
        struct tm *ti;
        ti = localtime(&localTime);

        RTC.yyyy = ti->tm_year + 1900;//Значения из структуры tm присваиваются переменным объекта RTC, 
        RTC.mm = ti->tm_mon + 1;//после чего вызывается RTC.writeTime() для записи времени в RTC (см. MD_DS3231::writeTime() в MD_DS3231.cpp).
        RTC.dd = ti->tm_mday;
        RTC.h = ti->tm_hour;
        RTC.m = ti->tm_min;
        RTC.s = ti->tm_sec;
        RTC.dow = ti->tm_wday;

        RTC.writeTime(); // Записываем время в RTC
      }
    // Serial.println("RTC time adjusted via NTP");
    // Serial.print("NTP Epoch Time: "); Serial.println(epochTime);
    // Serial.print("Setting RTC: ");
    // Serial.print(RTC.yyyy); Serial.print("-");
    // Serial.print(RTC.mm); Serial.print("-");
    // Serial.print(RTC.dd); Serial.print(" ");
    // Serial.print(RTC.h); Serial.print(":");
    // Serial.print(RTC.m); Serial.print(":");
    // Serial.print(RTC.s); Serial.print(", dow=");
    // Serial.println(RTC.dow);
}
//**************************************************************************************************
void rtc2mez()
{
 
//    byte Jahr, Tag, Monat, WoTag, Stunde, Minute, Sekunde;

//    Jahr = year();//年
//    if (Jahr > 99)
//        Jahr = 0;
//    Monat = month();//月
//    if (Monat > 12)
//        Monat = 0;
//    Tag = day();//天
//    if (Tag > 31)
//        Tag = 0;
//    WoTag = rtc_wochentag();
//    if (WoTag == 7)
//        WoTag = 0;
//    Stunde = hour(myTZ.toLocal(now()));//小时
//    if (Stunde > 23)
//        Stunde = 0;
//    Minute = minute();//分钟
//    if (Minute > 59)
//        Minute = 0;
//    Sekunde = second();//秒
//    if (Sekunde > 59)
//        Sekunde = 0;
    
    MEZ.WT = WoTag;          //So=0, Mo=1, Di=2 ...
    MEZ.sek1 = Sekunde % 10;
    MEZ.sek2 = Sekunde / 10;
    MEZ.sek12 = Sekunde;
    MEZ.min1 = Minute % 10;
    MEZ.min2 = Minute / 10;
    MEZ.min12 = Minute;
    MEZ.hour1 = Stunde % 10;
    MEZ.hour2 = Stunde / 10;
    MEZ.hour12 = Stunde;
    MEZ.tag12 = Tag;
    MEZ.tag1 = Tag % 10;
    MEZ.tag2 = Tag / 10;
    MEZ.mon12 = Monat;
    MEZ.mon1 = Monat % 10;
    MEZ.mon2 = Monat / 10;
    MEZ.year12 = Jahr;
    MEZ.year1 = Jahr % 10;
    MEZ.year2 = (Jahr % 100) / 10;
}
//**************************************************************************************************
void helpArr_init(void)  //инициализация вспомогательного массива
{
    uint8_t i, j = 0, k = 0;

    for (i = 0; i < NUM_MAX * 8; i++) 
    {
        helpArrPos[i] = (1 << j);   //битовая маска
        helpArrMAX[i] = k;
        j++;
        if (j > 7) 
        {
            j = 0;
            k++;
        }
    }
}
//**************************************************************************************************
void clear_Display()   //очистить все
{
    unsigned char i, j;
    for (i = 0; i < 8; i++)     //8 рядов
    {
        digitalWrite(CS_PIN, LOW);
        delayMicroseconds(1);
        for (j = NUM_MAX; j > 0; j--) 
        {
            LEDarr[j - 1][i] = 0;       //LED очистить
//            SPI.write(i + 1);           //текущая строка
//            SPI.write(LEDarr[j - 1][i]);
            shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, CMD_DIGIT0 + i);
            shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, LEDarr[j - 1][i]);
        }
        digitalWrite(CS_PIN, HIGH);
    }
}
//*********************************************************************************************************
void rotate_90() // for Generic displays
{
    for (uint8_t k = NUM_MAX; k > 0; k--) 
    {

        uint8_t i, j, m, imask, jmask;
        uint8_t tmp[8] = {0,0,0,0,0,0,0,0};
        for (i = 0, imask = 0x80; i < 8; i++, imask >>= 1) 
        {
          for (j = 0, jmask = 0x80; j < 8; j++, jmask >>= 1) 
          {
            if (LEDarr[k-1][i] & jmask) 
            {
              tmp[j] |= imask;
            }
          }
        }
        for(m = 0; m < 8; m++)
        {
            LEDarr[k-1][m] = tmp[m];
        }
    }
}
//**************************************************************************************************
void rotate_270() // for Generic displays
{
    for (uint8_t k = NUM_MAX; k > 0; k--) 
    {

        uint8_t i, j, m, imask, jmask;
        uint8_t tmp[8] = {0,0,0,0,0,0,0,0};
        for (i = 0, imask = 0x01; i < 8; i++, imask <<= 1) 
        {
          for (j = 0, jmask = 0x01; j < 8; j++, jmask <<= 1) 
          {
            if (LEDarr[k-1][i] & jmask) 
            {
              tmp[j] |= imask;
            }
          }
        }
        for(m = 0; m < 8; m++)
        {
            LEDarr[k-1][m] = tmp[m];
        }
    }
}
//**************************************************************************************************
void refresh_display() //принять информацию в LEDarr
{
    unsigned char i, j;

  #ifdef ROTATE_90
      rotate_90();
  #endif
  #ifdef ROTATE_270
      rotate_270();
  #endif

    for (i = 0; i < 8; i++)     //8 рядов
    {
        digitalWrite(CS_PIN, LOW);
        delayMicroseconds(1);
        #ifdef REVERSE_VERTICAL
        for (j = 0; j < NUM_MAX; j++)
        #else
        for (j = NUM_MAX; j > 0; j--) 
        #endif
        {
//            SPI.write(i + 1);  //current row
            #ifdef REVERSE_VERTICAL
            shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, CMD_DIGIT7 - i);
            #else
            shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, CMD_DIGIT0 + i);
            #endif

            #ifdef REVERSE_HORIZONTAL                    //обратный уровень
//                        SPI.setBitOrder(LSBFIRST);      // битовый порядок для обратных столбцов.  Обратный порядок битов столбца
//                        shiftOut(DIN_PIN, CLK_PIN, LSBFIRST, LEDarr[j - 1][i]);
            #endif
            
            #ifdef REVERSE_VERTICAL
//                        SPI.write(LEDarr[j - 1][7-i]);
                        shiftOut(DIN_PIN, CLK_PIN, LSBFIRST, LEDarr[j][i]);
            #else
//                        SPI.write(LEDarr[j - 1][i]);
                        shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, LEDarr[j - 1][i]);
            #endif
            
            #ifdef REVERSE_HORIZONTAL
//                        SPI.setBitOrder(MSBFIRST);      // сбросить битовый порядок
//                        shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, LEDarr[j - 1][i]);
            #endif
        }
        digitalWrite(CS_PIN, HIGH);
    }
}
//**************************************************************************************************
void char2Arr(uint8_t ch, char PosX, short PosY) 
{ //символы в arr
    uint8_t i, j, k, l, m, o1, o2, o3, o4;  //в LEDarr
    PosX++;
    k = ch - 32;                        //Позиция ASCII в шрифте
    if ((k >= 0) && (k < 96))           //символ найден в шрифте?
    {
        o4 = font1[k][0];                 //ширина символа
        o3 = 1 << (o4 - 2);
        for (i = 0; i < o4; i++) 
        {
            if (((PosX - i <= maxPosX) && (PosX - i >= 0)) && ((PosY > -8) && (PosY < 8))) //внутри матрицы?
            {
                o1 = helpArrPos[PosX - i];
                o2 = helpArrMAX[PosX - i];
                for (j = 0; j < 8; j++) 
                {
                    if (((PosY >= 0) && (PosY <= j)) || ((PosY < 0) && (j < PosY + 8))) //прокрутка по вертикали
                    {
                        l = font1[k][j + 1];
                        m = (l & (o3 >> i));  //e.g. o4=7  0zzzzz0, o4=4  0zz0
                        if (m > 0)
                            LEDarr[o2][j - PosY] = LEDarr[o2][j - PosY] | (o1);  //уставка точки
                        else
                            LEDarr[o2][j - PosY] = LEDarr[o2][j - PosY] & (~o1); //очистка точки
                    }
                }
            }
        }
    }
}

void char22Arr(uint8_t ch, char PosX, short PosY) 
{ //characters into arr
    uint8_t i, j, k, l, m, o1, o2, o3, o4;  //в LEDarr
    PosX++;
    k = ch - 32;                        //Позиция ASCII в шрифте
    if ((k >= 0) && (k < 96))           //символ найден в шрифте?
    {
        o4 = font2[k][0];                 //ширина символа
        o3 = 1 << (o4 - 2);
        for (i = 0; i < o4; i++) 
        {
            if (((PosX - i <= maxPosX) && (PosX - i >= 0)) && ((PosY > -8) && (PosY < 8))) //внутри матрицы?
            {
                o1 = helpArrPos[PosX - i];
                o2 = helpArrMAX[PosX - i];
                for (j = 0; j < 8; j++) 
                {
                    if (((PosY >= 0) && (PosY <= j)) || ((PosY < 0) && (j < PosY + 8))) //прокрутка по вертикали
                    {
                        l = font2[k][j + 1];
                        m = (l & (o3 >> i));  //e.g. o4=7  0zzzzz0, o4=4  0zz0
                        if (m > 0)
                            LEDarr[o2][j - PosY] = LEDarr[o2][j - PosY] | (o1);  //уставка точки
                        else
                            LEDarr[o2][j - PosY] = LEDarr[o2][j - PosY] & (~o1); //очистка точки
                    }
                }
            }
        }
    }
}
//**************************************************************************************************
void char2ArrRus(uint8_t ch, char PosX, short PosY) 
{ //символы в arr
    uint8_t i, j, k, l, m, o1, o2, o3, o4;  //в LEDarr
    PosX++;
//    if ((ch >= 128) && (ch <= 159))   ch -= 128;
//        if ((ch >= 160) && (ch <= 175))   ch -= 128;
//            if ((ch >= 224) && (ch <= 239))   ch -= 176;
    if (ch == 0xff)  return;
    k = ch;                        //Позиция ASCII в шрифте
    if ((k >= 0) && (k < 96))           //символ найден в шрифте?
    {
        o4 = Rus[k][0];                 //ширина символа
        o3 = 1 << (o4 - 2);
        for (i = 0; i < o4; i++) 
        {
            if (((PosX - i <= maxPosX) && (PosX - i >= 0)) && ((PosY > -8) && (PosY < 8))) //внутри матрицы?
            {
                o1 = helpArrPos[PosX - i];
                o2 = helpArrMAX[PosX - i];
                for (j = 0; j < 8; j++) 
                {
                    if (((PosY >= 0) && (PosY <= j)) || ((PosY < 0) && (j < PosY + 8))) //прокрутка по вертикали
                    {
                        l = Rus[k][j + 1];
                        m = (l & (o3 >> i));  //e.g. o4=7  0zzzzz0, o4=4  0zz0
                        if (m > 0)
                            LEDarr[o2][j - PosY] = LEDarr[o2][j - PosY] | (o1);  //уставка точки
                        else
                            LEDarr[o2][j - PosY] = LEDarr[o2][j - PosY] & (~o1); //очистка точки
                    }
                }
            }
        }
    }
}
//*****************************************************************************************************
void timer50ms() 
{
    static uint8_t cnt50ms = 0;
    f_tckr50ms = true;
    cnt50ms++;
    if (cnt50ms == 20) 
    {
        f_tckr1s = true; // 1 sec
        cnt50ms = 0;
    }
}
//**************************************************************************************************
//
//The setup function is called once at startup of the sketch
void setup() 
{
    // Serial.begin(9600);
    // Add your initialization code here
/*
    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH);
    Serial.begin(115200);
    SPI.begin();
    helpArr_init();
    max7219_init();
    rtc_init(SDA, SCL);
    clear_Display();
    refresh_display(); //take info into LEDarr
    tckr.attach(0.05, timer50ms);    // every 50 msec
    //////////////////////////////////
    if (!autoConfig())
    {
       smartConfig();
    }
    ///////////////////////////////////
    tm* tt;
    tt = connectNTP();
    if (tt != NULL)
        rtc_set(tt);
    else
        Serial.println("no timepacket received");
    //设置灯亮度
    max7219_set_brightness(0);*/
//-------------------------------------------------------------------------------------------------------------------
    // SerialGPS.begin(GPSBaud);
    initMAX7219();
    delay(100);
    sendCmdAll(CMD_SHUTDOWN, 1);
    sendCmdAll(CMD_INTENSITY, 8);
    delay(100);
    clear_Display();
    helpArr_init();
    // refresh_display(); //take info into LEDarr
    tckr.attach(0.05, timer50ms);    // every 50 msec
    // Wire.begin();
    // rtc.begin();
    // delay(1000);
    // if (!rtc.begin()) 
    // {
    //     // Обработка ошибки, если RTC не найден (например, зациклить или вывести сообщение)
    //     char22Arr('N', 6, 0);
        // Serial.print("no RTC");
        // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // rtc.adjust(DateTime(2025, 9, 11, 20, 50, 0));
    //     refresh_display();
    // }
    // else 
    // {
    //     Serial.print("ok! RTC");
    //     char22Arr('G', 6, 0);
    //     refresh_display();
    // }
//    tckr.attach(0.5, readGPS);
/*byte h, m;
      while (h == 0 && m == 0)
    {
        while (SerialGPS.available())
              {
                GPS.encode(SerialGPS.read());

                if (GPS.time.isValid() && GPS.date.isValid())
                   {
                      h = GPS.time.hour();
                      m = GPS.time.minute();
                   }
              }
      
    }*/

    // WiFi.begin(ssid);
    //           for(byte i = 31; i > 0; i--)
    //           {
    //               if (WiFi.status() != WL_CONNECTED) 
    //               {
    //                 delay(500);
    //                 char22Arr('.', i, 0);
    //                 refresh_display();
    //               }
    //               else
    //               {
    //               char22Arr('O', i, 0);
    //               char22Arr('k', (i - 6), 0);
    //               char22Arr('!', (i - 12), 0);
    //               refresh_display();
    //               delay(4000);
    //               adjust_time_date();
    //               delay(400);
    //               return; // Выходим из setup() после подключения
    //               }
    //           }

    // Пытаемся подключиться к каждой сети
  for (int i = 0; i < numNetworks; i++) 
  {
    // Serial.print("Попытка подключения к ");
    // Serial.println(ssid[i]);

    WiFi.begin(ssid[i], password[i]);

    // Ждем подключения или таймаута (например, 10 секунд)
    int attempts = 31;
    while (WiFi.status() != WL_CONNECTED && attempts > 0) 
        {
        delay(500);
        // Serial.print(".");
        char22Arr('.', attempts, 0);
        refresh_display();
        attempts--;
        }

    // Если подключение успешно, выходим из цикла
    if (WiFi.status() == WL_CONNECTED) 
        {
        // Serial.println("\nПодключено к Wi-Fi!");
        // Serial.print("SSID: ");
        // Serial.println(WiFi.SSID());
        // Serial.print("IP-адрес: ");
        // Serial.println(WiFi.localIP());
        char22Arr('O', attempts, 0);
        char22Arr('k', (attempts - 6), 0);
        char22Arr('!', (attempts - 12), 0);
        refresh_display();
        delay(3000);
        adjust_time_date();
        delay(50);
        break;
        } 
    else 
        {
        // Serial.println("\nНе удалось подключиться к ");
        // Serial.println(ssid[i]);
        clear_Display();
        // WiFi.disconnect(); // Отключаемся перед следующей попыткой
        }
  }// end for

  // Если не удалось подключиться ни к одной сети
  if (WiFi.status() != WL_CONNECTED) 
    {
        // Serial.println("Не удалось подключиться ни к одной сети!");
        char22Arr('W', 31, 0);
        char22Arr('i', 25, 0);
        char22Arr('F', 19, 0);
        char22Arr('i', 13, 0);
        char22Arr('?', 7, 0);
        // char22Arr('F', 1, 0);
        // char22Arr('i', 0, 0);
        refresh_display();
        delay(3000);
        Read__Time();
        delay(40);
    }  


    // timeClient.begin();   

    // set up RTC management
//   RTC.control(DS3231_12H, DS3231_OFF);        // set 24 hr mode
//   RTC.control(DS3231_CLOCK_HALT, DS3231_OFF); // start the clock

}
//**************************************************************************************************
// The loop function is called in an endless loop
void loop() 
{

    //Add your repeated code here
    uint8_t sek1 = 0, sek2 = 0, min1 = 0, min2 = 0, hour1 = 0, hour2 = 0;
    uint8_t sek11 = 0, sek12 = 0, sek21 = 0, sek22 = 0;
    uint8_t min11 = 0, min12 = 0, min21 = 0, min22 = 0;
    uint8_t hour11 = 0, hour12 = 0, hour21 = 0, hour22 = 0;
    int x = 0; //x1,x2;
    int y = 0, y_1 = 0, y2 = 0, y3 = 0;
    bool updown = false;
    uint8_t nomber1 = 0, nomber2 = 0, nomber3 = 0, nomber4 = 0, nomber5 = 0, nomber6 = 0;
    bool f_scrollend_y = false;
    bool f_scroll_x = false;
    char lenn = 7;
    static int jj = 0;
    static uint16_t len = 0;
    
    z_PosX = maxPosX;   //31
    d_PosX = -8;

    // refresh_display();
    updown = true;
    if (updown == false) 
    {
        y2 = -9;
        y_1 = 8;
    }
    if (updown == true) 
    { //scroll  up to down
        y2 = 8;
        y_1 = -8;
    }

//Бесконечный цикл    
    while (1) 
    {
        //   ESP.wdtFeed();
        //   timeClient.update();
        // ESP.wdtDisable(); // Отключает оба WDT (осторожно, может привести к нестабильности)
          if (jj++ > 3000)
          {
            jj = 0;
            // readGPS();
            // ReadTime();
            Read__Time();
            
          }



       yield();
//=============== 1 s =============================
        if (f_tckr1s == true)        // flag 1sek
        {
            rtc2mez();
            sek1 = MEZ.sek1;
            sek2 = MEZ.sek2;
            min1 = MEZ.min1;
            min2 = MEZ.min2;
            hour1 = MEZ.hour1;
            hour2 = MEZ.hour2;
            y = y2;                 //scroll updown
            nomber1 = 1;
            sek1++;
            if (sek1 == 10) 
            {
                nomber2 = 1;
                sek2++;
                sek1 = 0;
            }
                if (sek2 == 6) 
                {
                    min1++;
                    sek2 = 0;
                    nomber3 = 1;
                }
                    if (min1 == 10) 
                    {
                        min2++;
                        min1 = 0;
                        nomber4 = 1;
                    }
                        if (min2 == 6) 
                        {
                            hour1++;
                            min2 = 0;
                            nomber5 = 1;
                        }
                            if (hour1 == 10) 
                            {
                                hour2++;
                                hour1 = 0;
                                nomber6 = 1;
                            }
                                if ((hour2 == 2) && (hour1 == 4)) 
                                {
                                    hour1 = 0;
                                    hour2 = 0;
                                    nomber6 = 1;
                                }

            sek11 = sek12;
            sek12 = sek1;
            sek21 = sek22;
            sek22 = sek2;
            min11 = min12;
            min12 = min1;
            min21 = min22;
            min22 = min2;
            hour11 = hour12;
            hour12 = hour1;
            hour21 = hour22;
            hour22 = hour2;
            f_tckr1s = false;
            // if (MEZ.sek12 == 15)      f_scroll_x = true;//滚动开关
            // if (MEZ.sek12 == 30)      f_scroll_x = true;//滚动开关
            if (MEZ.sek12 == 45)      f_scroll_x = true;//滚动开关
            // if (MEZ.sek12 == 0)      f_scroll_x = true;//滚动开关
        }
//============ end 1s =====================
//==== 50 ms ==============================        
        if (f_tckr50ms == true) 
        {
            f_tckr50ms = false;
            
            if (f_scroll_x == true) //---------------------------------------------- Scrolling -------------------------------
            {
                z_PosX++;
                d_PosX++;
                if (d_PosX == 183)//101
                    z_PosX = 0;
                if (z_PosX == maxPosX) //maxPosX = 31
                {
                    f_scroll_x = false;
                    d_PosX = -8;
                }
            }//---------------------------- end scrolling ---------------------------------------
            if (nomber1 == 1)//--------------------------- nomber 1 ------------------------------- 
            {
                if (updown == 1)
                    y--;
                else
                     y++;
                     y3 = y;
               if (y3 > 0) 
                {
                   y3 = 0;
                }

                char22Arr(48 + sek12, z_PosX - sec1_position, y3);  //31 - 27 = 4
                char22Arr(48 + sek11, z_PosX - sec1_position, y + y_1);  //31 - 27 = 4

                if (y == 0) 
                {
                    nomber1 = 0;
                    f_scrollend_y = true;
                }
            }//-------------------------------------- end nomber 1 --------------------------------------
            else

                char22Arr(48 + sek1, z_PosX - sec1_position, 0);//31 -  27 = 4


            if (nomber2 == 1)//-------------------------- nomber 2 ------------------------------------------ 
            {

                char22Arr(48 + sek22, z_PosX - sec2_position, y3);//31 - 23 = 8
                char22Arr(48 + sek21, z_PosX - sec2_position, y + y_1);//31 - 23 = 8

                if (y == 0)
                    nomber2 = 0;
            }//-------------------------------------- end nomber 2 --------------------------------------
            else

                char22Arr(48 + sek2, z_PosX - sec2_position, 0);//31 - 23 = 8


            if (nomber3 == 1)//-------------------------- nomber 3 ------------------------------------------ 
            {

                char2Arr(48 + min12, z_PosX - min1_position, y);//31 - 19 = 12
                char2Arr(48 + min11, z_PosX - min1_position, y + y_1);//31 - 19 = 12

                if (y == 0)
                    nomber3 = 0;
            }//-------------------------------------- end nomber 3 --------------------------------------
            else

                char2Arr(48 + min1, z_PosX - min1_position, 0);//31 - 19 = 12


            if (nomber4 == 1)//-------------------------- nomber 4 ------------------------------------------ 
            {

                char2Arr(48 + min22, z_PosX - min2_position, y);//31 - 14 = 17
                char2Arr(48 + min21, z_PosX - min2_position, y + y_1);//31 - 14 = 17

                if (y == 0)
                    nomber4 = 0;
            }//-------------------------------------- end nomber 4 --------------------------------------
            else

                char2Arr(48 + min2, z_PosX - min2_position, 0);//31 - 14 = 17
//                if (jg++ > 3000)
//                  {
//                    jg = 0;
                    char2Arr(':', z_PosX - colon_position + x, 0);//31 - 11 = 20
//                  }

                

            if (nomber5 == 1)//-------------------------- nomber 5 ------------------------------------------ 
            {

                char2Arr(48 + hour12, z_PosX - hour1_position, y);//31 - 4 = 27
                char2Arr(48 + hour11, z_PosX - hour1_position, y + y_1);//31 - 4 = 27

                if (y == 0)
                    nomber5 = 0;
            }//-------------------------------------- end nomber 5 --------------------------------------
            else

                char2Arr(48 + hour1, z_PosX - hour1_position, 0);//31 - 4 = 27


            if (nomber6 == 1)//-------------------------- nomber 6 ------------------------------------------ 
            {

                char2Arr(48 + hour22, z_PosX + hour2_position, y);//31 + 1 = 32
                char2Arr(48 + hour21, z_PosX + hour2_position, y + y_1);//31 + 1 = 32

                if (y == 0)
                    nomber6 = 0;
            }//-------------------------------------- end nomber 6 --------------------------------------
            else

                char2Arr(48 + hour2, z_PosX + hour2_position, 0);//31 + 1 = 32


            char2Arr(' ', d_PosX + 5, 0);        //day of the week
            #ifdef Russian
//            dtostrf(WT_rus[MEZ.WT][4], 3, 0, szMesg);
//            char2Arr(szMesg[0], d_PosX - 1, 0);        //day of the week
//            char2Arr(szMesg[1], d_PosX - 7, 0);
//            char2Arr(szMesg[2], d_PosX - 13, 0);
//            char2Arr(szMesg[3], d_PosX - 19, 0);
            len += 1;
            char2ArrRus(WT_rus[MEZ.WT][0], d_PosX - len, 0); //1       //day of the week
            len += 6;
            char2ArrRus(WT_rus[MEZ.WT][1], d_PosX - len, 0);//7
            len += 6;
            char2ArrRus(WT_rus[MEZ.WT][2], d_PosX - len, 0);//13
            len += 6;
            char2ArrRus(WT_rus[MEZ.WT][3], d_PosX - len, 0);//19
            len += 6;
            char2ArrRus(WT_rus[MEZ.WT][4], d_PosX - len, 0);//25
            len += 6;
            char2ArrRus(WT_rus[MEZ.WT][5], d_PosX - len, 0);//31
            len += 6;
            char2ArrRus(WT_rus[MEZ.WT][6], d_PosX - len, 0);//37
            len += 6;
            char2ArrRus(WT_rus[MEZ.WT][7], d_PosX - len, 0);//43
            len += 6;
            char2ArrRus(WT_rus[MEZ.WT][8], d_PosX - len, 0);//49
            len += 6;
            char2ArrRus(WT_rus[MEZ.WT][9], d_PosX - len, 0);//43
            len += 6;
            char2ArrRus(WT_rus[MEZ.WT][10], d_PosX - len, 0);//49
            len += 7;
            char2Arr(48 + MEZ.tag2, d_PosX - len, 0); //56          //day
            len += 6;
            char2Arr(48 + MEZ.tag1, d_PosX - len, 0);//62
            len += 6;
            char2Arr(' ', d_PosX + len, 0);
            len += 3;
            char2ArrRus(M_rus[MEZ.mon12][0], d_PosX - len, 0);//69 //month
            len += 6;
            char2ArrRus(M_rus[MEZ.mon12][1], d_PosX - len, 0);//75
            len += 6;
            char2ArrRus(M_rus[MEZ.mon12][2], d_PosX - len, 0);//81
            len += 6;
            char2ArrRus(M_rus[MEZ.mon12][3], d_PosX - len, 0);//87
            len += 6;
            char2ArrRus(M_rus[MEZ.mon12][4], d_PosX - len, 0);//93
            len += 6;
            char2ArrRus(M_rus[MEZ.mon12][5], d_PosX - len, 0);//99
            len += 6;
            char2ArrRus(M_rus[MEZ.mon12][6], d_PosX - len, 0);//105
            len += 6;
            char2ArrRus(M_rus[MEZ.mon12][7], d_PosX - len, 0);//111
            len += 6;
            char2ArrRus(M_rus[MEZ.mon12][8], d_PosX - len, 0);//117
//            len += 6;
//            char2ArrRus(M_rus[MEZ.mon12][9], d_PosX - len, 0);//123
//            len += 6;
//            char2ArrRus(M_rus[MEZ.mon12][10], d_PosX - len, 0);//129
//            len += 6;
//            char2ArrRus(M_rus[MEZ.mon12][11], d_PosX - len, 0);//135
//            char2ArrRus(M_rus[MEZ.mon12 - 1][3], d_PosX - len, 0);
//            char2ArrRus(M_rus[MEZ.mon12 - 1][4], d_PosX - len, 0);
            len += 6;
            char2Arr('2', d_PosX - len, 0);//142                     //year
            len += 6;
            char2Arr('0', d_PosX - len, 0);//148
            len += 6;
            char2Arr(48 + MEZ.year2, d_PosX - len, 0);//154
            len += 6;
            char2Arr(48 + MEZ.year1, d_PosX - len, 0);//160
            len += 6;
            char2Arr(' ', d_PosX - len, 0);//167
            #else
            char2Arr(WT_arr[MEZ.WT][0], d_PosX - 1, 0);        //day of the week
            char2Arr(WT_arr[MEZ.WT][1], d_PosX - 7, 0);
            char2Arr(WT_arr[MEZ.WT][2], d_PosX - 13, 0);
            char2Arr(WT_arr[MEZ.WT][3], d_PosX - 19, 0);
            char2Arr(48 + MEZ.tag2, d_PosX - 26, 0);           //day
            char2Arr(48 + MEZ.tag1, d_PosX - 32, 0);
            char2Arr(M_arr[MEZ.mon12 - 1][0], d_PosX - 39, 0); //month
            char2Arr(M_arr[MEZ.mon12 - 1][1], d_PosX - 43, 0);
            char2Arr(M_arr[MEZ.mon12 - 1][2], d_PosX - 49, 0);
            char2Arr(M_arr[MEZ.mon12 - 1][3], d_PosX - 55, 0);
            char2Arr(M_arr[MEZ.mon12 - 1][4], d_PosX - 61, 0);
            char2Arr('2', d_PosX - 68, 0);                     //year
            char2Arr('0', d_PosX - 74, 0);
            char2Arr(48 + MEZ.year2, d_PosX - 80, 0);
            char2Arr(48 + MEZ.year1, d_PosX - 86, 0);
            char2Arr(' ', d_PosX - 93, 0); 
            #endif
            len = 0;
            refresh_display(); //каждые 50 мс

            if (f_scrollend_y == true)    f_scrollend_y = false;
        }
//======= end 50ms =====================================
        if (y == 0) 
        {
            // do something else
        }

        // yield(); // Разрешить системные задачи
    // delay(1); // Минимальная задержка

    }  //end while(true)
    //this section can not be reached
}
//---------------------------------------------------------------------------------------------------------------------
// void readGPS()
// {
//     while (SerialGPS.available())
//     {
//       GPS.encode(SerialGPS.read());
//       if (GPS.time.isValid() && GPS.date.isValid())
//       {
//         setTime(GPS.time.hour(), GPS.time.minute(), GPS.time.second(), GPS.date.day(), GPS.date.month(), GPS.date.year());
//           Jahr = year();
//           Monat = month();
//           Tag = day();
//           Stunde = hour(myTZ.toLocal(now()));
//           Minute = minute();
//           Sekunde = second();
// //          Формула расчета Кима Ларсона  W = (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) mod 7  http://baike.baidu.com/view/739374.htm
// //                                        int iWeek = (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7;
// //          WoTag = (Tag + 2 * Monat + 3 * (Monat + 1) / 5 + Jahr + Jahr / 4 - Jahr / 100 + Jahr / 400) % 7;
//           WoTag = weekday() - 1;
//       }
//     }
// }



//bc}~⌂АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмноп░▒▓│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚═╤╥╙▄▀рстуфхцчшщъыьэюяЁёЄєЇїЎў°∙·√№¤■ 

