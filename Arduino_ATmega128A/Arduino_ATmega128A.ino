// ATmega64a 8MГц Внутренний. Дата создания 14.10.2025  18:43

#include <Arduino.h>
#include "microDS18B20.h"
#include <EEPROM.h>
#include "GyverBME280.h"
//#include <GyverHTU21D.h>
#include "microDS3231.h"
#include "EncButton.h"
#include "AHT20.h"




// Пины дисплея/матрицы/устройства на порту C
#define SI     35   // PC0
#define CLK    36   // PC1
#define LAT    37   // PC2
#define BLK    38   // PC3

//Энкодер
#define A      39   // PC4 (вход)
#define C      40   // PC5 (вход)
#define S1     41   // PC6 (вход)

//Звук
#define Buzzer 35   // PC7

#define DS18b20 43  //PG2

//Датчик температуры DS18B20
#define photoresistor    A0   // 45 PF0

//#define EB_HOLD 1000   // таймаут удержания кнопки, мс
// #define EB_STEP 200    // период срабатывания степ, мс

#define INIT_ADDR 1023  // номер резервной ячейки
#define INIT_KEY 102     // ключ первого запуска. (101) 0-254, на выбор. Сначало прошить с ключём, а потом изменить ключ и прошить.

// для изменения направления энкодера поменяй A и B при инициализации
// по умолчанию пины настроены в INPUT_PULLUP
// EncButton<EB_TICK, 7, 6, 17> enc;  // энкодер с кнопкой <A, B, KEY>
EncButton<EB_TICK, A, C, S1> enc;

MicroDS18B20 <DS18b20> DS;  // датчик на A0 НЕ ЗАБЫВАЕМ ПОДКЛЮЧИТЬ PULL-UP РЕЗИСТОР!!!
MicroDS3231 rtc;
GyverBME280 bme;
AHT20 aht20;

uint32_t dots_timer = 0, light_timer = 0, bme_timer = 0, scroll_timer = 0, shift_timer = 0, sandclock_timer = 0;//__Переменные счетчиков
bool beep_start_flag, DS_online, bme_online, htu_online, ds, tflag, scroll_flag, beep_flag = 1, dot_flag = 0, shift, scroll, shift_flag; //____Служебные переменные
byte menu_item = 1, Display_ID = 128, data, cursor_x = 0, scroll_x, scroll_time = 75, shift_time = 30; //____Служебные переменные
byte symb_scroll = 3, screen_shift = 4, set_view = 0, set_view_v = 0, set_param = 0, font = 0, font_v = 1; //____Служебные переменные
byte dots, dots_up, dots_down, dot_l_vert, dot_r_vert, dots_vert,  arrow_left, arrow_right;
int scr, NIGHT = 80, DAY = 100, COR_TEMP = 0;
int  chime_start = 9, chime_end = 23, chime_enable = 1;
byte temp_indoor_c1, temp_indoor_c2, temp_indoor_d, t_sign_indoor; //_Переменные для отображения температуры  в помещении
//byte temp_BME_c1, temp_BME_c2, temp_BME_d, t_sign_BME; //_Переменные для отображения температуры  с датчика BME280
//byte temp_RTC_c1, temp_RTC_c2, temp_RTC_d, t_sign_RTC; //_Переменные для отображения температуры  с датчика DS3231
byte temp_DS_c1, temp_DS_c2, temp_DS_d, temp_DS_10, temp_DS_01;     //_Переменные для отображения температуры  с датчика DS18B20
byte hum_round_c1, hum_round_c2, hum_round_d;  //___________Переменные для отображения влажности с датчика BME280
byte pres_round_c1, pres_round_c2, pres_round_c3;  //_______Переменные для отображения давления с датчика BME280
int light, dim, dim_level = 3;  //___________Переключение яркости дисплея
float temp; // переменная для хранения температуры
int DS_temp;  // переменная для хранения температуры
bool vert, cdt_run = 0;
byte sand = 0;
int cdt_m = 0, cdt_s = 0;

uint8_t LED1 = 0;

byte Symbol_buf[2][15] =  { 0 };
byte Scroll[2] = { 0 };

#include "CharGen.h"  //Знакогенератор
#include "CharGen_bold.h"  //Знакогенератор
#include "CharGen_v.h"  //Знакогенератор
#include "VFD.h"

// Инициализация направления:
// PC0–PC3, PC7 — выходы; PC4–PC6 — входы
// static inline void io_init_pc(void) 
// {
//   // Выходы
//   DDRC |= _BV(SI) | _BV(CLK) | _BV(LAT) | _BV(BLK) | _BV(PIZZLE);
//   // Входы
//   DDRC &= ~(_BV(A) | _BV(C) | _BV(S1));
//   // Если нужны подтяжки на входах, раскомментируй:
//   PORTC |= _BV(A) | _BV(C) | _BV(S1);
// }

const byte BigNum[10][6] PROGMEM = 
{   //____Широкие цифры
  {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5}, //"0"
  {0xB6, 0xB7, 0x20, 0xB8, 0xB9, 0xBA}, //"1"
  {0xBB, 0xB1, 0xBC, 0xB0, 0xBD, 0xBE}, //"2"
  {0xBB, 0xBF, 0xC0, 0xC1, 0xB4, 0xC2}, //"3"
  {0xC3, 0xC4, 0x20, 0xC5, 0xC6, 0xC7}, //"4"
  {0xC8, 0xBD, 0xC9, 0xC1, 0xB4, 0xC2}, //"5"
  {0xB0, 0xCA, 0xCB, 0xB3, 0xB4, 0xC2}, //"6"
  {0xC5, 0xCC, 0xCD, 0x20, 0xCE, 0x20}, //"7"
  {0xCF, 0xBD, 0xC0, 0xD0, 0xB4, 0xC2}, //"8"
  {0xD1, 0xB1, 0xB2, 0xD2, 0xD3, 0xB5}, //"9"
};

const byte ddd[7][3] PROGMEM = 
{   //___Дни недели
  {0x0F, 0xEE, 0xED}, //Пон
  {0x02, 0xF2, 0xEE}, //Вто
  {0x11, 0xF0, 0xE5}, //Сре
  {0x17, 0xE5, 0xF2}, //Чет
  {0x0F, 0xFF, 0xF2}, //Пят
  {0x11, 0xF3, 0xE1}, //Суб
  {0x02, 0xEE, 0xF1}, //Вос
};

const byte mmm[12][3] PROGMEM = 
{   //___Месяцы
  {0x1F, 0xED, 0xE2}, //Янв
  {0x14, 0xE5, 0xE2}, //Фев
  {0x0C, 0xE0, 0xF0}, //Мар
  {0x00, 0xEF, 0xF0}, //Апр
  {0x0C, 0xE0, 0xFF}, //Мая
  {0x08, 0xFE, 0xED}, //Июн
  {0x08, 0xFE, 0xEB}, //Июл
  {0x00, 0xE2, 0xE3}, //Авг
  {0x11, 0xE5, 0xED}, //Сен
  {0x0E, 0xEA, 0xF2}, //Окт
  {0x0D, 0xEE, 0xFF}, //Ноя
  {0x04, 0xE5, 0xEA}, //Дек
};

const byte ddd_full[7][11] PROGMEM = 
{   //___Дни недели полностью
  {0x0F, 0x0E, 0x0D, 0x05, 0x04, 0x05, 0x0B, 0x1C, 0x0D, 0x08, 0x0A}, //ПОНЕДЕЛЬНИК
  {0x20, 0x20, 0x02, 0x12, 0x0E, 0x10, 0x0D, 0x08, 0x0A, 0x20, 0x20}, //  ВТОРНИК
  {0x20, 0x20, 0x20, 0x11, 0x10, 0x05, 0x04, 0x00, 0x20, 0x20, 0x20}, //   СРЕДА
  {0x20, 0x20, 0x17, 0x05, 0x12, 0x02, 0x05, 0x10, 0x03, 0x20, 0x20}, //  ЧЕТВЕРГ
  {0x20, 0x20, 0x0F, 0x1F, 0x12, 0x0D, 0x08, 0x16, 0x00, 0x20, 0x20}, //  ПЯТНИЦА
  {0x20, 0x20, 0x11, 0x13, 0x01, 0x01, 0x0E, 0x12, 0x00, 0x20, 0x20}, //  СУББОТА
  {0x02, 0x0E, 0x11, 0x0A, 0x10, 0x05, 0x11, 0x05, 0x0D, 0x1C, 0x05}, //ВОСКРЕСЕНЬЕ
};

const byte mmm_full[12][8] PROGMEM = 
{   //___Месяцы полностью
  {0x1F, 0x0D, 0x02, 0x00, 0x10, 0x1F, 0x20, 0x20}, //ЯНВАРЯ
  {0x14, 0x05, 0x02, 0x10, 0x00, 0x0B, 0x1F, 0x20}, //ФЕВРАЛЯ
  {0x0C, 0x00, 0x10, 0x12, 0x00, 0x20, 0x20, 0x20}, //МАРТА
  {0x00, 0x0F, 0x10, 0x05, 0x0B, 0x1F, 0x20, 0x20}, //АПРЕЛЯ
  {0x0C, 0x00, 0x1F, 0x20, 0x20, 0x20, 0x20, 0x20}, //МАЯ
  {0x08, 0x1E, 0x0D, 0x1F, 0x20, 0x20, 0x20, 0x20}, //ИЮНЯ
  {0x08, 0x1E, 0x0B, 0x1F, 0x20, 0x20, 0x20, 0x20}, //ИЮЛЯ
  {0x00, 0x02, 0x03, 0x13, 0x11, 0x12, 0x00, 0x20}, //АВГУСТА
  {0x11, 0x05, 0x0D, 0x12, 0x1F, 0x01, 0x10, 0x1F}, //СЕНТЯБРЯ
  {0x0E, 0x0A, 0x12, 0x1F, 0x01, 0x10, 0x1F, 0x20}, //ОКТЯБРЯ
  {0x0D, 0x0E, 0x1F, 0x01, 0x10, 0x1F, 0x20, 0x20}, //НОЯБРЯ
  {0x04, 0x05, 0x0A, 0x00, 0x01, 0x10, 0x1F, 0x20}, //ДЕКАБРЯ
};

const byte setscreen[6][20] PROGMEM = 
{   //___Экраны установки даты/времени
  {0x20, 0x20, 0x13, 0x11, 0x12, 0x00, 0x0D, 0x0E, 0x02, 0x0A, 0x00, 0x20, 0x17, 0x00, 0x11, 0x0E, 0x02, 0x20, 0x20, 0x20}, //"  УСТАНОВКА ЧАСОВ   "
  {0x20, 0x20, 0x13, 0x11, 0x12, 0x00, 0x0D, 0x0E, 0x02, 0x0A, 0x00, 0x20, 0x0C, 0x08, 0x0D, 0x13, 0x12, 0x20, 0x20, 0x20}, //"  УСТАНОВКА МИНУТ   "
  {0x20, 0x20, 0x20, 0x20, 0x11, 0x01, 0x10, 0x0E, 0x11, 0x20, 0x11, 0x05, 0x0A, 0x13, 0x0D, 0x04, 0x20, 0x20, 0x20, 0x20}, //"    СБРОС СЕКУНД    "
  {0x20, 0x20, 0x13, 0x11, 0x12, 0x00, 0x0D, 0x0E, 0x02, 0x0A, 0x00, 0x20, 0x17, 0x08, 0x11, 0x0B, 0x00, 0x20, 0x20, 0x20}, //"  УСТАНОВКА ЧИСЛА   "
  {0x20, 0x20, 0x13, 0x11, 0x12, 0x00, 0x0D, 0x0E, 0x02, 0x0A, 0x00, 0x20, 0x0C, 0x05, 0x11, 0x1F, 0x16, 0x00, 0x20, 0x20}, //"  УСТАНОВКА МЕСЯЦА  "
  {0x20, 0x20, 0x13, 0x11, 0x12, 0x00, 0x0D, 0x0E, 0x02, 0x0A, 0x00, 0x20, 0x03, 0x0E, 0x04, 0x00, 0x20, 0x20, 0x20, 0x20}, //"  УСТАНОВКА ГОДА    "
};

