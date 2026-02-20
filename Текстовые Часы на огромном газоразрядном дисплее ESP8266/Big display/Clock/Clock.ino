#include <RTClib.h> 
// #include <SoftwareSerial.h>
#include <Wire.h>
#include <ESP8266WiFi.h>  // Для ESP8266, для ESP32 используйте <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Timezone.h> // Библиотека для управления часовыми поясами

// Определение правил для зимнего (UTC+2) и летнего (UTC+3) времени
TimeChangeRule DSTRule = {"EEST", Last, Sun, Mar, 2, 180}; // Летнее время: UTC+3, последнее воскресенье марта, 02:00
TimeChangeRule StdRule = {"EET", Last, Sun, Oct, 2, 120};  // Зимнее время: UTC+2, последнее воскресенье октября, 02:00
Timezone myTZ(DSTRule, StdRule); // Создаем объект часового пояса

// const long timezoneOffset = 2; // ? hours
const char *ntpServer  = "pool.ntp.org"; // change it to local NTP server if needed

// Wi-Fi настройки
// const char* ssid = "TP-Link_22DC";
// const char* password = "Mar4enko2704";
const char* ssid[] = {"TP-LINK_333856", "TP-Link_22DC"};
const char* password[] = {"", "Mar4enko2704"};
const int numNetworks = 2;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer);

RTC_DS3231 RTC;

// Массив со словами 
char* myNumeral[]={
    "HO\272b",          // ноль - 0
    "O\262HA",          // одна - 1
    "\262BE",           // две - 2
    "TP\270",           // три - 3
    "\301ET\305PE",     // четыре - 4
    "\273\312Tb",       // пять - 5
    "\302ECTb",         // шесть - 6
    "CEMb",             // семь - 7
    "BOCEMb",           // восемь - 8
    "\262EB\312Tb",     // девять - 9
    "\262EC\312Tb",     // десять - 10
    "O\262\270HHA\262\300ATb",  // одиннадцать - 11
    "\262BEHA\262\300ATb",      // двенадцать - 12
    "TP\270HA\262\300ATb",      // тринадцать - 13
    "\301ET\305PHA\262\300ATb", // четырнадцать - 14
    "\273\312THA\262\300ATb",   // пятнадцать - 15
    "\302ECTHA\262\300ATb",     // шестнадцать - 16
    "CEMHA\262\300ATb",         // семнадцать - 17
    "BOCEMHA\262\300ATb",       // восемнадцать - 18
    "\262EB\312THA\262\300ATb", // девятнадцать - 19
    "\262BA\262\300ATb",        // двадцать - 20
    "TP\270\262\300ATb",        // тридцать - 21
    "COPOK",                    // сорок - 22
    "\273\312Tb\262EC\312T",    // пятьдесят - 23
    
    "O\262\270H",               // один - 24
    "\262BA",                   // два - 25
    
    "\301ACOB",                 // часов - 26
    "\301AC",                   // час - 27
    "\301ACA",                  // часа - 28
    
    "M\270H\274T",              // минут - 29
    "M\270H\274TA",             // минута - 30
    "M\270H\274T\305",          // минуты - 31
    "M\270H",                   // мин - 32
    "\273OHE\262E\272bH\270K",  // понедельник - 33
    "BTOPH\270K",               // вторник - 34
    "CPE\262A",                 // среда - 35
    "\301ETBEP\261",            // четверг - 36
    "\273\312TH\270\300A",      // пятница - 37
    "C\274\260\260OTA",         // суббота - 38
    "BOCKPECEHbE",              // воскресенье - 39
    "\312HBAP\312",             // января - 40
    "\277EBPA\272\312",         // февраля - 41
  //  "\214\200\220\222\200",     // марта - 42
    "MAPTA",                    // марта - 42
    "A\273PE\272\312",          // апреля - 43
    "MA\312",                   // мая - 44
    "\270\311H\312",            // июня - 45
    "\270\311\272\312",         // июля - 46
    "AB\261\274CTA",            // августа - 47
    "CEHT\312\260P\312",        // сентября - 48
    "OKT\312\260P\312",         // октября - 49
    "HO\312\260P\312",          // ноября - 50
    "\262EKA\260P\312",         // декабря - 51
    "\261O\262"                 // год - 52
};


