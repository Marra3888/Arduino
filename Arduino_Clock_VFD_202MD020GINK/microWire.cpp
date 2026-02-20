
#include "microWire.h"
#include <avr/io.h>  // Для работы с регистрами AVR (TWCR, TWDR и т.д.)
#include <stdint.h>
#include <inttypes.h>

void TwoWire::begin() {
    pinMode(SDA, INPUT_PULLUP);  // Подтяжка шины
    pinMode(SCL, INPUT_PULLUP);  // Подтяжка шины
    TWBR = 72;                   // Стандартная скорость - 100kHz
    TWSR = 0;                    // Делитель - /1, статус - 0
}

void TwoWire::setClock(uint32_t clock) {
    TWBR = (((long)F_CPU / clock) - 16) / 2;  // Расчёт baudrate-регистра
}

void TwoWire::beginTransmission(uint8_t address) {
    TwoWire::start();           // Старт
    TwoWire::write(address << 1);  // Отправка адреса с битом "write"
}

uint8_t TwoWire::endTransmission(void) {
    return TwoWire::endTransmission(true);
}

uint8_t TwoWire::endTransmission(bool stop) {
    if (stop) TwoWire::stop();      // Если stop задан — отпустить шину
    else TwoWire::start();          // Иначе — restart
    if (_address_nack) {            // Нет ответа при передаче адреса
        _address_nack = false;
        _data_nack = false;
        return 2;
    }
    if (_data_nack) {               // Нет ответа при передаче данных
        _address_nack = false;
        _data_nack = false;
        return 3;
    }
    return 0;                       // Всё OK
}

size_t TwoWire::write(uint8_t data) {
    TWDR = data;                    // Записать данные в регистр
    TWCR = _BV(TWEN) | _BV(TWINT);  // Запустить передачу
    while (!(TWCR & _BV(TWINT)));   // Дождаться окончания
    uint8_t _bus_status = TWSR & 0xF8;  // Чтение статуса шины
    if (_bus_status == 0x20) _address_nack = true;  // SLA + W + NACK
    if (_bus_status == 0x30) _data_nack = true;     // BYTE + NACK
    return 1;                       // Совместимость с Wire API
}

uint8_t TwoWire::available() {
    return _requested_bytes;        // Количество оставшихся байт
}

uint8_t TwoWire::read() {
    if (--_requested_bytes) {       // Если байт не последний
        TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);  // Чтение с ACK
        while (!(TWCR & _BV(TWINT)));  // Дождаться окончания
        return TWDR;                   // Вернуть данные
    }
    _requested_bytes = 0;           // Последний байт
    TWCR = _BV(TWEN) | _BV(TWINT);  // Чтение с NACK
    while (!(TWCR & _BV(TWINT)));   // Дождаться окончания
    if (_stop_after_request) TwoWire::stop();  // Отпустить шину
    else TwoWire::start();          // Иначе — restart
    return TWDR;                    // Вернуть данные
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t length) {
    return TwoWire::requestFrom(address, length, true);
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t length, bool stop) {
    _stop_after_request = stop;     // Stop или restart после чтения
    _requested_bytes = length;      // Количество запрошенных байт
    TwoWire::start();               // Начать работу на шине
    TwoWire::write((address << 1) | 0x1);  // Адрес + бит "read"
    return length;                  // Совместимость с Wire API
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop) {
    return requestFrom(address, quantity, sendStop != 0);
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint32_t iaddress, uint8_t isize, uint8_t sendStop) {
    if (isize > 0) {
        beginTransmission(address);
        if (isize > 4) isize = 4;  // Ограничение размера внутреннего адреса
        while (isize-- > 0)
            write((uint8_t)(iaddress >> (isize * 8)));
        endTransmission(false);
    }
    return requestFrom(address, quantity, sendStop);
}

uint8_t TwoWire::requestFrom(int address, int quantity) {
    return requestFrom((uint8_t)address, (uint8_t)quantity, true);
}

uint8_t TwoWire::requestFrom(int address, int quantity, int sendStop) {
    return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
}

size_t TwoWire::write(const uint8_t *buffer, size_t size) {
    for (size_t i = 0; i < size; ++i)
        write(buffer[i]);
    return size;
}

void TwoWire::start() {
    TWCR = _BV(TWSTA) | _BV(TWEN) | _BV(TWINT);  // Start + enable + выполнить задачу
    while (!(TWCR & _BV(TWINT)));                // Ожидание завершения
}

void TwoWire::stop() {
    TWCR = _BV(TWSTO) | _BV(TWEN) | _BV(TWINT);  // Stop + enable + выполнить задачу
}

TwoWire Wirer = TwoWire();
