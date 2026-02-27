#include <Wire.h>
#include <RTClib.h>
#include "TM1628.h"
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <time.h>

// ================== Пины TM1628 (те же, что в объекте dvdLED) ==================
static const uint8_t PIN_DIO = D6;
static const uint8_t PIN_CLK = D5;
static const uint8_t PIN_STB = D7;

// ================== TM1628 / RTC ==================
TM1628 dvdLED(PIN_DIO, PIN_CLK, PIN_STB, 5);
RTC_DS3231 rtc;

// ================== РЕЖИМЫ ==================
enum Mode : uint8_t { MODE_TIME=0, MODE_DATE=1, MODE_YEAR=2 };
Mode mode = MODE_TIME;

const uint32_t T_TIME_MS = 120000; // 2 минуты
const uint32_t T_DATE_MS = 4000;
const uint32_t T_YEAR_MS = 4000;
uint32_t modeT0 = 0;

// ================== Яркость (0..7) ==================
static const uint8_t NORMAL_BRIGHTNESS = 7;

// ================== Низкоуровневое управление яркостью TM1628 ==================
// (потому что в вашей либе intensity не используется)
inline void tmWriteByteLSB(uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_DIO, (data & 0x01) ? HIGH : LOW);
    data >>= 1;
    digitalWrite(PIN_CLK, HIGH);
  }
}

void tmSendCommand(uint8_t cmd) {
  pinMode(PIN_DIO, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_STB, OUTPUT);

  digitalWrite(PIN_STB, LOW);
  tmWriteByteLSB(cmd);
  digitalWrite(PIN_STB, HIGH);
}

// brightness: 0..7
void tmSetBrightness(uint8_t brightness) {
  tmSendCommand(0x88 | (brightness & 0x07)); // display ON + brightness
}

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

// "Good"
const uint8_t PAT_G = (SEG_A | SEG_F | SEG_G | SEG_E | SEG_C | SEG_D);
const uint8_t PAT_o = (SEG_C | SEG_D | SEG_E | SEG_G);
const uint8_t PAT_d = (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G);

// ================== Дисплей-хелперы ==================
void setColon(bool on){ dvdLED.setSegments(on ? (1 << 3) : 0, 4); } // pos4, bit3
void blankAllDigits(){ for(uint8_t p=0; p<4; ++p) dvdLED.setSegments(0, p); }

void render4Digits(const uint8_t d[4]) {
  dvdLED.setSegments(segPattern(d[0]), 0);
  dvdLED.setSegments(segPattern(d[1]), 1);
  dvdLED.setSegments(segPattern(d[2]), 2);
  dvdLED.setSegments(segPattern(d[3]), 3);
}

void makeTimeDigits(const DateTime& now, uint8_t out[4]){
  out[0] = (uint8_t)((now.hour()/10)%10);
  out[1] = (uint8_t)( now.hour()%10);
  out[2] = (uint8_t)((now.minute()/10)%10);
  out[3] = (uint8_t)( now.minute()%10);
}

void makeDateDigits(const DateTime& now, uint8_t out[4]){
  out[0] = (uint8_t)((now.day()/10)%10);
  out[1] = (uint8_t)( now.day()%10);
  out[2] = (uint8_t)((now.month()/10)%10);
  out[3] = (uint8_t)( now.month()%10);
}

void makeYearDigits(const DateTime& now, uint8_t out[4]){
  uint16_t y = now.year();
  out[0] = (uint8_t)((y/1000)%10);
  out[1] = (uint8_t)((y/100)%10);
  out[2] = (uint8_t)((y/10)%10);
  out[3] = (uint8_t)( y%10);
}

// ================== Двоеточие через Ticker ==================
Ticker colonTicker;
volatile bool colonState = false;
volatile bool colonDirty = false;

void IRAM_ATTR colonTickISR(){
  colonState = !colonState;
  colonDirty = true;
}

