#include <Arduino.h>

// Nano 16 МГц. D8=PB0, D9=PB1
#define F_LCD_HZ 32  // частота «переворота» сегмента (попробуйте 20..80)

constexpr uint32_t T2_PRESC = 1024;
constexpr uint8_t  OCR2A_VAL =
  (uint32_t)(F_CPU / (T2_PRESC * 2UL * F_LCD_HZ)) - 1;

ISR(TIMER2_COMPA_vect) {
  // Переключаем PB0 и PB1 одновременно (антифаза сохраняется)
  PINB = _BV(PINB0) | _BV(PINB1);
}

void setup() {
  DDRB |= _BV(DDB0) | _BV(DDB1);   // D8,D9 — выходы
  PORTB |=  _BV(PB0);              // старт: D8=1
  PORTB &= ~_BV(PB1);              //       D9=0
  cli();
  TCCR2A = _BV(WGM21);                                 // CTC
  TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);          // /1024
  OCR2A  = OCR2A_VAL;
  TCNT2  = 0;
  TIFR2  = _BV(OCF2A);
  TIMSK2 = _BV(OCIE2A);
  sei();
}

void loop() { /* таймер всё делает сам */ }



// #include <Arduino.h>

// // ===== впишите свои пины =====
// const uint8_t COM_PINS[3] = { 2, 3, 4 };                    // 3 общих линии
// const uint8_t SEG_PINS[8] = { 5, 6, 7, 8, 9, 10, 11, 12 };  // 8 сегментов (порядок любой)

// // Частота «переворота» включенного сегмента (30..80 Гц обычно ок)
// #define AC_HZ 64

// // Timer2 CTC, делитель /1024 → прерывание каждые 1/(2*AC_HZ)
// constexpr uint8_t OCR2A_VAL = (uint32_t)(F_CPU / (1024UL * 2UL * AC_HZ)) - 1;

// volatile bool     phase      = false; // 0/1 — текущая фаза
// volatile uint8_t  activeCom  = 0;     // какой COM демонстрируем (0..2)
// volatile uint8_t  segMask    = 0;     // какой SEG включен (бит 0..7), по одному

// static inline void comToHiZ(uint8_t idx) 
// {
//   pinMode(COM_PINS[idx], INPUT);      // Hi-Z
//   digitalWrite(COM_PINS[idx], LOW);   // без подтяжки
// }
// static inline void comToOutput(uint8_t idx) 
// {
//   pinMode(COM_PINS[idx], OUTPUT);
//   digitalWrite(COM_PINS[idx], phase); // сразу выставим текущую фазу
// }

// ISR(TIMER2_COMPA_vect) 
// {
//   phase = !phase;

//   // Активный COM — текущая фаза
//   digitalWrite(COM_PINS[activeCom], phase);

//   // Все SEG в «фазу» (выключено)
//   for (uint8_t s=0; s<8; ++s) digitalWrite(SEG_PINS[s], phase);

//   // Выбранный SEG — антифаза (включено)
//   for (uint8_t s=0; s<8; ++s)
//     if (segMask & (1U<<s)) digitalWrite(SEG_PINS[s], !phase);
// }

// void setupTimer2() 
// {
//   cli();
//   TCCR2A = _BV(WGM21);                         // CTC
//   TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);  // /1024
//   OCR2A  = OCR2A_VAL;
//   TCNT2  = 0;
//   TIFR2  = _BV(OCF2A);
//   TIMSK2 = _BV(OCIE2A);
//   sei();
// }

// void selectCom(uint8_t com) 
// {
//   // Остальные COM — Hi-Z, выбранный — OUTPUT
//   for (uint8_t i=0; i<3; ++i) 
//   {
//     if (i == com) continue;
//     comToHiZ(i);
//   }
//   activeCom = com;
//   comToOutput(com);
// }

// void allOff() 
// {
//   segMask = 0;
//   // Активный COM в текущей фазе, SEG — в фазу (нулевая разность)
//   digitalWrite(COM_PINS[activeCom], phase);
//   for (uint8_t s=0; s<8; ++s) digitalWrite(SEG_PINS[s], phase);
// }

// void setup() 
// {
//   // SEG — выходы, в стартовую фазу
//   for (uint8_t s=0; s<8; ++s) { pinMode(SEG_PINS[s], OUTPUT); digitalWrite(SEG_PINS[s], phase); }
//   // COM — пока Hi-Z
//   for (uint8_t i=0; i<3; ++i) comToHiZ(i);

//   setupTimer2();

//   const unsigned long HOLD_MS = 2000; // 2 секунды на сегмент

//   // COM0 → SEG0..7, затем COM1 → SEG0..7, затем COM2 → SEG0..7
//   for (uint8_t com=0; com<3; ++com) {
//     selectCom(com);
//     delay(50); // небольшая стабилизация

//     for (uint8_t seg=0; seg<8; ++seg) {
//       segMask = (1U << seg);  // включить только один сегмент
//       delay(HOLD_MS);
//       // при желании гасить между сегментами:
//       // segMask = 0; delay(200);
//     }
//     allOff();                  // погасить перед переходом к следующему COM
//     delay(300);
//   }

//   allOff(); // финально всё погасить (или оставьте нужный вид)
// }

// void loop() {
//   // пусто — тест завершён
// }