String stringOne, stringTwo, stringThree, stringFour, stringFive, stringSix, stringHour, stringMin,
        stringHourd,stringHourdd, stringMind,stringMindd, stringSecd, stringSecdd, stringPerd,
        stringDaty, stringVtor, stringdday,stringDatyd, stringddayy, stringdmonth, stringdmonthh, stringGod, stringGodn; 
        // массыв переменных для сборки времени словами из массива

const int buttonPin         = 2;    // на второй вывод повесил кнопку выбора варианта отображения времени
const int buttonChplus      = 3;    // на третий вывод повесил кнопку коррентировки час+
const int buttonChminus     = 4;    // на четвертый вывод повесил кнопку коррентировки час-
const int buttonMinplus     = 5;    // на пятый вывод повесил кнопку коррентировки минуты+
const int buttonMinminus    = 6;    // на шестой вывод повесил кнопку коррентировки минуты-
const int buttonSeknol      = 7;    // на седьмой вывод повесил кнопку коррентировки секунды в ноль
const int buttonDispleysbros = 8;   // на восьмой вывод повесил кнопку сброса дисплея
const int buttonCorrHour    = 12;   // на двенадцатом выводе переключатель корректировки час минута секунда
const int buttonCorrDay     = 9;    // на двенадцатом выводе переключатель корректировки дней месяца года

int den, mec, god, dday, agod, ygod, mgod;         // для вычисления дня недели
int yar, mesyac, denec, chas, minuta, sekunda;  // для корректировки
int korrektwrem;                                // коррекция ошибки часов реального времени

bool PrevState = false;

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
                        denec = ti->tm_mday;
                        mesyac = ti->tm_mon + 1;    // Месяцы начинаются с 0, добавляем 1
                        yar = ti->tm_year + 1900; // Годы отсчитываются от 1900
                        dday = ti->tm_wday;    // День недели (0 - воскресенье, 6 - суббота)
                        chas = ti->tm_hour;
                        minuta = ti->tm_min;
                        sekunda = ti->tm_sec;
                        
                        // Синхронизация RTC с локальным временем
                        RTC.adjust(DateTime(localTime));
              }
}


