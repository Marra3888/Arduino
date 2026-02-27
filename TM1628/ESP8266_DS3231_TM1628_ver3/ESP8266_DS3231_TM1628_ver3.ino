#include <Wire.h>
#include <RTClib.h>
#include "TM1628.h"
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <time.h>

// ================== TM1628 / RTC ==================
// TM1628: DIO=D6, CLK=D5, STB=D7  (ESP8266)
TM1628 dvdLED(D6, D5, D7, 5);
RTC_DS3231 rtc;

// ================== РЕЖИМЫ (ВАЖНО: ДО ФУНКЦИЙ, ГДЕ ИСПОЛЬЗУЕТСЯ Mode) ==================
enum Mode : uint8_t { MODE_TIME=0, MODE_DATE=1, MODE_YEAR=2 };
Mode mode = MODE_TIME;

const uint32_t T_TIME_MS = 120000; // 2 минуты
const uint32_t T_DATE_MS = 4000;
const uint32_t T_YEAR_MS = 4000;
uint32_t modeT0 = 0;

// ================== ШРИФТ ==================
const uint8_t PROGMEM NUMBER_FONT[] = {
  0b01111110, // 0
  0b01010000, // 1
  0b10111100, // 2
  0b11110100, // 3
  0b11010010, // 4
  0b11100110, // 5
  0b11101110, // 6
  0b01010100, // 7
  0b11111110, // 8
  0b11110110, // 9
  0b11011110, // A
  0b11101010, // b
  0b00101110, // C
  0b11111000, // d
  0b10101110, // E
  0b10001110  // F
};
inline uint8_t segPattern(uint8_t d){ return pgm_read_byte(&NUMBER_FONT[d & 0x0F]); }

// ================== МАСКИ СЕГМЕНТОВ (под вашу разводку) ==================
const uint8_t SEG_A = (1 << 2);
const uint8_t SEG_B = (1 << 4);
const uint8_t SEG_C = (1 << 6);
const uint8_t SEG_D = (1 << 5);
const uint8_t SEG_E = (1 << 3);
const uint8_t SEG_F = (1 << 1);
const uint8_t SEG_G = (1 << 7);

// паттерны "Good"
const uint8_t PAT_G = (SEG_A | SEG_F | SEG_G | SEG_E | SEG_C | SEG_D);
const uint8_t PAT_o = (SEG_C | SEG_D | SEG_E | SEG_G);
const uint8_t PAT_d = (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G);

// ================== ДВОЕТОЧИЕ ==================
Ticker colonTicker;
volatile bool colonState = false;
volatile bool colonDirty = false;

void IRAM_ATTR colonTickISR(){
  colonState = !colonState;
  colonDirty = true;
}

void setColon(bool on){ dvdLED.setSegments(on ? (1 << 3) : 0, 4); } // pos4, bit3
void startColonTickerForTime(){
  colonTicker.detach();
  colonTicker.attach(0.25, colonTickISR); // 2 Гц (переключение каждые 250мс)
}
void stopColonTickerAndSet(bool on){
  colonTicker.detach();
  setColon(on);
}

void applyColonIfDirty(){
  if(!colonDirty) return;
  colonDirty = false;
  if(mode == MODE_TIME){
    dvdLED.setSegments(colonState ? (1<<3) : 0, 4);
  }
}

// ================== ВЫВОД 4 ЦИФР ==================
void blankAllDigits(){ for(uint8_t p=0; p<4; ++p) dvdLED.setSegments(0, p); }

void render4Digits(const uint8_t d[4]){
  dvdLED.setSegments(segPattern(d[0]), 0);
  dvdLED.setSegments(segPattern(d[1]), 1);
  dvdLED.setSegments(segPattern(d[2]), 2);
  dvdLED.setSegments(segPattern(d[3]), 3);
}

void showDate(uint8_t day, uint8_t mon){
  setColon(true);
  uint8_t d[4] = {
    (uint8_t)((day/10)%10),
    (uint8_t)(day%10),
    (uint8_t)((mon/10)%10),
    (uint8_t)(mon%10)
  };
  render4Digits(d);
}

void showYear(uint16_t year){
  setColon(false);
  uint8_t d[4] = {
    (uint8_t)((year/1000)%10),
    (uint8_t)((year/100)%10),
    (uint8_t)((year/10)%10),
    (uint8_t)(year%10)
  };
  render4Digits(d);
}

// ================== WiFi “бегущий G” + Good ==================
inline void showGat(uint8_t pos){
  blankAllDigits();
  dvdLED.setSegments(SEG_G, pos);
}