const byte Err[40] PROGMEM = 
{
  0x20, 0x20, 0x20, 0x20, 0x0E, 0x18, 0x08, 0x01, 0x0A, 0x00, 0x20, 0x11, 0x02, 0x1F, 0x07, 0x08, 0x20, 0x20, 0x20, 0x20, //"    ОШИБКА СВЯЗИ    "
  0x11, 0x20, 0x0C, 0x0E, 0x04, 0x13, 0x0B, 0x05, 0x0C, 0x20, 0x52, 0x54, 0x43, 0x20, 0x44, 0x53, 0x33, 0x32, 0x33, 0x31  //"С МОДУЛЕМ RTC DS3231"
};

const byte menu_screen[10][20] PROGMEM = 
{   //___Экраны меню настроек
  {0x2A, 0x2A, 0x2A, 0x0C, 0x05, 0x0D, 0x1E, 0x20, 0x0D, 0x00, 0x11, 0x12, 0x10, 0x0E, 0x05, 0x0A, 0x2A, 0x2A, 0x2A, 0x20}, //  "***МЕНЮ НАСТРОЕК*** "
  {0xAC, 0x20, 0x0D, 0x00, 0x11, 0x12, 0x10, 0x0E, 0x09, 0x0A, 0x08, 0x20, 0x00, 0x0D, 0x08, 0x0C, 0x00, 0x16, 0x08, 0x08}, //  "> НАСТРОЙКИ АНИМАЦИИ"
  {0xAC, 0x20, 0x04, 0x00, 0x12, 0x00, 0x2F, 0x02, 0x10, 0x05, 0x0C, 0x1F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20}, //  "> ДАТА/ВРЕМЯ        "
  {0xAC, 0x20, 0x0D, 0x00, 0x11, 0x12, 0x10, 0x0E, 0x09, 0x0A, 0x08, 0x20, 0x11, 0x08, 0x03, 0x0D, 0x00, 0x0B, 0x00, 0x20}, //  "> НАСТРОЙКИ СИГНАЛА "
  {0xAC, 0x20, 0x0D, 0x00, 0x11, 0x12, 0x10, 0x0E, 0x09, 0x0A, 0x08, 0x20, 0x1F, 0x10, 0x0A, 0x0E, 0x11, 0x12, 0x08, 0x20}, //  "> НАСТРОЙКИ ЯРКОСТИ "
  {0xAC, 0x20, 0x0A, 0x0E, 0x10, 0x10, 0x20, 0x12, 0x05, 0x0C, 0x0F, 0x05, 0x10, 0x00, 0x12, 0x13, 0x10, 0x1B, 0x20, 0x20}, //  "> КОРР ТЕМПЕРАТУРЫ  "
  {0xAC, 0x20, 0x02, 0x1B, 0x15, 0x0E, 0x04, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20}, //  "> ВЫХОД             "
  {0x11, 0x08, 0x03, 0x0D, 0x00, 0x0B, 0x20, 0x0D, 0x00, 0x17, 0x00, 0x0B, 0x0E, 0x20, 0x20, 0x0A, 0x0E, 0x0D, 0x05, 0x16}, //  "СИГНАЛ НАЧАЛО  КОНЕЦ"
  {0x11, 0x02, 0x05, 0x12, 0x20, 0x20, 0x0D, 0x0E, 0x17, 0x1C, 0x20, 0x04, 0x05, 0x0D, 0x1C, 0x20, 0x20, 0x1F, 0x10, 0x0A}, //  "СВЕТ  НОЧЬ ДЕНЬ  ЯРК"
  {0x20, 0x0A, 0x0E, 0x10, 0x10, 0x05, 0x0A, 0x16, 0x08, 0x1F, 0x20, 0x04, 0x00, 0x12, 0x17, 0x08, 0x0A, 0x00, 0x20, 0x20}, //  " КОРРЕКЦИЯ ДАТЧИКА  "
};

const byte sandclock[28][16] PROGMEM = 
{
  {0x97, 0x60, 0x60, 0x85, 0x8A, 0x72, 0x72, 0x96, 0x97, 0x61, 0x61, 0x84, 0x8B, 0x73, 0x73, 0x96}, //  0
  {0x97, 0x74, 0x60, 0x85, 0x8A, 0x72, 0x70, 0x96, 0x97, 0x75, 0x61, 0x84, 0x8B, 0x73, 0x71, 0x96}, //  1
  {0x97, 0x76, 0x60, 0x85, 0x8A, 0x72, 0x6E, 0x96, 0x97, 0x77, 0x61, 0x84, 0x8B, 0x73, 0x6F, 0x96}, //  2
  {0x97, 0x78, 0x60, 0x85, 0x8A, 0x72, 0x6C, 0x96, 0x97, 0x79, 0x61, 0x84, 0x8B, 0x73, 0x6D, 0x96}, //  3
  {0x97, 0x7A, 0x60, 0x85, 0x8A, 0x72, 0x6A, 0x96, 0x97, 0x7B, 0x61, 0x84, 0x8B, 0x73, 0x6B, 0x96}, //  4
  {0x97, 0x7C, 0x60, 0x85, 0x8A, 0x72, 0x68, 0x96, 0x97, 0x7D, 0x61, 0x84, 0x8B, 0x73, 0x69, 0x96}, //  5
  {0x97, 0x7E, 0x60, 0x85, 0x8A, 0x72, 0x66, 0x96, 0x97, 0x7F, 0x61, 0x84, 0x8B, 0x73, 0x67, 0x96}, //  6
  {0x97, 0x80, 0x60, 0x85, 0x8A, 0x72, 0x64, 0x96, 0x97, 0x81, 0x61, 0x84, 0x8B, 0x73, 0x65, 0x96}, //  7
  {0x97, 0x82, 0x60, 0x85, 0x8A, 0x72, 0x62, 0x96, 0x97, 0x83, 0x61, 0x84, 0x8B, 0x73, 0x63, 0x96}, //  8
  {0x97, 0x72, 0x60, 0x85, 0x8A, 0x72, 0x60, 0x96, 0x97, 0x73, 0x61, 0x84, 0x8B, 0x73, 0x61, 0x96}, //  9
  {0x97, 0x72, 0x74, 0x85, 0x8A, 0x70, 0x60, 0x96, 0x97, 0x73, 0x75, 0x84, 0x8B, 0x71, 0x61, 0x96}, //  10
  {0x97, 0x72, 0x76, 0x85, 0x8A, 0x6E, 0x60, 0x96, 0x97, 0x73, 0x77, 0x84, 0x8B, 0x6F, 0x61, 0x96}, //  11
  {0x97, 0x72, 0x78, 0x85, 0x8A, 0x6C, 0x60, 0x96, 0x97, 0x73, 0x79, 0x84, 0x8B, 0x6D, 0x61, 0x96}, //  12
  {0x97, 0x72, 0x7A, 0x85, 0x8A, 0x6A, 0x60, 0x96, 0x97, 0x73, 0x7B, 0x84, 0x8B, 0x6B, 0x61, 0x96}, //  13
  {0x97, 0x72, 0x7C, 0x85, 0x8A, 0x68, 0x60, 0x96, 0x97, 0x73, 0x7D, 0x84, 0x8B, 0x69, 0x61, 0x96}, //  14
  {0x97, 0x72, 0x7E, 0x85, 0x8A, 0x66, 0x60, 0x96, 0x97, 0x73, 0x7F, 0x84, 0x8B, 0x67, 0x61, 0x96}, //  15
  {0x97, 0x72, 0x80, 0x85, 0x8A, 0x64, 0x60, 0x96, 0x97, 0x73, 0x81, 0x84, 0x8B, 0x65, 0x61, 0x96}, //  16
  {0x97, 0x72, 0x82, 0x85, 0x8A, 0x62, 0x60, 0x96, 0x97, 0x73, 0x83, 0x84, 0x8B, 0x63, 0x61, 0x96}, //  17
  {0x97, 0x72, 0x72, 0x85, 0x8A, 0x60, 0x60, 0x96, 0x97, 0x73, 0x73, 0x84, 0x8B, 0x61, 0x61, 0x96}, //  18
  {0x97, 0x72, 0x72, 0x8C, 0x88, 0x60, 0x60, 0x96, 0x97, 0x73, 0x73, 0x8D, 0x89, 0x61, 0x61, 0x96}, //  19
  {0x97, 0x72, 0x72, 0x8E, 0x86, 0x60, 0x60, 0x96, 0x97, 0x73, 0x73, 0x8F, 0x87, 0x61, 0x61, 0x96}, //  20
  {0x97, 0x72, 0x72, 0x90, 0x84, 0x60, 0x60, 0x96, 0x97, 0x73, 0x73, 0x91, 0x85, 0x61, 0x61, 0x96}, //  21
  {0x97, 0x72, 0x72, 0x90, 0x84, 0x60, 0x60, 0x96, 0x97, 0x73, 0x73, 0x91, 0x85, 0x61, 0x61, 0x96}, //  22
  {0x20, 0x97, 0x72, 0x90, 0x84, 0x60, 0x96, 0x20, 0x20, 0x97, 0x73, 0x91, 0x85, 0x61, 0x96, 0x20}, //  23
  {0x20, 0x20, 0x97, 0x90, 0x84, 0x96, 0x20, 0x20, 0x20, 0x20, 0x97, 0x91, 0x85, 0x96, 0x20, 0x20}, //  24
  //  {0x20, 0x20, 0x97, 0x94, 0x92, 0x96, 0x20, 0x20, 0x20, 0x20, 0x97, 0x95, 0x93, 0x96, 0x20, 0x20}, //  25
  {0x20, 0x20, 0x97, 0x85, 0x8A, 0x96, 0x20, 0x20, 0x20, 0x20, 0x97, 0x84, 0x8B, 0x96, 0x20, 0x20}, //  26
  {0x20, 0x97, 0x60, 0x85, 0x8A, 0x72, 0x96, 0x20, 0x20, 0x97, 0x61, 0x84, 0x8B, 0x73, 0x96, 0x20}, //  27
  {0x97, 0x60, 0x60, 0x85, 0x8A, 0x72, 0x72, 0x96, 0x97, 0x61, 0x61, 0x84, 0x8B, 0x73, 0x73, 0x96}  //  28
};

byte DDRAM[4][30] = 
{                      // DDRAM
  { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    //  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
  },
  { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    //  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
  },
  { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    //  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
  },
  { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    //  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
  }
};
//---------------------------------------------------------------------------------------------------------