void setup()  
{
    delay(500);
    // Инициализируем последовательный интерфейс и ждем открытия порта:
    Serial.begin(9600);
    Wire.begin();
    RTC.begin();
    // WiFi.begin(ssid, password);
    //           for(byte i = 0; i < 15; i++)
    //           {
    //               if (WiFi.status() != WL_CONNECTED) 
    //               {
    //                 delay(500);
    //                 Serial.print(F("."));
    //                 // matrix.show();
    //                 // Serial.print(".");
    //               }
    //               else
    //               {
    //               // matrix.fillScreen(0);
    //               // matrix.setCursor(0, 0);
    //               // Serial.println("\nConnected to WiFi");
    //               Serial.print(F("Ok!"));
    //               // matrix.show();
    //               delay(2000);
    //               adjust_time_date();
    //               return; // Выходим из setup() после подключения
    //               }
    //           }
    // RTC.adjust(DateTime(2018, 2, 5, 19, 16, 0));  // задаём год/ месяц/ дата/ часы/ минуты/ секунды
    // RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));

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
        Serial.print(F("."));
        // Serial.print(".");
        // char2Arr('.', attempts, 0, FONT2);
        // refresh_display();
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
        Serial.print(F("Ok!"));
        // char2Arr('O', attempts, 0, FONT2);
        // char2Arr('k', (attempts - 6), 0, FONT2);
        // char2Arr('!', (attempts - 12), 0, FONT2);
        // refresh_display();
        delay(2000);
        adjust_time_date();
        delay(50);
        break;
        } 
    else 
        {
        // Serial.println("\nНе удалось подключиться к ");
        // Serial.println(ssid[i]);
        // clear_Display();
        // WiFi.disconnect(); // Отключаемся перед следующей попыткой
        Serial.print("\f");
        }
  }// end for

  // Если не удалось подключиться ни к одной сети
  if (WiFi.status() != WL_CONNECTED) 
    {
        // Serial.println("Не удалось подключиться ни к одной сети!");
        // char2Arr('W', 31, 0, FONT2);
        // char2Arr('i', 25, 0, FONT2);
        // char2Arr('F', 19, 0, FONT2);
        // char2Arr('i', 13, 0, FONT2);
        // char2Arr('?', 7, 0, FONT2);
        // char22Arr('F', 1, 0);
        // char22Arr('i', 0, 0);
        Serial.print("\f");
        Serial.print(F("No WiFi"));
        // refresh_display();
        delay(2000);
        // Read__Time();
        // delay(40);
    }  

    Serial.print("\f");
    Serial.println("    Good aftenoon!");
    delay(2000);
    
    // pinMode(buttonPin, INPUT_PULLUP);          //  переключение режимов отображения времени
    // pinMode(buttonChplus, INPUT_PULLUP);       //  корректирока час +
    // pinMode(buttonChminus, INPUT_PULLUP);      //  корректировка час -
    // pinMode(buttonMinplus, INPUT_PULLUP);      //  корректировка минута +
    // pinMode(buttonMinminus, INPUT_PULLUP);     //  корректировка минута -
    // pinMode(buttonSeknol, INPUT_PULLUP);       //  корректировка секунда в 0
    // pinMode(buttonDispleysbros, INPUT_PULLUP); //  сброс дисплея
    // pinMode(buttonCorrHour, INPUT_PULLUP);     //  коррекция времени часминсек
    // pinMode(buttonCorrDay, INPUT_PULLUP);      //  коррекция времени деньмесяцгод
    Serial.print("\f");
}

