// #ifndef _AnalogKey_h
// #define _AnalogKey_h

// #include <Arduino.h>

// #define _AKEY_PERIOD 40   // период опроса в мс

// template <uint8_t PIN, uint8_t AMOUNT, int16_t* S_PTR = nullptr>
// class AnalogKey {
// public:
//     AnalogKey() {
//         if (S_PTR != nullptr) signals = S_PTR;  // Используем внешний массив
//     }

//     void attach(uint8_t num, int value) {
//         if (num >= AMOUNT || S_PTR != nullptr) return;
//         signals[num] = value;
//     }

//     void setWindow(int window) {
//         _window = window / 2;
//     }
    
//     bool status(uint8_t num) {
//         if (millis() - tmr >= _AKEY_PERIOD) {
//             tmr = millis();
//             int16_t thisRead = analogRead(PIN);
//             _ready = (abs(thisRead - _lastRead) < _window);
//             _lastRead = thisRead;        
//         }
//         return check(num);
//     }
    
//     int pressed() {
//         status(0);
//         for (uint8_t i = 0; i < AMOUNT; i++) {
//             if (check(i)) return i;
//         }
//         return -1;
//     }

// private:
//     bool check(int i) {
//         int16_t sig = signals[i];
//         return (_ready && (_lastRead > sig - _window) && (_lastRead < sig + _window));
//     }
    
//     int16_t signals[AMOUNT] = {0};  // Внутренний массив по умолчанию
//     int16_t* signals = S_PTR;       // Указатель на массив сигналов
//     int16_t _lastRead = 0;
//     int16_t _window = 20;
//     bool _ready = false;
//     uint32_t tmr = 0;
// };

// #endif

#ifndef _AnalogKey_h
#define _AnalogKey_h

#include <Arduino.h>

#define _AKEY_PERIOD 40   // период опроса в мс

template <uint8_t PIN, uint8_t AMOUNT, int16_t* S_PTR = nullptr>
class AnalogKey {
public:
    AnalogKey() {
        // Если предоставлен внешний массив S_PTR, копируем его значения
        if (S_PTR != nullptr) {
            for (uint8_t i = 0; i < AMOUNT; i++) {
                signals[i] = S_PTR[i];
            }
        }
    }

    void attach(uint8_t num, int value) {
        if (num >= AMOUNT || S_PTR != nullptr) return; // Не изменяем, если используется внешний массив
        signals[num] = value;
    }

    void setWindow(int window) {
        _window = window / 2;
    }
    
    bool status(uint8_t num) {
        if (millis() - tmr >= _AKEY_PERIOD) {
            tmr = millis();
            int16_t thisRead = analogRead(PIN);
            _ready = (abs(thisRead - _lastRead) < _window);
            _lastRead = thisRead;        
        }
        return check(num);
    }
    
    int pressed() {
        status(0);
        for (uint8_t i = 0; i < AMOUNT; i++) {
            if (check(i)) return i;
        }
        return -1;
    }

private:
    bool check(int i) {
        int16_t sig = signals[i];
        return (_ready && (_lastRead > sig - _window) && (_lastRead < sig + _window));
    }
    
    int16_t signals[AMOUNT] = {0};  // Внутренний массив по умолчанию
    int16_t _lastRead = 0;
    int16_t _window = 20;
    bool _ready = false;
    uint32_t tmr = 0;
};

#endif