void startColonTickerForTime(){
  colonTicker.detach();
  colonTicker.attach(0.25, colonTickISR);
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

// ================== WiFi анимация + GOOD ==================
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

// ================== АНИМАЦИЯ (как у вас, но для TM1628) ==================
struct Anim {
  bool     active = false;
  uint8_t  step   = 0;        // 0..9
  uint32_t tPrev  = 0;
  uint16_t period = 50;       // мс на кадр
  uint8_t  mask   = 0;        // bit0..bit3 какие разряды меняются
  uint8_t  target[4];         // новые цифры
} anim;

uint8_t lastDigits[4] = { 0xFF, 0xFF, 0xFF, 0xFF }; // что было на экране

void startAnim(uint8_t changedMask, const uint8_t newDigits[4]){
  anim.active = true;
  anim.step   = 0;
  anim.tPrev  = millis();
  anim.mask   = changedMask;
  for(int i=0;i<4;i++) anim.target[i] = newDigits[i];
}

// Плавное затухание/появление яркости (глобально на весь TM1628)
void renderAnimFrameFade(){
  // как в вашем коде:
  // 0..4  fadeOut: 7,5,3,1,0
  // 5..9  fadeIn : 0,2,4,6,7
  uint8_t br = NORMAL_BRIGHTNESS;
  if(anim.step <= 4){
    static const uint8_t fadeOut[5] = { 7, 5, 3, 1, 0 };
    br = fadeOut[anim.step];
  }else{
    static const uint8_t fadeIn[5]  = { 0, 2, 4, 6, 7 };
    br = fadeIn[anim.step - 5];
  }
  tmSetBrightness(br);

  // Формируем цифры кадра:
  // меняющиеся разряды: до 4 шага показываем старые, после 5 шага показываем новые
  uint8_t frameDigits[4];
  for(int i=0;i<4;i++){
    if(anim.mask & (1<<i)){
      frameDigits[i] = (anim.step <= 4) ? lastDigits[i] : anim.target[i];
      if(frameDigits[i] == 0xFF) frameDigits[i] = anim.target[i]; // подстраховка
    }else{
      frameDigits[i] = anim.target[i];
    }
  }
  render4Digits(frameDigits);
}

void updateAnimIfActive(){
  if(!anim.active) return;

  // чтобы двоеточие продолжало жить
  applyColonIfDirty();

  uint32_t nowMs = millis();
  if(nowMs - anim.tPrev >= anim.period){
    anim.tPrev = nowMs;
    anim.step++;

    if(anim.step >= 10){
      anim.active = false;
      tmSetBrightness(NORMAL_BRIGHTNESS);
      render4Digits(anim.target);
      for(int i=0;i<4;i++) lastDigits[i] = anim.target[i];
      return;
    }
  }
  renderAnimFrameFade();
}

// ================== Переходы режимов (анимация при входе в ДАТУ/ГОД) ==================
void enterMode(Mode m, bool animateOnEnter){
  anim.active = false;

  mode = m;
  modeT0 = millis();

  DateTime now = rtc.now();
  uint8_t target[4];

  if(mode == MODE_TIME){
    startColonTickerForTime();
    setColon(colonState);

    makeTimeDigits(now, target);

    tmSetBrightness(NORMAL_BRIGHTNESS);
    render4Digits(target);
    for(int i=0;i<4;i++) lastDigits[i] = target[i];
    return;
  }

  if(mode == MODE_DATE){
    stopColonTickerAndSet(true);
    makeDateDigits(now, target);
  }else{ // MODE_YEAR
    stopColonTickerAndSet(false);
    makeYearDigits(now, target);
  }

  if(animateOnEnter && lastDigits[0] != 0xFF){
    startAnim(0x0F, target);      // анимируем все 4 разряда
    renderAnimFrameFade();        // первый кадр сразу
  }else{
    tmSetBrightness(NORMAL_BRIGHTNESS);
    render4Digits(target);
    for(int i=0;i<4;i++) lastDigits[i] = target[i];
  }
}

void nextMode(){
  Mode next = (Mode)((mode + 1) % 3);
  bool animate = (next == MODE_DATE || next == MODE_YEAR); // анимация при переходе на дату/год
  enterMode(next, animate);
}

// ================== SETUP / LOOP ==================
#define ADJUST_AT_BOOT 0

void setup(){
  delay(2000);
  Wire.begin();

  // подготовка пинов яркости/команд
  pinMode(PIN_DIO, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_STB, OUTPUT);
  tmSetBrightness(NORMAL_BRIGHTNESS);

  setColon(false);
  blankAllDigits();

  if(!rtc.begin()){
    uint8_t d[4] = {8,8,8,8};
    render4Digits(d);
    setColon(true);
    while(1){}
  }

#if ADJUST_AT_BOOT
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
#endif

  enterMode(MODE_TIME, false);

  // WiFi + NTP (если нужно)
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
  }
}

void loop(){
  uint32_t ms = millis();

  uint32_t dwell = (mode==MODE_TIME) ? T_TIME_MS :
                   (mode==MODE_DATE) ? T_DATE_MS : T_YEAR_MS;
  if(ms - modeT0 >= dwell) nextMode();

  applyColonIfDirty();

  updateAnimIfActive();
  if(anim.active){
    delay(5);
    return;
  }

  static uint32_t t250 = 0;
  if(ms - t250 >= 250){
    t250 += 250;

    DateTime now = rtc.now();

    if(mode == MODE_TIME){
      uint8_t newDigits[4];
      makeTimeDigits(now, newDigits);

      // маска изменившихся разрядов
      uint8_t mask = 0;
      for(int i=0;i<4;i++){
        if(lastDigits[i] == 0xFF || newDigits[i] != lastDigits[i]) mask |= (1<<i);
      }

      if(mask){
        startAnim(mask, newDigits);
        renderAnimFrameFade();
      }else{
        render4Digits(lastDigits);
      }
    }
    else if(mode == MODE_DATE){
      uint8_t d[4];
      makeDateDigits(now, d);
      setColon(true);
      tmSetBrightness(NORMAL_BRIGHTNESS);
      render4Digits(d);
      for(int i=0;i<4;i++) lastDigits[i] = d[i];
    }
    else { // MODE_YEAR
      uint8_t d[4];
      makeYearDigits(now, d);
      setColon(false);
      tmSetBrightness(NORMAL_BRIGHTNESS);
      render4Digits(d);
      for(int i=0;i<4;i++) lastDigits[i] = d[i];
    }
  }

  delay(5);
}