bool waitWifiWithGRun(uint32_t durationMs){
  const uint16_t stepMs = 120;
  uint8_t pos = 0;
  int8_t dir = +1;
  uint32_t t0 = millis(), tAnim = 0;

  while(WiFi.status() != WL_CONNECTED && (millis() - t0) < durationMs){
    if(millis() - tAnim >= stepMs){
      tAnim = millis();
      showGat(pos);
      pos += dir;
      if(pos >= 3) { pos = 3; dir = -1; }
      if(pos == 0) { dir = +1; }
    }
    delay(1);
    yield();
  }
  return WiFi.status() == WL_CONNECTED;
}

void showGOOD(uint16_t holdMs = 1500){
  blankAllDigits();
  dvdLED.setSegments(PAT_G, 0);
  dvdLED.setSegments(PAT_o, 1);
  dvdLED.setSegments(PAT_o, 2);
  dvdLED.setSegments(PAT_d, 3);
  setColon(false);
  delay(holdMs);
}

// ================== NTP -> RTC ==================
bool adjustRtcFromNtp(long tzOffsetSec = 2*3600, long dstOffsetSec = 0){
  configTime(tzOffsetSec, dstOffsetSec, "pool.ntp.org", "time.nist.gov", "time.google.com");

  const time_t VALID_EPOCH = 1609459200; // 2021-01-01
  uint32_t t0 = millis();
  time_t now = 0;

  while((now = time(nullptr)) < VALID_EPOCH && (millis() - t0) < 15000){
    delay(200);
    yield();
  }
  if(now < VALID_EPOCH) return false;

  struct tm tmLocal;
  localtime_r(&now, &tmLocal);

  rtc.adjust(DateTime(
    tmLocal.tm_year + 1900,
    tmLocal.tm_mon + 1,
    tmLocal.tm_mday,
    tmLocal.tm_hour,
    tmLocal.tm_min,
    tmLocal.tm_sec
  ));
  return true;
}

// ================== АНИМАЦИЯ (как в вашем TM1637 коде) ==================
struct Anim {
  bool     active = false;
  uint8_t  step   = 0;        // 0..9
  uint32_t tPrev  = 0;
  uint16_t period = 60;       // 10 кадров ~600мс
  uint8_t  mask   = 0;        // bit0..bit3 какие разряды меняются
  uint8_t  target[4];         // новые цифры 0..9
} anim;

