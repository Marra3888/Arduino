
#ifndef _microWire_h
#define _microWire_h

#include <Arduino.h>
#include <pins_arduino.h>

#define BUFFER_LENGTH 255

class TwoWire {
public:
    void begin(void);                                                 // инициализация шины
    void setClock(uint32_t clock);                                    // установка частоты шины 31-900 kHz (в герцах)
    void beginTransmission(uint8_t address);                          // открыть соединение (для записи данных)
    uint8_t endTransmission(bool stop);                               // закрыть соединение, произвести stop или restart
    uint8_t endTransmission(void);                                    // закрыть соединение, произвести stop
    size_t write(uint8_t data);                                       // отправить байт данных
    uint8_t requestFrom(uint8_t address, uint8_t length, bool stop);  // запросить данные от устройства
    uint8_t requestFrom(uint8_t address, uint8_t length);             // запросить данные, отпустить шину
    uint8_t read(void);                                               // прочитать байт
    uint8_t available(void);                                          // вернёт количество оставшихся байт

    // Совместимость с Wire API
    inline void beginTransmission(int address) {
        beginTransmission((uint8_t)address);
    }
    uint8_t requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop);
    uint8_t requestFrom(uint8_t address, uint8_t quantity, uint32_t iaddress, uint8_t isize, uint8_t sendStop);
    uint8_t requestFrom(int address, int quantity);
    uint8_t requestFrom(int address, int quantity, int sendStop);
    size_t write(const uint8_t *buffer, size_t size);
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }

private:
    uint8_t _requested_bytes = 0;     // количество запрошенных и непрочитанных байт
    bool _address_nack = false;       // флаг ошибки при передаче адреса
    bool _data_nack = false;          // флаг ошибки при передаче данных
    bool _stop_after_request = true;  // stop или restart после чтения последнего байта
    void start(void);                 // начало работы с шиной
    void stop(void);                  // конец работы с шиной
};

extern TwoWire Wirer;

#endif