void setup() 
{
  // Настройка выходов
  pinMode(SI, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(LAT, OUTPUT);
  pinMode(BLK, OUTPUT);
  pinMode(Buzzer, OUTPUT);  //Зуммер
  pinMode(DS18b20, OUTPUT);
  pinMode(PA0, OUTPUT);
  digitalWrite(PA0, LOW);  

  // Настройка входов
  pinMode(A, INPUT_PULLUP);
  pinMode(C, INPUT_PULLUP);
  pinMode(S1, INPUT_PULLUP);

  // Настройка таймера 1 на прерывание для ATmega64A (8 МГц)
  cli();  // Отключить прерывания на время настройки
  TCCR1A = 0b00000000;    // Нормальный режим (WGM10=0, WGM11=0), OC1A отключен (COM1A1=0, COM1A0=0)
  TCCR1B = 0b00001011;    // CTC режим (WGM13=0, WGM12=1), prescaler 64 (CS12=0, CS11=1, CS10=1)
  // OCR1A для ~1984 Гц: (F_CPU / prescaler / freq) - 1 = (8e6 / 64 / 1984) - 1 ≈ 63 - 1 = 62
  OCR1A = 125;             // ~1984 Гц частота сканирования сеток дисплея
  TIMSK = 0b00010000;    // Включить прерывание по compared match A (OCIE1A=1)
  sei();  // Включить прерывания

    // Преднастройка АЦП для ускорения функции "analogRead"
  // Делитель АЦП = 64 (ADPS2:0 = 0b110) → 8 МГц / 64 = 125 кГц
  // ADCSRA = (ADCSRA & ~(_BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0))) | _BV(ADPS2) | _BV(ADPS1);

  if (!rtc.begin()) { // Определяем, подключен ли модуль часов DS3231
    // show_error(); // Если нет, выдаём сообщение об ошибке
    // for (;;);
     
  }

//   if (EEPROM.read(INIT_ADDR) != INIT_KEY) 
//       { // первый запуск
//         EEPROM.write(INIT_ADDR, INIT_KEY);    // записали ключ
//         // записали стандартное значение яркости, коррекции температуры и других параметров
//         // в данном случае это значение переменных, объявленных выше
//         EEPROM.put(0, font);
//         EEPROM.put(1, dim_level);
//         EEPROM.put(3, NIGHT);
//         EEPROM.put(5, DAY);
//         EEPROM.put(7, COR_TEMP);
//         EEPROM.put(9, chime_start);
//         EEPROM.put(11, chime_end);
//         EEPROM.put(13, chime_enable);
//         EEPROM.put(15, scroll_time);
//         EEPROM.put(16, symb_scroll);
//         EEPROM.put(17, shift_time);
//         EEPROM.put(18, screen_shift);
//     //    EEPROM.put(20, night_brightness);
//         EEPROM.put(22, font_v);
//       }
//   // Извлекаем значения параметров из EEPROM
//   EEPROM.get(0, font);
//   EEPROM.get(1, dim_level);
//   EEPROM.get(3, NIGHT);
//   EEPROM.get(5, DAY);
//   EEPROM.get(7, COR_TEMP);
//   EEPROM.get(9, chime_start);
//   EEPROM.get(11, chime_end);
//   EEPROM.get(13, chime_enable);
//   EEPROM.get(15, scroll_time);
//   EEPROM.get(16, symb_scroll);
//   EEPROM.get(17, shift_time);
//   EEPROM.get(18, screen_shift);
// //  EEPROM.get(20, night_brightness);
//   EEPROM.get(22, font_v);

    if (bme.begin(0x77))  
      {   //_____Определяем подключен ли датчик BME280
        bme_online = 1;  //___BME280 подключен
        // digitalWrite(PE6, HIGH); 
      } 
    else  
       bme_online = 0;   //___BME280 не подключен

  // aht20.begin();
  // delay (100);
  
  if (bme_online) 
    {
      get_temp_indoor();
      // get_hum();
      get_pres();
    } 
  else 
      get_temp_indoor();

  // считываем температуру
  if (DS.readTemp())  
      {   //_____Определяем подключен ли датчик DS18B20
        DS_online = 1;   //___DS18B20 подключен
        // digitalWrite(PE6, HIGH); 
      } 
  else   
        DS_online = 0;   //___DS18B20 не подключен

        beep();
        // digitalWrite(PE6, LOW); 
        // digitalWrite(PE6, HIGH); 
}

//---------------------------------------------------------------------------------------------------------
void loop() 
{
  if (millis() - scroll_timer >= scroll_time) 
    {
      scroll_timer += scroll_time;
      if (scroll_flag)  
        scroll_x++;
    }
  if (scroll_x == 8)  
    {
      scroll_x = 0;
      scroll_flag = 0;
      Update_DDRAM();
    }

  // Управление яркостью дисплея, считывание температуры с датчиков
  if (millis() - light_timer >= 1000) 
      { //_______Измеряем освещённость и температуру раз в секунду
        light_timer += 1000;
        if (DS_online)  
          // readDS18B20();           // считываем температуру

        // light = analogRead(photoresistor) >> 2; //___Датчик света подключен к 61 пину (ADC0). Используем старшие 8 бит АЦП
        if ((light <= NIGHT) && ((dim) == 1)) 
          dim = 0;

        if ((light >= DAY) && ((dim) == 0)) 
          dim = 1;
    
        if (cdt_run) 
          cdt_s -= 1;


           LED1 = !LED1;
             digitalWrite(PA0, LED1);    
      }

  if (millis() - bme_timer >= 5000) 
      { //_получаем температуру, влажность и давление раз в 5 секунд
        bme_timer += 5000;
        tflag = 0;  //______Автовозврат в режим часов
        shift_flag = !shift_flag;
        //    if ((bme_online) || (htu_online)) {
        if (bme_online) 
            {
              get_temp_indoor();
              // get_hum();
              get_pres();
            } 
        else 
              get_temp_indoor();
      }

  if (millis() - dots_timer >= 500) 
      {
        dots_timer += 500;
        dot_flag = !dot_flag;
      }

  if (dot_flag) 
      { //_____Мигаем точками.
        dots_up = 0xA8;
        dots_down = 0xA9;
        dots = ':';
        arrow_left = 0xAC;
        arrow_right = 0xAD;
        dot_l_vert = 0x14;
        dot_r_vert = 0x15;
        dots_vert = 0x16;
      } 
  else 
      {
        dots_up = ' ';
        dots_down = ' ';
        dots = ' ';
        arrow_left = ' ';
        arrow_right = ' ';
        dot_l_vert = ' ';
        dot_r_vert = ' ';
        dots_vert = ' ';
      }

  enc.tick();
  //  butt2.tick();
  // butt3.tick();
  // vbutt0.tick(keys.status(0));
  // vbutt1.tick(keys.status(1));
  // vbutt2.tick(keys.status(2));

  // if (digitalRead(5) == 0) 
  //   vert = 1;
  // else 
    vert = 0;

    // digitalWrite(PE6, HIGH);  
  // delay(100);                     
  // digitalWrite(PE6, LOW);    
  // delay(100);  

    if ((cdt_run) && (cdt_m == 0) && (cdt_s == 0)) 
        {
          cdt_run = !cdt_run;
          for (byte i = 0; i < 5; i++)  
              {
                beep();
                delay(150);
              }
        }  

  if (!vert) 
      {
        sandclock_timer = millis();

        if  (enc.held()) 
            {  //__Переключение режимов отображения/настройки времени
              if (!tflag) 
                  {
                    if (set_view == 0)  
                        {
                          menu_item = 1;
                          set_view = 1;
                          beep ();
                        } 
                    else if (set_view == 1)  
                            {
                              set_view = 0;
                              beep ();
                            } 
                    else if (set_view > 1)  
                            {
                              set_view = 1;
                              beep ();
                            }
                  }
            }

        switch (set_view) 
              {
                case 0:   //______Выводим время

                  if (DS_online)  
                      {
                        if (enc.click()) 
                            {
                              bme_timer = millis();
                              tflag = !tflag;
                              beep();
                            }
                      }
                  if (tflag)  
                    show_temp_digital();
                  else 
                    show_time();
                  
                  if (enc.rightH()) 
                      { //Переключение шрифтов
                        if (!tflag) 
                            {
                              if (font < 5) 
                                  {
                                    font++;
                                    EEPROM.update(0, font);
                                    beep ();
                                  }
                            }
                      }

                  if (enc.leftH()) 
                      { //Переключение шрифтов
                        if (!tflag) 
                            {
                              if (font > 0) 
                                  {
                                    font--;
                                    EEPROM.update(0, font);
                                    beep ();
                                  }
                            }
                      }

                  if (symb_scroll > 0) 
                    scroll = 1;
                  else  
                    scroll = 0;
                  
                  if (screen_shift > 0) 
                    shift = 1;
                  else 
                    shift = 0;
                  
                  break;

                case 1: //________________________________Выводим меню
                  scr = 0;
                  show_menu();

                  if (enc.right()) 
                      {
                        if (menu_item < 6) 
                            {
                              menu_item ++;
                              beep ();
                            }
                      }

                  if (enc.left()) 
                      {
                        if (menu_item > 1) 
                            {
                              menu_item --;
                              beep ();
                            }
                      }

                  if (enc.click()) 
                      {
                        set_param = 0;
                        set_view = (menu_item + 1);
                        beep();
                        if (menu_item == 6) 
                          set_view = 0;
                      }
                  break;

                case 2: //________________________________Настраиваем анимацию
                  //    scroll = 0;
                  if (enc.click()) 
                      {
                        set_param ++;
                        beep ();
                      }
                  if (set_param > 1) set_param = 0;
                  set_animation();
                  break;

                case 3: //________________________________Устанавливаем время
                  scroll = 0;
                  if (enc.click()) 
                      {
                        set_param ++;
                        beep ();
                      }
                  if (set_param > 5) set_param = 0;
                  set_time();
                  break;

                case 4: //________________________________Устанавливаем кукушку
                  scroll = 0;
                  if (enc.click()) 
                      {
                        set_param ++;
                        beep ();
                      }
                  if (set_param > 2) set_param = 0;
                  set_chime();
                  break;

                case 5: //________________________________Устанавливаем переключение яркости
                  scroll = 0;
                  if (enc.click()) 
                      {
                        set_param ++;
                        beep ();
                      }
                  if (set_param > 2) set_param = 0;
                  set_bright();
                  break;

                case 6: //________Коррекция температуры
                  scroll = 0;
                  set_temp_corr();
                  break;
              }//switch
      }// if(!vert)
   else  
        {
          if (millis() - sandclock_timer >= 200) 
              {
                sandclock_timer += 200;
                sand += 1;
              }
          if (sand > 27) sand = 0;

          if (enc.held() && (!cdt_run) && (!tflag))  
              {  //__Переключение режимов отображения/настройки времени
                set_view_v = !set_view_v;
                cdt_m = 0;
                cdt_s = 0;
                beep();
              }
              
          switch (set_view_v) 
                {
                  case 0:   //______Выводим время
                    if (DS_online)  
                        {
                          if (enc.click()) 
                              {
                                bme_timer = millis();
                                tflag = !tflag;
                                beep();
                              }
                        }
                    if (tflag)  
                      show_temp_scale();
                    else 
                      show_time_sandclock();
                    
                    if (enc.rightH()) 
                        { //Переключение шрифтов
                          if (!tflag) 
                              {
                                if (font_v < 1) 
                                    {
                                      font_v++;
                                      EEPROM.update(22, font_v);
                                      beep ();
                                    }
                              }
                        }

                    if (enc.leftH()) 
                        { //Переключение шрифтов
                          if (!tflag) 
                              {
                                if (font_v > 0) 
                                    {
                                      font_v--;
                                      EEPROM.update(22, font_v);
                                      beep ();
                                    }
                              }
                        }
                    if (symb_scroll > 0) 
                         scroll = 1;
                    else  
                      scroll = 0;
                    
                    break;
                  case 1:   //______таймер
                    scr = 0;
                    if (enc.click())  
                        {
                          if ((cdt_m > 0) || (cdt_s > 0)) 
                              {
                                cdt_run = !cdt_run;
                                beep();
                              }
                        }
                    if (!cdt_run)  
                        {
                          sandclock_timer = millis();
                          sand = 22;
                        }

                    if ((symb_scroll > 0) && (cdt_run)) 
                         scroll = 1;
                    else  
                      scroll = 0;
                    
                    countdown_timer();
                    break;
                }//switch
        }//else
}
//---------------------------------------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect) //обработчик прерывания по совпадению А таймера 1, счетчик сбрасывается в 0
{      
  output_DDRAM();
 
}
//---------------------------------------------------------------------------------------------------------
void show_time() //___Отображение времени
{ 
  byte h1, h2, m1, m2, s1, s2, d1, d2, mon, wd, d01, y1, y2, y3, y4;

  DateTime now = rtc.getTime();
  h1 = now.hour / 10;
  h2 = now.hour % 10;
  m1 = now.minute / 10;
  m2 = now.minute % 10;
  s1 = now.second / 10;
  s2 = now.second % 10;
  d1 = now.date / 10 + 0x30;
  d2 = now.date % 10 + 0x30;
  mon = now.month - 1;
  wd = now.day - 1;
  y1 = now.year / 1000 + 0x30;
  y2 = now.year / 100 % 10 + 0x30;
  y3 = now.year / 10 % 10 + 0x30;
  y4 = now.year % 10 % 10 + 0x30;

  if ((now.hour >= chime_start) && (now.hour <= chime_end) && chime_enable) 
      {
        if ((now.minute == 00) && (now.second == 00) && beep_flag)  
            {
              beep();
              beep_flag = 0;
            }
        if ((now.minute == 00) && (now.second == 01))          
            beep_flag = 1;
      }

  if (now.date < 10)  // Гашение старшего ноля числа
    {  
      d01 = ' ';
    } 
  else      
        d01 = d1;
    

  if (font < 4) 
        {

          if (millis() - shift_timer >= shift_time * 10) 
              {
                shift_timer += shift_time * 10;

                if (shift)  
                      {
                        if (((m2 % 2) != 0) && (now.second > 0)) //___________Если изменение порядка отображения на экране активно
                            { 
                              scr--;
                              scr = constrain(scr, 0, 7);
                            } 
                        else if (((m2 % 2) == 0) && (now.second > 0)) 
                                {
                                  scr++;
                                  scr = constrain(scr, 0, 7);
                                }

                        if ((rtc.getSeconds() > 0) && (rtc.getSeconds() < 31)) 
                            {
                              DDRAM[0][21] = t_sign_indoor;
                              DDRAM[0][22] = temp_indoor_c1;
                              DDRAM[0][23] = temp_indoor_c2;
                              DDRAM[0][24] =  '.';
                              DDRAM[0][25] = temp_indoor_d;
                              DDRAM[0][26] = 0xAB;

                              if ((bme_online) || (htu_online)) 
                                  {
                                    DDRAM[1][21] = 0xAF;
                                    DDRAM[1][22] = hum_round_c1;
                                    DDRAM[1][23] = hum_round_c2;
                                    DDRAM[1][24] = '.';
                                    DDRAM[1][25] = hum_round_d;
                                    DDRAM[1][26] = '%';
                                  } 
                              else 
                                  {
                                    if (DS_online)  
                                        {
                                          DDRAM[1][21] = temp_DS_10;
                                          DDRAM[1][22] = temp_DS_01;
                                          DDRAM[1][23] = temp_DS_c2 + 0x30;
                                          DDRAM[1][24] = '.';
                                          DDRAM[1][25] = temp_DS_d + 0x30;
                                          DDRAM[1][26] = 0xAB;
                                        } 
                                    else 
                                        {
                                          DDRAM[1][21] = ' ';
                                          DDRAM[1][22] = 0xDD;
                                          DDRAM[1][23] = 0xDE;
                                          DDRAM[1][24] = 0xDE;
                                          DDRAM[1][25] = 0xDE;
                                          DDRAM[1][26] = 0xDF;
                                        }
                                  }
                            } 
                        else 
                            {
                              if (bme_online) 
                                  {
                                    if (DS_online)  
                                        {
                                          DDRAM[0][21] = temp_DS_10;
                                          DDRAM[0][22] = temp_DS_01;
                                          DDRAM[0][23] = temp_DS_c2 + 0x30;
                                          DDRAM[0][24] =  '.';
                                          DDRAM[0][25] = temp_DS_d + 0x30;
                                          DDRAM[0][26] = 0xAB;
                                        } 
                                    else 
                                        {
                                          DDRAM[0][21] = t_sign_indoor;
                                          DDRAM[0][22] = temp_indoor_c1;
                                          DDRAM[0][23] = temp_indoor_c2;
                                          DDRAM[0][24] =  '.';
                                          DDRAM[0][25] = temp_indoor_d;
                                          DDRAM[0][26] = 0xAB;
                                        }
                                    DDRAM[1][21] = 0xD8;
                                    DDRAM[1][22] = pres_round_c1;
                                    DDRAM[1][23] = pres_round_c2;
                                    DDRAM[1][24] = pres_round_c3;
                                    DDRAM[1][25] = 'H';
                                    DDRAM[1][26] = 'g';
                                  } 
                              else  
                                  {
                                    DDRAM[0][21] = t_sign_indoor;
                                    DDRAM[0][22] = temp_indoor_c1;
                                    DDRAM[0][23] = temp_indoor_c2;
                                    DDRAM[0][24] =  '.';
                                    DDRAM[0][25] = temp_indoor_d;
                                    DDRAM[0][26] = 0xAB;
                                    if (DS_online)  
                                        {
                                          DDRAM[1][21] = temp_DS_10;
                                          DDRAM[1][22] = temp_DS_01;
                                          DDRAM[1][23] = temp_DS_c2 + 0x30;
                                          DDRAM[1][24] = '.';
                                          DDRAM[1][25] = temp_DS_d + 0x30;
                                          DDRAM[1][26] = 0xAB;
                                        } 
                                    else  
                                        {
                                          DDRAM[1][21] = ' ';
                                          DDRAM[1][22] = 0xDD;
                                          DDRAM[1][23] = 0xDE;
                                          DDRAM[1][24] = 0xDE;
                                          DDRAM[1][25] = 0xDE;
                                          DDRAM[1][26] = 0xDF;
                                        }
                                  }
                            }
                      }  
                else 
                    { //___________Если изменение порядка отображения на экране не активно

                      if (((m2 % 2) != 0) && (now.second > 0)) 
                          {
                            scr = 7;

                            DDRAM[0][21] = ' ';
                            DDRAM[0][22] = ' ';
                            DDRAM[0][23] = pgm_read_byte(&ddd[wd][0]);
                            DDRAM[0][24] = pgm_read_byte(&ddd[wd][1]);
                            DDRAM[0][25] = pgm_read_byte(&ddd[wd][2]);
                            DDRAM[0][26] = ' ';

                            DDRAM[1][21] = d01;
                            DDRAM[1][22] = d2;
                            DDRAM[1][23] = ' ';
                            DDRAM[1][24] = pgm_read_byte(&mmm[mon][0]);
                            DDRAM[1][25] = pgm_read_byte(&mmm[mon][1]);
                            DDRAM[1][26] = pgm_read_byte(&mmm[mon][2]);

                          }  
                      else if (((m2 % 2) == 0) && (now.second > 0)) 
                          {
                            scr = 7;

                            if ((rtc.getSeconds() > 0) && (rtc.getSeconds() < 31)) 
                                {

                                  DDRAM[0][21] = t_sign_indoor;
                                  DDRAM[0][22] = temp_indoor_c1;
                                  DDRAM[0][23] = temp_indoor_c2;
                                  DDRAM[0][24] =  '.';
                                  DDRAM[0][25] = temp_indoor_d;
                                  DDRAM[0][26] = 0xAB;

                                  if ((bme_online) || (htu_online)) 
                                      {
                                        DDRAM[1][21] = 0xAF;
                                        DDRAM[1][22] = hum_round_c1;
                                        DDRAM[1][23] = hum_round_c2;
                                        DDRAM[1][24] = '.';
                                        DDRAM[1][25] = hum_round_d;
                                        DDRAM[1][26] = '%';
                                      } 
                                  else 
                                      {
                                        if (DS_online)  
                                            {
                                              DDRAM[1][21] = temp_DS_10;
                                              DDRAM[1][22] = temp_DS_01;
                                              DDRAM[1][23] = temp_DS_c2 + 0x30;
                                              DDRAM[1][24] = '.';
                                              DDRAM[1][25] = temp_DS_d + 0x30;
                                              DDRAM[1][26] = 0xAB;
                                            } 
                                        else 
                                            {
                                              DDRAM[1][21] = ' ';
                                              DDRAM[1][22] = 0xDD;
                                              DDRAM[1][23] = 0xDE;
                                              DDRAM[1][24] = 0xDE;
                                              DDRAM[1][25] = 0xDE;
                                              DDRAM[1][26] = 0xDF;
                                            }
                                      }
                                } 
                            else 
                                {
                                  if (bme_online) 
                                      {
                                        if (DS_online)  
                                            {
                                              DDRAM[0][21] = temp_DS_10;
                                              DDRAM[0][22] = temp_DS_01;
                                              DDRAM[0][23] = temp_DS_c2 + 0x30;
                                              DDRAM[0][24] =  '.';
                                              DDRAM[0][25] = temp_DS_d + 0x30;
                                              DDRAM[0][26] = 0xAB;
                                            } 
                                        else 
                                            {
                                              DDRAM[0][21] = t_sign_indoor;
                                              DDRAM[0][22] = temp_indoor_c1;
                                              DDRAM[0][23] = temp_indoor_c2;
                                              DDRAM[0][24] =  '.';
                                              DDRAM[0][25] = temp_indoor_d;
                                              DDRAM[0][26] = 0xAB;
                                            }
                                        DDRAM[1][21] = 0xD8;
                                        DDRAM[1][22] = pres_round_c1;
                                        DDRAM[1][23] = pres_round_c2;
                                        DDRAM[1][24] = pres_round_c3;
                                        DDRAM[1][25] = 'H';
                                        DDRAM[1][26] = 'g';
                                      } 
                                  else  
                                      {
                                        DDRAM[0][21] = t_sign_indoor;
                                        DDRAM[0][22] = temp_indoor_c1;
                                        DDRAM[0][23] = temp_indoor_c2;
                                        DDRAM[0][24] =  '.';
                                        DDRAM[0][25] = temp_indoor_d;
                                        DDRAM[0][26] = 0xAB;
                                        if (DS_online)  
                                            {
                                              DDRAM[1][21] = temp_DS_10;
                                              DDRAM[1][22] = temp_DS_01;
                                              DDRAM[1][23] = temp_DS_c2 + 0x30;
                                              DDRAM[1][24] = '.';
                                              DDRAM[1][25] = temp_DS_d + 0x30;
                                              DDRAM[1][26] = 0xAB;
                                            } 
                                        else  
                                            {
                                              DDRAM[1][21] = ' ';
                                              DDRAM[1][22] = 0xDD;
                                              DDRAM[1][23] = 0xDE;
                                              DDRAM[1][24] = 0xDE;
                                              DDRAM[1][25] = 0xDE;
                                              DDRAM[1][26] = 0xDF;
                                            }
                                      }
                                }
                          }
                    }
              }

          DDRAM[0][0] = ' ';
          DDRAM[0][1] = ' ';
          DDRAM[0][2] = pgm_read_byte(&ddd[wd][0]);
          DDRAM[0][3] = pgm_read_byte(&ddd[wd][1]);
          DDRAM[0][4] = pgm_read_byte(&ddd[wd][2]);
          DDRAM[0][5] = ' ';
          DDRAM[0][6] = ' ';

          DDRAM[1][0] = d01;
          DDRAM[1][1] = d2;
          DDRAM[1][2] = ' ';
          DDRAM[1][3] = pgm_read_byte(&mmm[mon][0]);
          DDRAM[1][4] = pgm_read_byte(&mmm[mon][1]);
          DDRAM[1][5] = pgm_read_byte(&mmm[mon][2]);
          DDRAM[1][6] = ' ';

          if (font < 2) 
              {
                byte h10, h11, h12, h13; // Гашение старшего ноля часов
                if (rtc.getHours() < 10) 
                    {
                      h10 = ' ';
                      h11 = ' ';
                      h12 = ' ';
                      h13 = ' ';
                    } 
                else 
                    {
                      h10 = h1 * 4 + 0x80;
                      h11 = h1 * 4 + 0x81;
                      h12 = h1 * 4 + 0x82;
                      h13 = h1 * 4 + 0x83;
                    }

                if (chime_enable)         
                  DDRAM[0][18] = 0xDA;  //"Bell"
                else          
                  DDRAM[0][18] = ' ';
              

                DDRAM[0][7] = ' ';
                DDRAM[0][8] = h10;
                DDRAM[0][9] = h11;
                DDRAM[0][10] = h2 * 4 + 0x80;
                DDRAM[0][11] = h2 * 4 + 0x81;
                DDRAM[0][12] = dots_up;
                DDRAM[0][13] = m1 * 4 + 0x80;
                DDRAM[0][14] = m1 * 4 + 0x81;
                DDRAM[0][15] = m2 * 4 + 0x80;
                DDRAM[0][16] = m2 * 4 + 0x81;
                DDRAM[0][17] = ' ';
                // DDRAM[0][18] = ' ';
                DDRAM[0][19] = ' ';
                DDRAM[0][20] = ' ';

                DDRAM[1][7] = ' ';
                DDRAM[1][8] = h12;
                DDRAM[1][9] = h13;
                DDRAM[1][10] = h2 * 4 + 0x82;
                DDRAM[1][11] = h2 * 4 + 0x83;
                DDRAM[1][12] = dots_down;
                DDRAM[1][13] = m1 * 4 + 0x82;
                DDRAM[1][14] = m1 * 4 + 0x83;
                DDRAM[1][15] = m2 * 4 + 0x82;
                DDRAM[1][16] = m2 * 4 + 0x83;
                DDRAM[1][17] = dots;
                DDRAM[1][18] = s1 + 0x30;
                DDRAM[1][19] = s2 + 0x30;
                DDRAM[1][20] = ' ';

                DDRAM[2][12] = dots_up;
                DDRAM[3][12] = dots_down;
                DDRAM[3][17] = dots;

              } 
          else if ((font == 2) || (font == 3))  
                  {
                    byte h10, h11, h12, h13, h14, h15; // Гашение старшего ноля часов
                    if (rtc.getHours() < 10) 
                        {
                          h10 = ' ';
                          h11 = ' ';
                          h12 = ' ';
                          h13 = ' ';
                          h14 = ' ';
                          h15 = ' ';
                        } 
                    else 
                        {
                          h10 = pgm_read_byte(&BigNum[h1][0]);
                          h11 = pgm_read_byte(&BigNum[h1][1]);
                          h12 = pgm_read_byte(&BigNum[h1][2]);
                          h13 = pgm_read_byte(&BigNum[h1][3]);
                          h14 = pgm_read_byte(&BigNum[h1][4]);
                          h15 = pgm_read_byte(&BigNum[h1][5]);
                        }
                    DDRAM[0][7] = h10;
                    DDRAM[0][8] = h11;
                    DDRAM[0][9] = h12;
                    DDRAM[0][10] = pgm_read_byte(&BigNum[h2][0]);
                    DDRAM[0][11] = pgm_read_byte(&BigNum[h2][1]);
                    DDRAM[0][12] = pgm_read_byte(&BigNum[h2][2]);
                    DDRAM[0][13] = dots_up;
                    DDRAM[0][14] = pgm_read_byte(&BigNum[m1][0]);
                    DDRAM[0][15] = pgm_read_byte(&BigNum[m1][1]);
                    DDRAM[0][16] = pgm_read_byte(&BigNum[m1][2]);
                    DDRAM[0][17] = pgm_read_byte(&BigNum[m2][0]);
                    DDRAM[0][18] = pgm_read_byte(&BigNum[m2][1]);
                    DDRAM[0][19] = pgm_read_byte(&BigNum[m2][2]);
                    DDRAM[0][20] = ' ';

                    DDRAM[1][7] = h13;
                    DDRAM[1][8] = h14;
                    DDRAM[1][9] = h15;
                    DDRAM[1][10] = pgm_read_byte(&BigNum[h2][3]);
                    DDRAM[1][11] = pgm_read_byte(&BigNum[h2][4]);
                    DDRAM[1][12] = pgm_read_byte(&BigNum[h2][5]);
                    DDRAM[1][13] = dots_down;
                    DDRAM[1][14] = pgm_read_byte(&BigNum[m1][3]);
                    DDRAM[1][15] = pgm_read_byte(&BigNum[m1][4]);
                    DDRAM[1][16] = pgm_read_byte(&BigNum[m1][5]);
                    DDRAM[1][17] = pgm_read_byte(&BigNum[m2][3]);
                    DDRAM[1][18] = pgm_read_byte(&BigNum[m2][4]);
                    DDRAM[1][19] = pgm_read_byte(&BigNum[m2][5]);
                    DDRAM[1][20] = ' ';

                    DDRAM[2][13] = dots_up;
                    DDRAM[3][13] = dots_down;
                  }
        } 
  if (font > 3)  
        {
          scr = 0;

          byte h10; // Гашение старшего ноля часов
          if (rtc.getHours() < 10)         
            h10 = ' ';
          else          
            h10 = h1 + 0x30;
         

          DDRAM[0][0] = h10;
          DDRAM[0][1] = h2 + 0x30;
          DDRAM[0][2] = dots;
          DDRAM[0][3] = m1 + 0x30;
          DDRAM[0][4] = m2 + 0x30;
          DDRAM[0][5] = dots;
          DDRAM[0][6] = s1 + 0x30;
          DDRAM[0][7] = s2 + 0x30;
          DDRAM[0][8] = ' ';
          DDRAM[0][9] = pgm_read_byte(&ddd_full[wd][0]);
          DDRAM[0][10] = pgm_read_byte(&ddd_full[wd][1]);
          DDRAM[0][11] = pgm_read_byte(&ddd_full[wd][2]);
          DDRAM[0][12] = pgm_read_byte(&ddd_full[wd][3]);
          DDRAM[0][13] = pgm_read_byte(&ddd_full[wd][4]);
          DDRAM[0][14] = pgm_read_byte(&ddd_full[wd][5]);
          DDRAM[0][15] = pgm_read_byte(&ddd_full[wd][6]);
          DDRAM[0][16] = pgm_read_byte(&ddd_full[wd][7]);
          DDRAM[0][17] = pgm_read_byte(&ddd_full[wd][8]);
          DDRAM[0][18] = pgm_read_byte(&ddd_full[wd][9]);
          DDRAM[0][19] = pgm_read_byte(&ddd_full[wd][10]);

          DDRAM[2][2] = dots;
          DDRAM[2][5] = dots;

          if (((m2 % 2) != 0) && (now.second > 0)) 
              {
                DDRAM[1][0] = d01;
                DDRAM[1][1] = d2;
                DDRAM[1][2] = ' ';
                DDRAM[1][3] = pgm_read_byte(&mmm_full[mon][0]);
                DDRAM[1][4] = pgm_read_byte(&mmm_full[mon][1]);
                DDRAM[1][5] = pgm_read_byte(&mmm_full[mon][2]);
                DDRAM[1][6] = pgm_read_byte(&mmm_full[mon][3]);
                DDRAM[1][7] = pgm_read_byte(&mmm_full[mon][4]);
                DDRAM[1][8] = pgm_read_byte(&mmm_full[mon][5]);
                DDRAM[1][9] = pgm_read_byte(&mmm_full[mon][6]);
                DDRAM[1][10] = pgm_read_byte(&mmm_full[mon][7]);
                DDRAM[1][11] = ' ';
                DDRAM[1][12] = y1;
                DDRAM[1][13] = y2;
                DDRAM[1][14] = y3;
                DDRAM[1][15] = y4;
                DDRAM[1][16] = ' ';
                DDRAM[1][17] = 0x03;
                DDRAM[1][18] = 0x0E;
                DDRAM[1][19] = 0x04;

              } 
          else if (((m2 % 2) == 0) && (now.second > 0)) 
                  {
                    if ((bme_online) || (htu_online)) 
                        {
                          if ((rtc.getSeconds() > 0) && (rtc.getSeconds() < 31)) 
                              {
                                DDRAM[1][0] = t_sign_indoor;
                                DDRAM[1][1] = temp_indoor_c1;
                                DDRAM[1][2] = temp_indoor_c2;
                                DDRAM[1][3] = '.';
                                DDRAM[1][4] = temp_indoor_d;
                                DDRAM[1][5] = 0xAB;
                              } 
                          else 
                              {
                                if (DS_online)  
                                    {
                                      DDRAM[1][0] = temp_DS_10;
                                      DDRAM[1][1] = temp_DS_01;
                                      DDRAM[1][2] = temp_DS_c2 + 0x30;
                                      DDRAM[1][3] = '.';
                                      DDRAM[1][4] = temp_DS_d + 0x30;
                                      DDRAM[1][5] = 0xAB;
                                    } 
                                else 
                                    {
                                      DDRAM[1][0] = t_sign_indoor;
                                      DDRAM[1][1] = temp_indoor_c1;
                                      DDRAM[1][2] = temp_indoor_c2;
                                      DDRAM[1][3] = '.';
                                      DDRAM[1][4] = temp_indoor_d;
                                      DDRAM[1][5] = 0xAB;
                                    }
                              }
                          DDRAM[1][6] = ' ';
                          DDRAM[1][7] = 0xAF;
                          DDRAM[1][8] = hum_round_c1;
                          DDRAM[1][9] = hum_round_c2;
                          DDRAM[1][10] = '.';
                          DDRAM[1][11] = hum_round_d;
                          DDRAM[1][12] = '%';
                          DDRAM[1][13] = ' ';
                          DDRAM[1][14] = 0xD8;
                          DDRAM[1][15] = pres_round_c1;
                          DDRAM[1][16] = pres_round_c2;
                          DDRAM[1][17] = pres_round_c3;
                          DDRAM[1][18] = 'H';
                          DDRAM[1][19] = 'g';
                        } 
                    else 
                        {
                          DDRAM[1][3] = t_sign_indoor;
                          DDRAM[1][4] = temp_indoor_c1;
                          DDRAM[1][5] = temp_indoor_c2;
                          DDRAM[1][6] = '.';
                          DDRAM[1][7] = temp_indoor_d;
                          DDRAM[1][8] = 0xAB;
                          DDRAM[1][9] = ' ';
                          DDRAM[1][10] = ' ';
                          if (DS_online)  
                              {
                                DDRAM[1][11] = temp_DS_10;
                                DDRAM[1][12] = temp_DS_01;
                                DDRAM[1][13] = temp_DS_c2 + 0x30;
                                DDRAM[1][14] = '.';
                                DDRAM[1][15] = temp_DS_d + 0x30;
                                DDRAM[1][16] = 0xAB;
                              } 
                          else 
                              {
                                DDRAM[1][11] = ' ';
                                DDRAM[1][12] = ' ';
                                DDRAM[1][13] = ' ';
                                DDRAM[1][14] = ' ';
                                DDRAM[1][15] = ' ';
                                DDRAM[1][16] = ' ';
                              }
                          DDRAM[1][0] = ' ';
                          DDRAM[1][1] = ' ';
                          DDRAM[1][2] = ' ';
                          DDRAM[1][17] = ' ';
                          DDRAM[1][18] = ' ';
                          DDRAM[1][19] = ' ';
                        }
                  }
        }
}
//---------------------------------------------------------------------------------------------------------
void set_animation()  
{
  byte h1, h2, m1, m2, s1, s2;
  DateTime now = rtc.getTime();
  h1 = now.hour / 10 + 0x30;
  h2 = now.hour % 10 + 0x30;
  m1 = now.minute / 10 + 0x30;
  m2 = now.minute % 10 + 0x30;
  s1 = now.second / 10 + 0x30;
  s2 = now.second % 10 + 0x30;

  const byte anim[2][6] = 
                          {
                            {0, 125, 100, 75, 50, 25}, //____Скорость скролла символов
                            {0, 50, 40, 30, 20, 10} // ______Скорость сдвига экрана
                          };

  if (set_param == 0) 
        {
          if (symb_scroll > 0)          
            scroll = 1;
          else          
            scroll = 0;
       
          shift = 0;
          scr = 4;

          DDRAM[0][7 + scr] = arrow_left;
          DDRAM[0][9 + scr] = arrow_right;
          DDRAM[2][7 + scr] = arrow_left;
          DDRAM[2][9 + scr] = arrow_right;
          DDRAM[0][17 + scr] = ' ';
          DDRAM[0][19 + scr] = ' ';

          if (enc.right())   
              {
                if (symb_scroll < 5)  
                    {
                      symb_scroll += 1;
                      scroll_time = anim[0][symb_scroll];
                      EEPROM.update(15, scroll_time);
                      EEPROM.update(16, symb_scroll);
                      beep ();
                    }
              }
          if (enc.left())  
              {
                if (symb_scroll > 0)  
                    {
                      symb_scroll -= 1;
                      scroll_time = anim[0][symb_scroll];
                      EEPROM.update(15, scroll_time);
                      EEPROM.update(16, symb_scroll);
                      beep ();
                    }
              }
        }
  if (set_param == 1) 
        {
          if (screen_shift > 0)   
            shift = 1;
          else     
            shift = 0;
        
          scroll = 0;

          DDRAM[0][7 + scr] = ' ';
          DDRAM[0][9 + scr] = ' ';
          DDRAM[0][17 + scr] = arrow_left;
          DDRAM[0][19 + scr] = arrow_right;
          DDRAM[2][17 + scr] = arrow_left;
          DDRAM[2][19 + scr] = arrow_right;

          if (shift)  
              {
                if (millis() - shift_timer >= shift_time * 10) 
                    {
                      shift_timer += shift_time * 10;

                      if (shift_flag) 
                          {
                            scr ++;
                            scr = constrain(scr, 0, 7);
                          } 
                      else 
                          {
                            scr --;
                            scr = constrain(scr, 0, 7);
                          }
                    }
              } 
          else        
                scr = 4;
          
          if (enc.right())   
              {
                if (screen_shift < 5)  
                    {
                      screen_shift += 1;
                      shift_time = anim[1][screen_shift];
                      EEPROM.update(17, shift_time);
                      EEPROM.update(18, screen_shift);
                      beep ();
                    }
              }
          if (enc.left())  
              {
                if (screen_shift > 0)  
                    {
                      screen_shift -= 1;
                      shift_time = anim[1][screen_shift];
                      EEPROM.update(17, shift_time);
                      EEPROM.update(18, screen_shift);
                      beep ();
                    }
              }
        }

  DDRAM[0][0 + scr] = 0x11;
  DDRAM[0][1 + scr] = 0x08;
  DDRAM[0][2 + scr] = 0x0C;
  DDRAM[0][3 + scr] = 0x02;
  DDRAM[0][4 + scr] = 0x0E;
  DDRAM[0][5 + scr] = 0x0B;
  DDRAM[0][6 + scr] = ' ';
  //  DDRAM[0][7 + scr] =
  DDRAM[0][8 + scr] = symb_scroll + 0x30;
  //  DDRAM[0][9 + scr] =
  DDRAM[0][10 + scr] = ' ';
  DDRAM[0][11 + scr] = 0x1D;
  DDRAM[0][12 + scr] = 0x0A;
  DDRAM[0][13 + scr] = 0x10;
  DDRAM[0][14 + scr] = 0x00;
  DDRAM[0][15 + scr] = 0x0D;
  DDRAM[0][16 + scr] = ' ';
  //  DDRAM[0][17 + scr] =
  DDRAM[0][18 + scr] = screen_shift + 0x30;
  //  DDRAM[0][19 + scr] =

  DDRAM[1][0] = ' ';
  DDRAM[1][1] = ' ';
  DDRAM[1][2] = ' ';
  DDRAM[1][3] = ' ';
  DDRAM[1][4] = ' ';
  DDRAM[1][5] = ' ';
  DDRAM[1][6] = ' ';
  DDRAM[1][7] = ' ';
  DDRAM[1][8] = ' ';
  DDRAM[1][9] = ' ';
  DDRAM[1][10] = h1;
  DDRAM[1][11] = h2;
  DDRAM[1][12] = ':';
  DDRAM[1][13] = m1;
  DDRAM[1][14] = m2;
  DDRAM[1][15] = ':';
  DDRAM[1][16] = s1;
  DDRAM[1][17] = s2;
  DDRAM[1][18] = ' ';
  DDRAM[1][19] = ' ';
  DDRAM[1][20] = ' ';
  DDRAM[1][21] = ' ';
  DDRAM[1][22] = ' ';
  DDRAM[1][23] = ' ';
  DDRAM[1][24] = ' ';
  DDRAM[1][25] = ' ';
  DDRAM[1][26] = ' ';
}
//---------------------------------------------------------------------------------------------------------
void set_time() //__Экран установки времени
{ 
  scr = 0;
  byte h1, h2, m1, m2, s1, s2, d1, d2, mon, y1, y2, y3, y4, i;
  DateTime now = rtc.getTime();
  h1 = now.hour / 10 + 0x30;
  h2 = now.hour % 10 + 0x30;
  m1 = now.minute / 10 + 0x30;
  m2 = now.minute % 10 + 0x30;
  s1 = now.second / 10 + 0x30;
  s2 = now.second % 10 + 0x30;
  d1 = now.date / 10 + 0x30;
  d2 = now.date % 10 + 0x30;
  mon = now.month - 1;
  y1 = now.year / 1000 + 0x30;
  y2 = now.year / 100 % 10 + 0x30;
  y3 = now.year / 10 % 10 + 0x30;
  y4 = now.year % 10 % 10 + 0x30;

  cursor_x = 0;
  for (i = 0; i < 20; i++)  
      {
        data = pgm_read_byte(&setscreen[set_param][i]);
        Record(cursor_x, data);
      }

  //__________________________________________Часы
  if (set_param == 0) 
      {
        if (enc.right())  
            {
              //beep ();
              now.hour += 1;
              rtc.setTime(now);
            }

        if (enc.left())  
            {
              //  beep ();
              now.hour -= 1;
              rtc.setTime(now);
            }
      }
  //__________________________________________Минуты
  if (set_param == 1) 
      {
        if (enc.right())  
            {
              //  beep ();
              now.minute += 1;
              rtc.setTime(now);
            }
        if (enc.left())  
            {
              // beep ();
              now.minute -= 1;
              rtc.setTime(now);
            }
      }
  //__________________________________________Секунды
  if (set_param == 2) 
      {
        if (enc.right())  
            {
              beep ();
              now.minute += 1;
              now.second = 0;
              rtc.setTime(now);
            }
        if (enc.left())  
            {
              beep ();
              now.second = 0;
              rtc.setTime(now);
            }
      }
  //__________________________________________Число
  if (set_param == 3) 
      {
        if (enc.right())  
            {
              // beep ();
              now.date += 1;
              rtc.setTime(now);
            }
        if (enc.left())  
            {
              // beep ();
              now.date -= 1;
              rtc.setTime(now);
            }
      }
  //__________________________________________Месяц
  if (set_param == 4) 
      {
        if (enc.right())  
            {
              //  beep ();
              now.month += 1;
              rtc.setTime(now);
            }
        if (enc.left())  
            {
              // beep ();
              now.month -= 1;
              rtc.setTime(now);
            }
      }
  //__________________________________________Год
  if (set_param == 5) 
      {
        if (enc.right())  
            {
              //beep ();
              now.year += 1;
              rtc.setTime(now);
            }
        if (enc.left())  
            {
              //  beep ();
              now.year -= 1;
              rtc.setTime(now);
            }
      }

  DDRAM[1][0] = h1;
  DDRAM[1][1] = h2;
  DDRAM[1][2] = dots;
  DDRAM[1][3] = m1;
  DDRAM[1][4] = m2;
  DDRAM[1][5] = dots;
  DDRAM[1][6] = s1;
  DDRAM[1][7] = s2;
  DDRAM[1][8] = ' ';
  DDRAM[1][9] = d1;
  DDRAM[1][10] = d2;
  DDRAM[1][11] = ' ';
  DDRAM[1][12] = pgm_read_byte(&mmm[mon][0]);
  DDRAM[1][13] = pgm_read_byte(&mmm[mon][1]);
  DDRAM[1][14] = pgm_read_byte(&mmm[mon][2]);
  DDRAM[1][15] = ' ';
  DDRAM[1][16] = y1;
  DDRAM[1][17] = y2;
  DDRAM[1][18] = y3;
  DDRAM[1][19] = y4;
}
//---------------------------------------------------------------------------------------------------------
void set_chime() 
{
  scr = 0;
  byte cs1, cs2, ce1, ce2, i;
  cs1 = chime_start / 10 + 0x30;
  cs2 = chime_start % 10 + 0x30;
  ce1 = chime_end / 10 + 0x30;
  ce2 = chime_end % 10 + 0x30;

  cursor_x = 0;
  for (i = 0; i < 20; i++)  
      {
        data = pgm_read_byte(&menu_screen[7][i]);
        Record(cursor_x, data);
      }

  //__________________________________________Вкл/Выкл кукушки
  if (set_param == 0) 
      {
        DDRAM[1][0] = arrow_left;
        DDRAM[1][7] = ' ';
        DDRAM[1][14] = ' ';

        if (enc.right()) 
            {
              if (chime_enable == 0) 
                  {
                    chime_enable = 1;
                    EEPROM.update(13, chime_enable);
                    beep ();
                  }
            }
        if (enc.left())  
            {
              if (chime_enable == 1) 
                  {
                    chime_enable = 0;
                    EEPROM.update(13, chime_enable);
                    beep ();
                  }
            }
      }
  //__________________________________________В каком часу подавать первый сигнал
  if (set_param == 1) 
      {
        DDRAM[1][0] = ' ';
        DDRAM[1][7] = arrow_left;
        DDRAM[1][14] = ' ';

        if (enc.right())  
            {
              if (chime_start < 22) 
                  {
                    chime_start += 1;
                    chime_start = constrain(chime_start, 0, 22);
                    EEPROM.update(9, chime_start);
                    //   beep ();
                  }
            }

        if (enc.left())  
            {
              if (chime_start > 0) 
                  {
                    chime_start -= 1;
                    chime_start = constrain(chime_start, 0, 22);
                    EEPROM.update(9, chime_start);
                    //    beep ();
                  }
            }
      }
  //__________________________________________В каком часу подавать последний сигнал
  if (set_param == 2) 
      {
        DDRAM[1][0] = ' ';
        DDRAM[1][7] = ' ';
        DDRAM[1][14] = arrow_left;

        if (enc.right())  
            {
              if (chime_start < 23) 
              {
                chime_end += 1;
                chime_end = constrain(chime_end, 1, 23);
                EEPROM.update(11, chime_end);
                //   beep ();
              }
            }
        if (enc.left())  
            {
              if (chime_start > 1) 
              {
                chime_end -= 1;
                chime_end = constrain(chime_end, 1, 23);
                EEPROM.update(11, chime_end);
                //    beep ();
              }
            }
      }

  if (chime_enable) 
      {
        //  DDRAM[1][0] = ' ';
        DDRAM[1][1] = ' ';
        DDRAM[1][2] = 0x02;
        DDRAM[1][3] = 0x0A;
        DDRAM[1][4] = 0x0B;
        DDRAM[1][5] = ' ';
      } 
  else 
      {
        DDRAM[1][1] = ' ';
        DDRAM[1][2] = 0x02;
        DDRAM[1][3] = 0x1B;
        DDRAM[1][4] = 0x0A;
        DDRAM[1][5] = 0x0B;
      }

  DDRAM[1][6] = ' ';
  //  DDRAM[1][7] = ' ';
  DDRAM[1][8] = cs1;
  DDRAM[1][9] = cs2;
  DDRAM[1][10] = ':';
  DDRAM[1][11] = '0';
  DDRAM[1][12] = '0';
  DDRAM[1][13] = ' ';
  //  DDRAM[1][14] = ' ';
  DDRAM[1][15] = ce1;
  DDRAM[1][16] = ce2;
  DDRAM[1][17] = ':';
  DDRAM[1][18] = '0';
  DDRAM[1][19] = '0';
}
//---------------------------------------------------------------------------------------------------------
void set_bright() 
{
  scr = 0;
  byte l1, l2, l3, nl1, nl2, nl3, dl1, dl2, dl3, i;
  l1 = light / 100 + 0x30;
  l2 = light / 10 % 10 + 0x30;
  l3 = light % 10 + 0x30;
  nl1 = NIGHT / 100 + 0x30;
  nl2 = NIGHT / 10 % 10 + 0x30;
  nl3 = NIGHT % 10 + 0x30;
  dl1 = DAY / 100 + 0x30;
  dl2 = DAY / 10 % 10 + 0x30;
  dl3 = DAY % 10 + 0x30;

  cursor_x = 0;
  for (i = 0; i < 20; i++)  
      {
        data = pgm_read_byte(&menu_screen[8][i]);
        Record(cursor_x, data);
      }
  NIGHT = constrain(NIGHT, 0, 240);
  DAY = constrain(DAY, NIGHT + 5, 255);

  //__________________________________________Ночь
  if (set_param == 0) 
                    {
                      DDRAM[1][6] = arrow_left;
                      DDRAM[1][10] = arrow_right;
                      DDRAM[1][11] = ' ';
                      DDRAM[1][15] = ' ';
                      DDRAM[1][17] = ' ';
                      DDRAM[1][19] = ' ';

                      if (enc.right())  
                        {
                          if (NIGHT < 250) 
                            {
                              NIGHT += 5;
                              //        NIGHT = constrain(NIGHT, 0, 250);
                              EEPROM.update(3, NIGHT);
                              //   beep ();
                            }
                        }
                      if (enc.left())  
                        {
                          if (NIGHT > 0) 
                            {
                              NIGHT -= 5;
                              //       NIGHT = constrain(NIGHT, 0, 250);
                              EEPROM.update(3, NIGHT);
                              //   beep ();
                            }
                        }
                    }
  //__________________________________________День
  if (set_param == 1) 
                    {
                      DDRAM[1][6] = ' ';
                      DDRAM[1][10] = ' ';
                      DDRAM[1][11] = arrow_left;
                      DDRAM[1][15] = arrow_right;
                      DDRAM[1][17] = ' ';
                      DDRAM[1][19] = ' ';

                      if (enc.right())  
                        {
                          if (DAY < 255) 
                            {
                              DAY += 5;
                              //       DAY = constrain(DAY, NIGHT + 5, 255);
                              EEPROM.update(5, DAY);
                              //   beep ();
                            }
                        }
                      if (enc.left())  
                        {
                          if (DAY > (NIGHT + 5)) 
                            {
                              DAY -= 5;
                              //        DAY = constrain(DAY, NIGHT + 5, 255);
                              EEPROM.update(5, DAY);
                              //   beep ();
                            }
                        }
                    }

  if (set_param == 2) 
                  {
                    dim = 1;
                    DDRAM[1][6] = ' ';
                    DDRAM[1][10] = ' ';
                    DDRAM[1][11] = ' ';
                    DDRAM[1][15] = ' ';
                    DDRAM[1][17] = arrow_left;
                    DDRAM[1][19] = arrow_right;

                    if (enc.right()) 
                      {
                        if (dim_level < 7)  
                          {
                          dim_level += 1;
                          dim_level = constrain(dim_level, 0, 7);
                          EEPROM.update(1, dim_level);
                          beep ();
                          }
                      }
                    if (enc.left())  
                          {
                            if (dim_level > 0)  
                              {
                                dim_level -= 1;
                                dim_level = constrain(dim_level, 0, 7);
                                EEPROM.update(1, dim_level);
                                beep ();
                              }
                          }
                  }
    DDRAM[1][0] = ' ';
    DDRAM[1][1] = l1;
    DDRAM[1][2] = l2;
    DDRAM[1][3] = l3;
    DDRAM[1][4] = ' ';
    DDRAM[1][5] = ' ';
    //  DDRAM[1][6] = ' ';
    DDRAM[1][7] = nl1;
    DDRAM[1][8] = nl2;
    DDRAM[1][9] = nl3;
    //  DDRAM[1][10] = ' ';
    //  DDRAM[1][11] = ' ';
    DDRAM[1][12] = dl1;
    DDRAM[1][13] = dl2;
    DDRAM[1][14] = dl3;
    //  DDRAM[1][15] = ' ';
    DDRAM[1][16] = ' ';
//    DDRAM[1][17] = dim_l1;
    DDRAM[1][18] = dim_level + 0x31;
    //  DDRAM[1][19] =
}
//---------------------------------------------------------------------------------------------------------
void get_temp_indoor() //___Получение данных о температуре и обработка (округление, разбиение на разряды)
  { 
    int RAWtemp;
    if (bme_online) 
    {
      RAWtemp  = (bme.readTemperature()  * 10 - COR_TEMP);
      //  } else if (htu_online) {
      //   RAWtemp  = (htu.getTemperatureWait() * 10 - COR_TEMP);
    } 
    else     
      RAWtemp  = (rtc.getTemperatureFloat() * 10 - COR_TEMP);
    
    temp_indoor_c1 = abs(RAWtemp / 100 % 10) + 0x30;
    temp_indoor_c2 = abs(RAWtemp / 10 % 10) + 0x30;
    temp_indoor_d = abs(RAWtemp % 10) + 0x30;
    if (RAWtemp >= 0) 
         t_sign_indoor = 0xAE;
   
    else  
         t_sign_indoor = '-';
  
  }
//---------------------------------------------------------------------------------------------------------
void get_hum() //___Получение данных о влажности и  обработка (округление, разбиение на разряды)
  { 
    int RAWhum;

      // RAWhum  = (bme.readHumidity() * 10);
      //  } else if (htu_online) {
      //  RAWhum  = (htu.getHumidityWait() * 10);
      RAWhum  = (aht20.getHumidity() * 10);

    hum_round_c1 = (RAWhum / 100 % 10) + 0x30;
    hum_round_c2 = (RAWhum / 10 % 10) + 0x30;
    hum_round_d = (RAWhum % 10) + 0x30;
  }
//---------------------------------------------------------------------------------------------------------
void get_pres() //___Получение данных о давлении и обработка (округление, разбиение на разряды)
  { 
    float pressure = bme.readPressure();
    int RAWpres = (pressureToMmHg(pressure));
    pres_round_c1 = RAWpres / 100 % 10 + 0x30;
    pres_round_c2 = RAWpres / 10 % 10 + 0x30;
    pres_round_c3 = RAWpres % 10 + 0x30;
  }
//---------------------------------------------------------------------------------------------------------
void beep() 
  {
    digitalWrite(3, HIGH);
    delay(150);
    digitalWrite(3, LOW);
  }
//---------------------------------------------------------------------------------------------------------
  
void output_DDRAM() // Функция выводит двадцать рядов символов из DDRAM на дисплей,
  {
    static byte gr, i, l;

    for (l = 0; l < 2; l++) 
      {
        for (i = 0; i < 7; i++) 
          {
            if (!vert)  
            {
                if (font % 2 == 0)  
                    {
                    Symbol_buf[l][i] = pgm_read_byte(&symbol[DDRAM[l + 2][gr]][i]);
                    Symbol_buf[l][i + 8] = pgm_read_byte(&symbol[DDRAM[l][gr]][i]);            
                    } 
                else 
                    {
                    Symbol_buf[l][i] = pgm_read_byte(&symbol_bold[DDRAM[l + 2][gr]][i]);
                    Symbol_buf[l][i + 8] = pgm_read_byte(&symbol_bold[DDRAM[l][gr]][i]);
                    }
              } 
              else 
                  {
                    Symbol_buf[l][i] = pgm_read_byte(&symbol_v[DDRAM[l + 2][gr]][i]);
                    Symbol_buf[l][i + 8] = pgm_read_byte(&symbol_v[DDRAM[l][gr]][i]);
                  }
          }
      }

    if (scroll) 
        {
          if (DDRAM[2][gr] == DDRAM[0][gr])  
              {
                Scroll[0] = 0;
              } 
              else  
              {
                scroll_flag = 1;
                Scroll[0] = scroll_x;
              }

          if (DDRAM[3][gr] == DDRAM[1][gr])  
              {
                Scroll[1] = 0;
              } 
              else  
              {
                scroll_flag = 1;
                Scroll[1] = scroll_x;
              }
        } 
    else 
        {
          Scroll[0] = 8;
          Scroll[1] = 8;
        }
    output(gr - scr);
    if (gr < (scr + 20)) 
      gr++;
 
    else  
      gr = scr;

  }
//---------------------------------------------------------------------------------------------------------
void Update_DDRAM() 
  {
    unsigned char h;
    for (h = 0; h < 30; h++) 
      {
        DDRAM[2][h] = DDRAM[0][h];
        DDRAM[3][h] = DDRAM[1][h];
      }
  }
//---------------------------------------------------------------------------------------------------------
void readDS18B20()  //___Получение данных о температуре за окном
  {
    ds = !ds;
    if (ds == 0)   
      DS.requestTemp();
 
    else   
//      DS.getTemp();
      temp = DS.getTemp();
  
    DS_temp = temp * 100 ;
    temp_DS_c1 = abs(DS_temp / 1000) % 10;
    temp_DS_c2 = abs(DS_temp / 100) % 10;
    temp_DS_d = abs(DS_temp / 10) % 10;

    if (((temp_DS_c1) == 0) && ((DS_temp) >= 0)) 
        {
          temp_DS_10 = '*';
          temp_DS_01 = ' ';
        } 
    else if (((temp_DS_c1) == 0) && ((DS_temp) < 0)) 
            {
              temp_DS_10 = '*';
              temp_DS_01 = '-';
            }
    else if (((temp_DS_c1) != 0) && ((DS_temp) < 0))
            {
              temp_DS_10 = '-';
              temp_DS_01 = temp_DS_c1 + 0x30;
            }
    else if (((temp_DS_c1) != 0) && ((DS_temp) >= 0)) 
            {
              temp_DS_10 = '*';
              temp_DS_01 = temp_DS_c1 + 0x30;
            }
  }
//---------------------------------------------------------------------------------------------------------
void show_temp_digital() 
  {
    scr = 0;
    byte m1, m2, m3, t10, t11, t12, t13, t14, t15; //_________________Гашение старшего ноля, знак минус.
    if (((temp_DS_c1) == 0) && ((DS_temp) >= 0)) 
        {
          m1 = ' ';
          m2 = ' ';
          m3 = ' ';
          t10 = ' ';
          t11 = ' ';
          t12 = ' ';
          t13 = ' ';
          t14 = ' ';
          t15 = ' ';

        } 
    else if (((temp_DS_c1) == 0) && ((DS_temp) < 0)) 
            {
              m1 = ' ';
              m2 = ' ';
              m3 = ' ';
              t10 = ' ';
              t11 = ' ';
              t12 = ' ';
              t13 = 0xB1;
              t14 = 0xB1;
              t15 = 0xB1;

            } 
    else if (((temp_DS_c1) != 0) && ((DS_temp) < 0)) 
            {
              m1 = 0xB1;
              m2 = 0xB1;
              m3 = 0xB1;
              t10 = pgm_read_byte(&BigNum[temp_DS_c1][0]);
              t11 = pgm_read_byte(&BigNum[temp_DS_c1][1]);
              t12 = pgm_read_byte(&BigNum[temp_DS_c1][2]);
              t13 = pgm_read_byte(&BigNum[temp_DS_c1][3]);
              t14 = pgm_read_byte(&BigNum[temp_DS_c1][4]);
              t15 = pgm_read_byte(&BigNum[temp_DS_c1][5]);

            } 
    else if (((temp_DS_c1) != 0) && ((DS_temp) >= 0)) 
            {
              m1 = ' ';
              m2 = ' ';
              m3 = ' ';
              t10 = pgm_read_byte(&BigNum[temp_DS_c1][0]);
              t11 = pgm_read_byte(&BigNum[temp_DS_c1][1]);
              t12 = pgm_read_byte(&BigNum[temp_DS_c1][2]);
              t13 = pgm_read_byte(&BigNum[temp_DS_c1][3]);
              t14 = pgm_read_byte(&BigNum[temp_DS_c1][4]);
              t15 = pgm_read_byte(&BigNum[temp_DS_c1][5]);
            }

    DDRAM[0][0] = ' ';
    DDRAM[0][1] = ' ';
    DDRAM[0][2] = ' ';
    DDRAM[0][3] = ' ';
    DDRAM[0][4] = t10;
    DDRAM[0][5] = t11;
    DDRAM[0][6] = t12;
    DDRAM[0][7] = pgm_read_byte(&BigNum[temp_DS_c2][0]);
    DDRAM[0][8] = pgm_read_byte(&BigNum[temp_DS_c2][1]);
    DDRAM[0][9] = pgm_read_byte(&BigNum[temp_DS_c2][2]);
    DDRAM[0][10] = ' ';
    DDRAM[0][11] = pgm_read_byte(&BigNum[temp_DS_d][0]);
    DDRAM[0][12] = pgm_read_byte(&BigNum[temp_DS_d][1]);
    DDRAM[0][13] = pgm_read_byte(&BigNum[temp_DS_d][2]);
    DDRAM[0][14] = 0xD4;
    DDRAM[0][15] = 0xD5;
    DDRAM[0][16] = 0xB0;
    DDRAM[0][17] = 0xB1;
    DDRAM[0][18] = 0xD6;
    DDRAM[0][19] = ' ';

    DDRAM[1][0] = ' ';
    DDRAM[1][1] = m1;
    DDRAM[1][2] = m2;
    DDRAM[1][3] = m3;
    DDRAM[1][4] = t13;
    DDRAM[1][5] = t14;
    DDRAM[1][6] = t15;
    DDRAM[1][7] = pgm_read_byte(&BigNum[temp_DS_c2][3]);
    DDRAM[1][8] = pgm_read_byte(&BigNum[temp_DS_c2][4]);
    DDRAM[1][9] = pgm_read_byte(&BigNum[temp_DS_c2][5]);
    DDRAM[1][10] = 0xAA;
    DDRAM[1][11] = pgm_read_byte(&BigNum[temp_DS_d][3]);
    DDRAM[1][12] = pgm_read_byte(&BigNum[temp_DS_d][4]);
    DDRAM[1][13] = pgm_read_byte(&BigNum[temp_DS_d][5]);
    DDRAM[1][14] = ' ';
    DDRAM[1][15] = ' ';
    DDRAM[1][16] = 0xB3;
    DDRAM[1][17] = 0xB4;
    DDRAM[1][18] = 0xD7;
    DDRAM[1][19] = ' ';
  }
//---------------------------------------------------------------------------------------------------------
void show_error() 
  {
    byte i;
    cursor_x = 0;
    for (i = 0; i < 40; i++)  
        {
          data = pgm_read_byte(&Err[i]);
          Record(cursor_x, data);
        }
  }
//---------------------------------------------------------------------------------------------------------
  
void Record(byte curs_x, byte symbol_code) // Функция записи цифрового кода символа в DDRAM
  {

    if (curs_x < 20)   DDRAM[0][curs_x] = symbol_code;
    else  DDRAM[1][curs_x - 20] = symbol_code;
    cursor_x++;
    if (curs_x == 39)  cursor_x = 0;
  }
//---------------------------------------------------------------------------------------------------------
void show_menu()  
  {
    byte i;
    cursor_x = 0;
    for (i = 0; i < 20; i++)  
        {
          data = pgm_read_byte(&menu_screen[0][i]);
          Record(cursor_x, data);
        }
    cursor_x = 20;
    for (i = 0; i < 20; i++)  
        {
          data = pgm_read_byte(&menu_screen[menu_item][i]);
          Record(cursor_x, data);
        }
  }
//---------------------------------------------------------------------------------------------------------
void set_temp_corr()  
  {
    byte tc1, tc2, tc3, i;
    tc1 = COR_TEMP / 100 + 0x30;
    tc2 = COR_TEMP / 10 % 10 + 0x30;
    tc3 = COR_TEMP % 10 + 0x30;

    cursor_x = 0;
    for (i = 0; i < 20; i++)  
        {
          data = pgm_read_byte(&menu_screen[9][i]);
          Record(cursor_x, data);
        }

    //__________________________________________Коррекция Температуры
    //  if (set_param == 2) {

    if (enc.right())  
        {
          if (COR_TEMP < 200)  
            {
              COR_TEMP += 1;
              COR_TEMP = constrain(COR_TEMP, 0, 200);
              EEPROM.update(7, COR_TEMP);
              //  beep ();
            }
        }
    if (enc.left())  
        {
          if (COR_TEMP > 0)  
            {
              COR_TEMP -= 1;
              COR_TEMP = constrain(COR_TEMP, 0, 200);
              EEPROM.update(7, COR_TEMP);
              //  beep ();
            }
        }
    if (bme_online) 
        {
          DDRAM[1][0] = ' ';
          DDRAM[1][1] = ' ';
          DDRAM[1][2] = arrow_left;
          DDRAM[1][3] = 'B';
          DDRAM[1][4] = 'M';
          DDRAM[1][5] = 'E';
          DDRAM[1][6] = '2';
          DDRAM[1][7] = '8';
          DDRAM[1][8] = '0';
    /*      
        }  else if (htu_online) {
          DDRAM[1][0] = ' ';
          DDRAM[1][1] = ' ';
          DDRAM[1][2] = arrow_left;
          DDRAM[1][3] = 'H';
          DDRAM[1][4] = 'T';
          DDRAM[1][5] = 'U';
          DDRAM[1][6] = '2';
          DDRAM[1][7] = '1';
          DDRAM[1][8] = 'D';
    */      
        }  
    else  
        {
          DDRAM[1][0] = ' ';
          DDRAM[1][1] = ' ';
          DDRAM[1][2] = arrow_left;
          DDRAM[1][3] = 'D';
          DDRAM[1][4] = 'S';
          DDRAM[1][5] = '3';
          DDRAM[1][6] = '2';
          DDRAM[1][7] = '3';
          DDRAM[1][8] = '1';
        }

    DDRAM[1][9] = ' ';
    DDRAM[1][10] = '-';
    DDRAM[1][11] = tc1;
    DDRAM[1][12] = tc2;
    DDRAM[1][13] = '.';
    DDRAM[1][14] = tc3;
    DDRAM[1][15] = 0xAB;
    DDRAM[1][16] = 'C';
    DDRAM[1][17] = arrow_right;
    DDRAM[1][18] = ' ';
    DDRAM[1][19] = ' ';
  }
//---------------------------------------------------------------------------------------------------------
void show_time_sandclock()  
  {
    scr = 0;
    byte h1, h2, m1, m2, s1, s2, s, d, mon, wd1, wd2, h10, h11;
    byte i;
    DateTime now = rtc.getTime();
    h1 = now.hour / 10;
    h2 = now.hour % 10;
    m1 = now.minute / 10;
    m2 = now.minute % 10;
    s = now.second + 0x21;
    s1 = now.second / 10;
    s2 = now.second % 10;
    //  d1 = now.date / 10 + 0x30;
    //  d2 = now.date % 10 + 0x30;
    d = now.date + 0x21;
    mon = (now.month) + 0x21;
    wd1 = (now.day - 1) + 0x99;
    wd2 = (now.day - 1) + 0xA0;

    if ((now.hour >= chime_start) && (now.hour <= chime_end) && chime_enable) 
        {
          if ((now.minute == 00) && (now.second == 00) && beep_flag)  
              {
                beep();
                beep_flag = 0;
              }
          if ((now.minute == 00) && (now.second == 01))    
            beep_flag = 1;
      
        }

    if (rtc.getHours() < 10) 
        {
          h10 = ' ';
          h11 = ' ';
        } 
    else 
        {
          h10 = h1;
          h11 = h1 + 0x0A;
        }

    if (font_v) 
        {
          DDRAM[0][12] = s1;
          DDRAM[0][13] = s1 + 0x0A;
          DDRAM[0][14] = dot_l_vert;

          DDRAM[1][12] = s2;
          DDRAM[1][13] = s2 + 0x0A;
          DDRAM[1][14] = dot_r_vert;
        } 
    else  
        {
          DDRAM[0][12] = ' ';
          DDRAM[0][13] = ' ';
          DDRAM[0][14] = ' ';

          DDRAM[1][12] = ' ';
          DDRAM[1][13] = s;
          DDRAM[1][14] = dots_vert;
        }

    cursor_x = 4;
    for (i = 0; i < 8; i++)  
        {
          data = pgm_read_byte(&sandclock[sand][i]);
          // data = i + 0x72;
          Record(cursor_x, data);
          DDRAM[2][i + 4] = DDRAM[0][i + 4];
        }
    cursor_x = 24;
    for (i = 0; i < 8; i++)  
        {
          data = pgm_read_byte(&sandclock[sand][i + 8]);
          // data = i + 0x72;
          Record(cursor_x, data);
          DDRAM[3][i + 4] = DDRAM[1][i + 4];
        }

    DDRAM[0][0] = ' ';
    DDRAM[0][1] = d;
    DDRAM[0][2] = wd1;
    DDRAM[0][3] = ' ';

    DDRAM[0][15] = m1;
    DDRAM[0][16] = m1 + 0x0A;
    DDRAM[0][17] = dot_l_vert;
    DDRAM[0][18] = h10;
    DDRAM[0][19] = h11;

    DDRAM[1][0] = mon;
    DDRAM[1][1] = ' ';
    DDRAM[1][2] = wd2;
    DDRAM[1][3] = ' ';

    DDRAM[1][15] = m2;
    DDRAM[1][16] = m2 + 0x0A;
    DDRAM[1][17] = dot_r_vert;
    DDRAM[1][18] = h2;
    DDRAM[1][19] = h2 + 0x0A;

    DDRAM[2][14] = DDRAM[0][14];
    DDRAM[2][17] = DDRAM[0][17];
    DDRAM[3][14] = DDRAM[1][14];
    DDRAM[3][17] = DDRAM[1][17];
  }
//---------------------------------------------------------------------------------------------------------
void show_temp_scale() 
  {
    byte i;
    int RAWtemp = round(temp) + 43 ; //_________округление, сдвиг диапазона
    RAWtemp = constrain(RAWtemp, 0, 85); //____ограничение диапазона
    byte t0, t1, t2, t3, t4;
    t0 = (RAWtemp / 5); //____Общее Число заполненых знакомест
    t1 = (RAWtemp % 5); //____Число полосок в знакоместе
    t2 = (16 - (t0)); //______Число пустых знакомест
    int DIGtemp = round(temp); //_________округление
    DIGtemp = constrain(DIGtemp, -59, 59); //____ограничение диапазона
    t3 = abs(DIGtemp / 10); //__десятки градусов маааленьких циферок
    t4 = abs(DIGtemp % 10); //__единицы градусов маааленьких циферок

    const byte t_minus[6] = {0xB1, 0xB3, 0xB4, 0xB5, 0xB6, 0xA7};
    const byte t_plus[6] = {0xB2, 0xB7, 0xB8, 0xB9, 0xBA, 0xA8};
    const byte t_deg[10] = {0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4};
    const byte t_sc[5] = {0xC5, 0xC6, 0xC7, 0xC8, 0xC9};
    const byte scale[19] = {0xCB, 0xAF, 0xAE, 0xAD, 0xAC, 0xAB, 0xAA, 0xA9, 0xA7, 0xBB, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xCC};

    cursor_x = 0;
    for (i = 0; i < 19; i++)  
        {
          data = i[scale];
          Record(cursor_x, data);
        }

    cursor_x = 21;
    for (i = 0; i < t0; i++)  
        {
          data = 0xCA;
          Record(cursor_x, data);
        }
    if (RAWtemp < 85) 
        {
          data = t1[t_sc];
          Record(cursor_x, data);

          for (i = 0; i < t2; i++)  
              {
                data = 0xC5;
                Record(cursor_x, data);
              }
        }
    if (temp < 0)  
      DDRAM[0][19] = t_minus[t3];
 
    else   
      DDRAM[0][19] = t_plus[t3];
 

    DDRAM[1][0] = 0xB0;
    DDRAM[1][18] = 0xB0;
    DDRAM[1][19] = t_deg[t4];
  }
//---------------------------------------------------------------------------------------------------------
  void countdown_timer()  
  {
    byte m1, m2, s1, s2, i;
    m1 = cdt_m / 10;
    m2 = cdt_m % 10;
    s1 = cdt_s / 10;
    s2 = cdt_s % 10;


    if (!cdt_run) 
    {
      if (enc.right())  
          {
            if (cdt_m < 90) 
                {
                  if (cdt_m < 15)       
                    cdt_m += 1;
              
                  else       
                    cdt_m += 5;
              
                }
          }
      if (enc.left())  
          {
            if (cdt_m > 0) 
                {
                  if (cdt_m < 16)         
                    cdt_m -= 1;
                
                  else        
                    cdt_m -= 5;
                
                }
          }
    }
    if ((cdt_run) && (cdt_m > 0)) 
        {
          if (cdt_s < 0)  
            {
              cdt_s = 59;
              cdt_m -= 1;
            }
        }
/*    
    if ((cdt_run) && (cdt_m == 0) && (cdt_s == 0)) {
      cdt_run = !cdt_run;
      for (i = 0; i < 5; i++)  {
        beep();
        delay(150);
      }
    }
*/
    cursor_x = 4;
    for (i = 0; i < 8; i++)  
        {
          data = pgm_read_byte(&sandclock[sand][i]);
          Record(cursor_x, data);
          DDRAM[2][i + 4] = DDRAM[0][i + 4];
        }
    cursor_x = 24;
    for (i = 0; i < 8; i++)  
        {
          data = pgm_read_byte(&sandclock[sand][i + 8]);
          Record(cursor_x, data);
          DDRAM[3][i + 4] = DDRAM[1][i + 4];
        }
    if (font_v) 
        {
          DDRAM[0][12] = s1;
          DDRAM[0][13] = s1 + 0x0A;

          DDRAM[1][12] = s2;
          DDRAM[1][13] = s2 + 0x0A;
          if (cdt_run)  
              {
                DDRAM[0][14] = dot_l_vert;
                DDRAM[1][14] = dot_r_vert;
              } 
          else  
              {
                DDRAM[0][14] = 0x14;
                DDRAM[1][14] = 0x15;
              }
        } 
    else  
        {
          DDRAM[0][12] = ' ';
          DDRAM[0][13] = ' ';

          DDRAM[1][12] = ' ';
          DDRAM[1][13] = cdt_s + 0x21;
          if (cdt_run)  
              {
                DDRAM[0][14] = ' ';
                DDRAM[1][14] = dots_vert;
              } 
          else  
              {
                DDRAM[0][14] = ' ';
                DDRAM[1][14] = 0x16;
              }
        }

    if (cdt_m < 10) 
        {
          DDRAM[0][15] = ' ';
          DDRAM[0][16] = ' ';
        } 
    else  
        {
          DDRAM[0][15] = m1;
          DDRAM[0][16] = m1 + 0x0A;
        }

    DDRAM[1][15] = m2;
    DDRAM[1][16] = m2 + 0x0A;

    DDRAM[2][14] = DDRAM[0][14];
    DDRAM[3][14] = DDRAM[1][14];

    DDRAM[0][17] = ' ';
    DDRAM[0][18] = ' ';
    DDRAM[0][19] = ' ';
    DDRAM[1][17] = ' ';
    DDRAM[1][18] = ' ';
    DDRAM[1][19] = ' ';

    DDRAM[0][0] = ' ';
    DDRAM[0][1] = ' ';
    DDRAM[0][2] = ' ';
    DDRAM[0][3] = ' ';
    DDRAM[1][0] = ' ';
    DDRAM[1][1] = ' ';
    DDRAM[1][2] = ' ';
    DDRAM[1][3] = ' ';
  }