// последние показанные цифры (для анимации)
uint8_t lastDigits[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

void startAnim(uint8_t changedMask, const uint8_t newDigits[4]){
  anim.active = true;
  anim.step   = 0;
  anim.tPrev  = millis();
  anim.mask   = changedMask;
  for(int i=0;i<4;i++) anim.target[i] = newDigits[i];
}

// кадр: мигает старая -> пусто -> мигает новая -> фикс
void renderAnimFrame(){
  for(int pos=0; pos<4; ++pos){
    uint8_t out = 0xFF; // 0xFF = пусто

    if(anim.mask & (1<<pos)){
      if(anim.step <= 3){
        bool on = (anim.step % 2 == 0);
        out = on ? lastDigits[pos] : 0xFF;
      }else if(anim.step <= 5){
        out = 0xFF;
      }else{
        if(anim.step < 9){
          bool on = (anim.step % 2 == 0);
          out = on ? anim.target[pos] : 0xFF;
        }else{
          out = anim.target[pos];
        }
      }
    }else{
      out = anim.target[pos];
    }

    if(out == 0xFF) dvdLED.setSegments(0, pos);
    else           dvdLED.setSegments(segPattern(out), pos);
  }
}

void updateAnimIfActive(){
  if(!anim.active) return;

  uint32_t nowMs = millis();
  if(nowMs - anim.tPrev >= anim.period){
    anim.tPrev = nowMs;
    anim.step++;

    if(anim.step >= 10){
      anim.active = false;
      render4Digits(anim.target);
      for(int i=0;i<4;i++) lastDigits[i] = anim.target[i];
      return;
    }
  }
  renderAnimFrame();
}

// ================== ПЕРЕКЛЮЧЕНИЕ РЕЖИМОВ (анимация при входе в дату/год) ==================
void enterMode(Mode m, bool animateOnEnter){
  anim.active = false;

  mode = m;
  modeT0 = millis();

  DateTime now = rtc.now();
  uint8_t target[4];

  if(mode == MODE_TIME){
    startColonTickerForTime();
    setColon(colonState);

    target[0] = (uint8_t)((now.hour()/10)%10);
    target[1] = (uint8_t)( now.hour()%10);
    target[2] = (uint8_t)((now.minute()/10)%10);
    target[3] = (uint8_t)( now.minute()%10);

    render4Digits(target);
    for(int i=0;i<4;i++) lastDigits[i] = target[i];
    return;
  }

  if(mode == MODE_DATE){
    stopColonTickerAndSet(true);
    target[0] = (uint8_t)((now.day()/10)%10);
    target[1] = (uint8_t)( now.day()%10);
    target[2] = (uint8_t)((now.month()/10)%10);
    target[3] = (uint8_t)( now.month()%10);
  }else{ // MODE_YEAR
    stopColonTickerAndSet(false);
    uint16_t y = now.year();
    target[0] = (uint8_t)((y/1000)%10);
    target[1] = (uint8_t)((y/100)%10);
    target[2] = (uint8_t)((y/10)%10);
    target[3] = (uint8_t)( y%10);
  }

  // АНИМАЦИЯ ПРИ ПЕРЕХОДЕ НА ДАТУ/ГОД
  if(animateOnEnter && lastDigits[0] != 0xFF){
    startAnim(0x0F, target);  // анимируем все 4 разряда
    renderAnimFrame();
  }else{
    render4Digits(target);
    for(int i=0;i<4;i++) lastDigits[i] = target[i];
  }
}

void nextMode(){
  Mode next = (Mode)((mode + 1) % 3);
  bool animate = (next == MODE_DATE || next == MODE_YEAR);
  enterMode(next, animate);
}

// ================== SETUP / LOOP ==================
#define ADJUST_AT_BOOT 0

void setup(){
  delay(2000);
  Wire.begin(); // ESP8266: SDA=D2, SCL=D1 по умолчанию

  setColon(false);
  blankAllDigits();

  if(!rtc.begin()){
    // 8888 как ошибка
    uint8_t d[4] = {8,8,8,8};
    render4Digits(d);
    setColon(true);
    while(1){}
  }

#if ADJUST_AT_BOOT
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
#endif

  // стартуем с времени
  enterMode(MODE_TIME, false);

  // WiFi + NTP (по желанию)
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);

  const char* ssid[]     = {"TP-LINK_333856", "TP-Link_22DC"};
  const char* password[] = {"",               "Mar4enko2704"};
  const int numNetworks = 2;

  bool connected = false;
  for(int i=0; i<numNetworks && !connected; ++i){
    WiFi.disconnect(true);
    delay(50);
    WiFi.begin(ssid[i], password[i]);
    connected = waitWifiWithGRun(10000UL);
  }

  if(connected){
    showGOOD(1500);

    if(!adjustRtcFromNtp(2*3600 /*TZ*/, 0 /*DST*/)){
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    enterMode(mode, false);
  }else{
    // "oo" если не подключились
    blankAllDigits();
    dvdLED.setSegments(PAT_o, 2);
    dvdLED.setSegments(PAT_o, 3);
    setColon(false);
    delay(500);

    enterMode(MODE_TIME, false);
  }
}

void loop(){
  uint32_t ms = millis();

  // переключение режимов по времени
  uint32_t dwell = (mode==MODE_TIME) ? T_TIME_MS :
                   (mode==MODE_DATE) ? T_DATE_MS : T_YEAR_MS;
  if(ms - modeT0 >= dwell) nextMode();

  applyColonIfDirty();

  // если идёт анимация — крутим и выходим
  updateAnimIfActive();
  if(anim.active){
    delay(5);
    return;
  }

  // периодическое обновление данных
  static uint32_t t250 = 0;
  if(ms - t250 >= 250){
    t250 += 250;

    DateTime now = rtc.now();

    if(mode == MODE_TIME){
      uint8_t newDigits[4] = {
        (uint8_t)((now.hour()/10)%10),
        (uint8_t)( now.hour()%10),
        (uint8_t)((now.minute()/10)%10),
        (uint8_t)( now.minute()%10)
      };

      // какие разряды изменились
      uint8_t mask = 0;
      for(int i=0;i<4;i++){
        if(lastDigits[i] == 0xFF || newDigits[i] != lastDigits[i]) mask |= (1<<i);
      }

      if(mask){
        startAnim(mask, newDigits);
        renderAnimFrame();
      }else{
        render4Digits(lastDigits);
      }
    }
    else if(mode == MODE_DATE){
      showDate(now.day(), now.month());
      // фиксируем lastDigits (чтобы переход DATE->YEAR анимировался корректно)
      lastDigits[0] = (uint8_t)((now.day()/10)%10);
      lastDigits[1] = (uint8_t)( now.day()%10);
      lastDigits[2] = (uint8_t)((now.month()/10)%10);
      lastDigits[3] = (uint8_t)( now.month()%10);
    }
    else { // MODE_YEAR
      showYear(now.year());
      uint16_t y = now.year();
      lastDigits[0] = (uint8_t)((y/1000)%10);
      lastDigits[1] = (uint8_t)((y/100)%10);
      lastDigits[2] = (uint8_t)((y/10)%10);
      lastDigits[3] = (uint8_t)( y%10);
    }
  }

  delay(5);
}