void loop() // run over and over
{    
    DateTime now = RTC.now();
    yar = now.year();       // присваиваем переменным значения года
    mesyac = now.month();   // месяца
    denec = now.day();      // дня
    chas = now.hour();      // часа
    minuta = now.minute();  // минуты
    sekunda = now.second(); // и секунды
    // den = now.day();
    // mec = now.month();
    // god = now.year();
    dday = now.dayOfTheWeek();
	/*
    yar = 2021;       // присваиваем переменным значения года
    mesyac = 10;   // месяца
    denec = 28;      // дня
    chas = 15;      // часа
    minuta = 58;  // минуты
    sekunda = 13; // и секунды
    den = 28;
    mec = 10;
    god = 2021;
	*/


                                                                    // считываем состояние тумблера корректировки часминсек
    // if (!(digitalRead(buttonCorrHour))){                            // если низкая, корректируем время часминсек
        
    //     if (!(digitalRead(buttonChplus))){                          //настройка времени кнопками    + час
    //         chas = chas + 1;
    //         RTC.adjust(DateTime(yar, mesyac, denec, chas, minuta, sekunda));
    //         delay(50);
    //     }
        
    //     if (!(digitalRead(buttonChminus))){                         //настройка времени кнопками    - час
    //         chas = chas - 1;
    //         RTC.adjust(DateTime(yar, mesyac, denec, chas, minuta, sekunda));
    //         delay(50);
    //     }
        
    //     if (!(digitalRead(buttonMinplus))){                         //настройка времени кнопками    + минуты
    //         minuta = minuta + 1;
    //         RTC.adjust(DateTime(yar, mesyac, denec, chas, minuta, sekunda));
    //         delay(50);
    //     }
        
    //     if (!(digitalRead(buttonMinminus))){                        //настройка времени кнопками    - минуты
    //         minuta = minuta - 1;
    //         RTC.adjust(DateTime(yar, mesyac, denec, chas, minuta, sekunda));
    //         delay(50);
    //     }
        
    //     if (!(digitalRead(buttonSeknol))){                          //настройка времени кнопками    секунда в ноль
    //         sekunda = 0;
    //         RTC.adjust(DateTime(yar, mesyac, denec, chas, minuta, sekunda));
    //         delay(50);
    //     }
        
    //     if (!(digitalRead(buttonDispleysbros))){                    //сбросить дисплей
    //         Serial.print("\f");
    //         delay(50);
    //     }
    // }


       

                                                                // считываем состояние тумблера корректировки деньмесяцгод
    // if (!(digitalRead(buttonCorrDay))){                         // если низкая, можно корректировать время деньмесяцгод
        
    //     if (!(digitalRead(buttonChplus))){                      //настройка времени кнопками    + год
    //         yar = yar + 1;
    //         RTC.adjust(DateTime(yar, mesyac, denec, chas, minuta, sekunda));
    //         delay(50);
    //     }
        
    //     if (!(digitalRead(buttonChminus))){                     //настройка времени кнопками    - год
    //         yar = yar - 1;
    //         RTC.adjust(DateTime(yar, mesyac, denec, chas, minuta, sekunda));
    //         delay(50);
    //     }
        
    //     if (!(digitalRead(buttonMinplus))){                     //настройка времени кнопками    + день
    //         denec = denec + 1;
    //         RTC.adjust(DateTime(yar, mesyac, denec, chas, minuta, sekunda));
    //         delay(50);
    //     }
        
    //     if (!(digitalRead(buttonMinminus))){                    //настройка времени кнопками    - день
    //         denec = denec - 1;
    //         RTC.adjust(DateTime(yar, mesyac, denec, chas, minuta, sekunda));
    //         delay(50);
    //     }
        
    //     if (!(digitalRead(buttonSeknol))){                      //настройка времени кнопками    + месяц
    //         mesyac = mesyac + 1;
    //         RTC.adjust(DateTime(yar, mesyac, denec, chas, minuta, sekunda));
    //         delay(50);
    //     }
        
    //     if (!(digitalRead(buttonDispleysbros))){                 //сбросить дисплей
    //         Serial.print("\f");
    //         delay(50);
    //     }
    // }



    // if (chas == 19 && minuta == 03 && sekunda == 30 && korrektwrem == 0 ){      //  корректировка часов реального времени 
    //     sekunda = sekunda - 7;
    //     korrektwrem = korrektwrem + 1 ;
    //     RTC.adjust(DateTime(yar, mesyac, denec, chas, minuta, sekunda));        // на 7 секунд
    // };
    
    // if (chas == 19 && minuta == 03 && sekunda == 59 ){
    //     korrektwrem = 0; 
    // }

  
bool rrr = false;
                                                                    // считываем состояние кнопки
    // if (!(digitalRead(buttonPin))) {                                // если высокая, время словами - иначе вся информация
if(rrr)
{
        switch (chas) 
        {                                             //собираем первую строку
            case 0:  stringOne =  myNumeral[0]; stringThree =  myNumeral[26]; stringHour = "     " + stringOne + " " + stringThree + "     "; break;
            case 1:  stringOne =  myNumeral[24]; stringThree =  myNumeral[27]; stringHour = "      " + stringOne + " " + stringThree + "      ";break;
            case 3:  stringOne =  myNumeral[3]; stringThree =  myNumeral[28]; stringHour = "      " + stringOne + " " + stringThree + "      ";break;
            case 4:  stringOne =  myNumeral[4]; stringThree =  myNumeral[28]; stringHour = "    " + stringOne + " " + stringThree + "     ";break;
            case 5:  stringOne =  myNumeral[5]; stringThree =  myNumeral[26]; stringHour = "     " + stringOne + " " + stringThree + "     ";break;
            case 6:  stringOne =  myNumeral[6]; stringThree =  myNumeral[26]; stringHour = "    " + stringOne + " " + stringThree + "     ";break;
            case 7:  stringOne =  myNumeral[7]; stringThree =  myNumeral[26]; stringHour = "     " + stringOne + " " + stringThree + "     ";break;
            case 8:  stringOne =  myNumeral[8]; stringThree =  myNumeral[26]; stringHour = "    " + stringOne + " " + stringThree + "    ";break;
            case 9:  stringOne =  myNumeral[9]; stringThree =  myNumeral[26]; stringHour = "    " + stringOne + " " + stringThree + "    ";break;
            case 10:  stringOne =  myNumeral[10]; stringThree =  myNumeral[26]; stringHour = "     " + stringOne + " " + stringThree + "     ";break;
            case 11:  stringOne =  myNumeral[11]; stringThree =  myNumeral[26]; stringHour = " " + stringOne + " " + stringThree + "  ";break;
            case 12:  stringOne =  myNumeral[12]; stringThree =  myNumeral[26]; stringHour = "  " + stringOne + " " + stringThree + "  ";break; 
            case 13:  stringOne =  myNumeral[13]; stringThree =  myNumeral[26]; stringHour = "  " + stringOne + " " + stringThree + "  ";break;
            case 14:  stringOne =  myNumeral[14]; stringThree =  myNumeral[26]; stringHour = " " + stringOne + " " + stringThree + " ";break;
            case 15:  stringOne =  myNumeral[15]; stringThree =  myNumeral[26]; stringHour = "  " + stringOne + " " + stringThree + "  ";break;
            case 16:  stringOne =  myNumeral[16]; stringThree =  myNumeral[26]; stringHour = " " + stringOne + " " + stringThree + "  ";break;
            case 17:  stringOne =  myNumeral[17]; stringThree =  myNumeral[26]; stringHour = "  " + stringOne + " " + stringThree + "  ";break;
            case 18:  stringOne =  myNumeral[18]; stringThree =  myNumeral[26]; stringHour = " " + stringOne + " " + stringThree + " ";break;
            case 19:  stringOne =  myNumeral[19]; stringThree =  myNumeral[26]; stringHour = " " + stringOne + " " + stringThree + " ";break;
            case 20:  stringOne =  myNumeral[20]; stringThree =  myNumeral[26]; stringHour = "   " + stringOne + " " + stringThree + "   ";break;
            case 21:  stringOne =  myNumeral[20]; stringTwo =  myNumeral[24]; stringThree =  myNumeral[27]; stringHour = " " + stringOne + " " + stringTwo + " " + stringThree + "  ";break;
            case 22:  stringOne =  myNumeral[20]; stringTwo =  myNumeral[25]; stringThree =  myNumeral[28]; stringHour = " " + stringOne + " " + stringTwo + " " + stringThree + "  ";break;
            case 23:  stringOne =  myNumeral[20]; stringTwo =  myNumeral[3]; stringThree =  myNumeral[28]; stringHour = " " + stringOne + " " + stringTwo + " " + stringThree + "  ";break;
        }
            
        switch (minuta) 
        {                             // собираем вторую строку
            case 0:  stringFour =  myNumeral[0]; stringSix =  myNumeral[29]; stringMin = "\n     " + stringFour + " " + stringSix + "     ";  break;
            case 1:  stringFour =  myNumeral[1]; stringSix =  myNumeral[30]; stringMin = "\n    " + stringFour + " " + stringSix + "     ";  break;
            case 2:  stringFour =  myNumeral[2]; stringSix =  myNumeral[31]; stringMin = "\n     " + stringFour + " " + stringSix + "     ";  break;
            case 3:  stringFour =  myNumeral[3]; stringSix =  myNumeral[31]; stringMin = "\n     " + stringFour + " " + stringSix + "     ";  break;
            case 4:  stringFour =  myNumeral[4]; stringSix =  myNumeral[31]; stringMin = "\n   " + stringFour + " " + stringSix + "    ";  break;
            case 5:  stringFour =  myNumeral[5]; stringSix =  myNumeral[29]; stringMin = "\n     " + stringFour + " " + stringSix + "     ";  break;
            case 6:  stringFour =  myNumeral[6]; stringSix =  myNumeral[29]; stringMin = "\n    " + stringFour + " " + stringSix + "     ";  break;
            case 7:  stringFour =  myNumeral[7]; stringSix =  myNumeral[29]; stringMin = "\n     " + stringFour + " " + stringSix + "     ";  break;
            case 8:  stringFour =  myNumeral[8]; stringSix =  myNumeral[29]; stringMin = "\n    " + stringFour + " " + stringSix + "    ";  break;
            case 9:  stringFour =  myNumeral[9]; stringSix =  myNumeral[29]; stringMin = "\n    " + stringFour + " " + stringSix + "    ";  break;
            case 10:  stringFour =  myNumeral[10]; stringSix =  myNumeral[29]; stringMin = "\n    " + stringFour + " " + stringSix + "    ";  break;
            case 11:  stringFour =  myNumeral[11]; stringSix =  myNumeral[29]; stringMin = "\n " + stringFour + " " + stringSix + "  ";  break;
            case 12:  stringFour =  myNumeral[12]; stringSix =  myNumeral[29]; stringMin = "\n  " + stringFour + " " + stringSix + "  ";  break;
            case 13:  stringFour =  myNumeral[13]; stringSix =  myNumeral[29]; stringMin = "\n  " + stringFour + " " + stringSix + "  ";  break;
            case 14:  stringFour =  myNumeral[14]; stringSix =  myNumeral[29]; stringMin = "\n " + stringFour + " " + stringSix + "  ";  break;
            case 15:  stringFour =  myNumeral[15]; stringSix =  myNumeral[29]; stringMin = "\n  " + stringFour + " " + stringSix + "  ";  break;
            case 16:  stringFour =  myNumeral[16]; stringSix =  myNumeral[29]; stringMin = "\n " + stringFour + " " + stringSix + "  ";  break;
            case 17:  stringFour =  myNumeral[17]; stringSix =  myNumeral[29]; stringMin = "\n  " + stringFour + " " + stringSix + "  ";  break;
            case 18:  stringFour =  myNumeral[18]; stringSix =  myNumeral[29]; stringMin = "\n " + stringFour + " " + stringSix + " ";  break;
            case 19:  stringFour =  myNumeral[19]; stringSix =  myNumeral[29]; stringMin = "\n " + stringFour + " " + stringSix + " ";  break;
            case 20:  stringFour =  myNumeral[20]; stringSix =  myNumeral[29]; stringMin = "\n   " + stringFour + " " + stringSix + "   ";  break;
            case 21:  stringFour =  myNumeral[20]; stringFive =  myNumeral[1];stringSix =  myNumeral[30]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix;  break;
            case 22:  stringFour =  myNumeral[20]; stringFive =  myNumeral[2];stringSix =  myNumeral[31]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 23:  stringFour =  myNumeral[20]; stringFive =  myNumeral[3];stringSix =  myNumeral[31]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 24:  stringFour =  myNumeral[20]; stringFive =  myNumeral[4];stringSix =  myNumeral[32]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 25:  stringFour =  myNumeral[20]; stringFive =  myNumeral[5];stringSix =  myNumeral[29]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 26:  stringFour =  myNumeral[20]; stringFive =  myNumeral[6];stringSix =  myNumeral[29]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix;  break;
            case 27:  stringFour =  myNumeral[20]; stringFive =  myNumeral[7];stringSix =  myNumeral[29]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 28:  stringFour =  myNumeral[20]; stringFive =  myNumeral[8];stringSix =  myNumeral[32]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 29:  stringFour =  myNumeral[20]; stringFive =  myNumeral[9];stringSix =  myNumeral[32]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 30:  stringFour =  myNumeral[21]; stringSix =  myNumeral[29]; stringMin = "\n   " + stringFour + " " + stringSix + "   ";  break;
            case 31:  stringFour =  myNumeral[21]; stringFive =  myNumeral[1];stringSix =  myNumeral[30]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix;  break;
            case 32:  stringFour =  myNumeral[21]; stringFive =  myNumeral[2];stringSix =  myNumeral[31]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 33:  stringFour =  myNumeral[21]; stringFive =  myNumeral[3];stringSix =  myNumeral[31]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 34:  stringFour =  myNumeral[21]; stringFive =  myNumeral[4];stringSix =  myNumeral[32]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 35:  stringFour =  myNumeral[21]; stringFive =  myNumeral[5];stringSix =  myNumeral[29]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 36:  stringFour =  myNumeral[21]; stringFive =  myNumeral[6];stringSix =  myNumeral[29]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix;  break;
            case 37:  stringFour =  myNumeral[21]; stringFive =  myNumeral[7];stringSix =  myNumeral[29]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 38:  stringFour =  myNumeral[21]; stringFive =  myNumeral[8];stringSix =  myNumeral[32]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 39:  stringFour =  myNumeral[21]; stringFive =  myNumeral[9];stringSix =  myNumeral[32]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 40:  stringFour =  myNumeral[22]; stringSix =  myNumeral[29]; stringMin = "\n    " + stringFour + " " + stringSix + "     ";  break;
            case 41:  stringFour =  myNumeral[22]; stringFive =  myNumeral[1];stringSix =  myNumeral[30]; stringMin = "\n " + stringFour + " " + stringFive + " " + stringSix + "  ";  break;
            case 42:  stringFour =  myNumeral[22]; stringFive =  myNumeral[2];stringSix =  myNumeral[31]; stringMin = "\n  " + stringFour + " " + stringFive + " " + stringSix + "  ";  break;
            case 43:  stringFour =  myNumeral[22]; stringFive =  myNumeral[3];stringSix =  myNumeral[31]; stringMin = "\n  " + stringFour + " " + stringFive + " " + stringSix + "  ";  break;
            case 44:  stringFour =  myNumeral[22]; stringFive =  myNumeral[4];stringSix =  myNumeral[31]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 45:  stringFour =  myNumeral[22]; stringFive =  myNumeral[5];stringSix =  myNumeral[29]; stringMin = "\n  " + stringFour + " " + stringFive + " " + stringSix + "  ";  break;
            case 46:  stringFour =  myNumeral[22]; stringFive =  myNumeral[6];stringSix =  myNumeral[29]; stringMin = "\n " + stringFour + " " + stringFive + " " + stringSix + "  ";  break;
            case 47:  stringFour =  myNumeral[22]; stringFive =  myNumeral[7];stringSix =  myNumeral[29]; stringMin = "\n  " + stringFour + " " + stringFive + " " + stringSix + "  ";  break;
            case 48:  stringFour =  myNumeral[22]; stringFive =  myNumeral[8];stringSix =  myNumeral[29]; stringMin = "\n " + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 49:  stringFour =  myNumeral[22]; stringFive =  myNumeral[9];stringSix =  myNumeral[29]; stringMin = "\n " + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 50:  stringFour =  myNumeral[23]; stringSix =  myNumeral[29]; stringMin = "\n  " + stringFour + " " + stringSix + "   ";  break;
            case 51:  stringFour =  myNumeral[23]; stringFive =  myNumeral[1];stringSix =  myNumeral[32]; stringMin = "\n " + stringFour + " " + stringFive + " " + stringSix + " ";  break;
            case 52:  stringFour =  myNumeral[23]; stringFive =  myNumeral[2];stringSix =  myNumeral[31]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix;  break;
            case 53:  stringFour =  myNumeral[23]; stringFive =  myNumeral[3];stringSix =  myNumeral[31]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix;  break;
            case 54:  stringFour =  myNumeral[23]; stringFive =  myNumeral[4];stringSix =  myNumeral[32]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix;  break;
            case 55:  stringFour =  myNumeral[23]; stringFive =  myNumeral[5];stringSix =  myNumeral[29]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix;  break;
            case 56:  stringFour =  myNumeral[23]; stringFive =  myNumeral[6];stringSix =  myNumeral[32]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix  + " ";  break;
            case 57:  stringFour =  myNumeral[23]; stringFive =  myNumeral[7];stringSix =  myNumeral[29]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix;  break;
            case 58:  stringFour =  myNumeral[23]; stringFive =  myNumeral[8];stringSix =  myNumeral[32]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix;  break;
            case 59:  stringFour =  myNumeral[23]; stringFive =  myNumeral[9];stringSix =  myNumeral[32]; stringMin = "\n" + stringFour + " " + stringFive + " " + stringSix;  break;
        }

        // if(PrevState){
        //     PrevState = false;
        //     // Serial.print("\f");
        // }
        Serial.println(stringHour);   //печать первой строки
        Serial.print(stringMin);      // печать второй строки
        delay(1000);
    }
    else 
    {         
        // agod = (14 - mec) / 12;
        // ygod = god - agod;
        // mgod = mec + 12 * agod - 2;           
        // dday = ( den + ygod + (ygod / 4) - ( ygod / 100 ) + (ygod / 400 ) + (31 * mgod) / 12)  %7;               // вычисление дня недел
        

        stringHourdd = chas; 
        stringMindd = minuta; 
        stringSecdd = sekunda; 
        stringDatyd = denec; 
        stringGod = yar; 
        stringGodn = myNumeral[52];

        
        switch (dday) 
        {                                                                       // блок имени дня недели
            case 0:   stringdday = myNumeral[39]; break;
            case 1:   stringdday = myNumeral[33]; break;
            case 2:   stringdday = stringddayy + "    "; stringddayy =myNumeral[34]; break;
            case 3:   stringdday = stringddayy + "      "; stringddayy = myNumeral[35]; break;
            case 4:   stringdday = stringddayy + "    "; stringddayy = myNumeral[36]; break;
            case 5:   stringdday = stringddayy + "    "; stringddayy = myNumeral[37]; break;
            case 6:   stringdday = stringddayy + "    "; stringddayy = myNumeral[38]; break;
        }
        
        switch (mesyac) 
        {                                                                  // блок имени месяца
            case 1:   stringdmonth = stringdmonthh + "   "; stringdmonthh = myNumeral[40]; break;
            case 2:   stringdmonth = stringdmonthh + "  "; stringdmonthh = myNumeral[41]; break;
            case 3:   stringdmonth = stringdmonthh + "    "; stringdmonthh = myNumeral[42]; break;
            case 4:   stringdmonth = stringdmonthh + "   "; stringdmonthh = myNumeral[43]; break;
            case 5:   stringdmonth = stringdmonthh + "      "; stringdmonthh = myNumeral[44]; break;
            case 6:   stringdmonth = stringdmonthh + "     "; stringdmonthh = myNumeral[45]; break;
            case 7:   stringdmonth = stringdmonthh + "     "; stringdmonthh = myNumeral[46]; break;
            case 8:   stringdmonth = stringdmonthh + "  "; stringdmonthh = myNumeral[47]; break;
            case 9:   stringdmonth = stringdmonthh + " "; stringdmonthh = myNumeral[48]; break;
            case 10:   stringdmonth = stringdmonthh + "  "; stringdmonthh = myNumeral[49]; break;
            case 11:   stringdmonth = stringdmonthh + "   "; stringdmonthh = myNumeral[50]; break;
            case 12:   stringdmonth = stringdmonthh + "  "; stringdmonthh = myNumeral[51]; break;
        }
        
        if (chas < 10)
        {   // это нолики перед часами до 10
            stringHourd = '0' + stringHourdd;
        }
        else
        {
            stringHourd = stringHourdd;
        }
        
        if (minuta < 10)
        {   // это нолики перед минутами до 10
            stringMind = '0' + stringMindd;
        }
        else
        {
            stringMind = stringMindd;
        }
        
        if (sekunda < 10)
        {   // это нолики перед секундами до 10
            stringSecd = '0' + stringSecdd;
        }
        else
        {
            stringSecd = stringSecdd;
        }        
        
        if (denec < 10)
        {   // это нолики перед датой до 10
            stringDaty = '0' + stringDatyd;
        }
        else
        {
            stringDaty = stringDatyd;
        }
        
        stringPerd = stringHourd + ':' + stringMind + ':' + stringSecd + " " + stringdday;       // собираем первую строку
        stringVtor = "\r\n" + stringDaty + " " + stringdmonth +  stringGod + " " + stringGodn;   // собираем вторую строку

        // if(!PrevState){
        //     PrevState = true;
        //     // Serial.print("\f");
        // }
        
        Serial.println(stringPerd);  // печатаем первую строку
        Serial.print(stringVtor);    // а это вторую
        delay(500);
    }
}
  
