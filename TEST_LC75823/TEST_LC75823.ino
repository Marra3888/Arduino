#include <SPI.h>

// --- НАСТРОЙКИ ---
const int csPin = 10;
const int address = 130; // 0x82
const int buttonPin = 2; // Кнопка между D2 и GND

// Переменные для хранения текущей позиции
int currentByte = 0;
int currentBit = -1; // Начинаем с -1, чтобы первое нажатие включило 0.0

void setup() {
  Serial.begin(9600);
  
  // Настройка кнопки (внутренний резистор подтягивает к 5В)
  pinMode(buttonPin, INPUT_PULLUP);
  
  // Настройка экрана
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);
  SPISettings(38000, MSBFIRST, SPI_MODE0);
  SPI.begin();

  Serial.println("--- MANUAL STEPPER MODE ---");
  Serial.println("Press the button on Pin 2 to advance.");
  
  // Гасим экран в начале
  sendPacket(-1, -1);
}

void loop() {
  // Читаем кнопку (LOW означает нажатие, т.к. замыкаем на землю)
  if (digitalRead(buttonPin) == LOW) {
    
    // --- ЛОГИКА ШАГА ---
    currentBit++;
    
    // Если биты в байте кончились (больше 7)
    if (currentBit > 7) {
      currentBit = 0;
      currentByte++;
    }
    
    // Если байты кончились (больше 19) -> начинаем сначала
    if (currentByte > 19) {
      currentByte = 0;
      currentBit = 0;
    }

    // === ЗАЩИТА ОТ ВЫКЛЮЧЕНИЯ ЭКРАНА (Byte 19, Bit 1) ===
    if (currentByte == 19 && currentBit == 1) {
      Serial.println(">>> SKIPPING CONTROL BIT (Screen ON/OFF) <<<");
      currentBit++; // Перепрыгиваем через него
    }

    // --- ВЫВОД ИНФОРМАЦИИ ---
    printInfo();

    // --- ОТПРАВКА НА ЭКРАН ---
    sendPacket(currentByte, currentBit);

    // Антидребезг (ждем пока отпустят или просто задержка)
    delay(200); 
    while(digitalRead(buttonPin) == LOW); // Ждем отпускания кнопки
    delay(50);
  }
}

void printInfo() {
  Serial.print("BYTE: ");
  Serial.print(currentByte);
  Serial.print("\t BIT: ");
  Serial.print(currentBit);
  
  Serial.print("\t -> CODE: ");
  
  if (currentByte < 15) {
     Serial.print("_screen[");
     Serial.print(currentByte);
     Serial.print("]");
  } else {
     Serial.print("_symbols[");
     Serial.print(currentByte - 15);
     Serial.print("]");
  }
  
  Serial.print(", bitWrite(..., ");
  Serial.print(currentBit);
  Serial.println(", 1);");
}

void sendPacket(int targetByte, int targetBit) {
  digitalWrite(csPin, LOW);
  SPI.transfer(address);
  digitalWrite(csPin, HIGH); // CS High после адреса
  delayMicroseconds(5);

  // Отправляем 20 байт
  for (int i = 0; i < 20; i++) {
    byte dataToSend = 0x00;

    // Включаем только целевой бит
    if (i == targetByte) {
      dataToSend = (1 << targetBit);
    }

    // Обработка управляющего байта (19)
    if (i == 19) {
       // Бит 1 (SC) должен быть 0, чтобы экран работал
       bitWrite(dataToSend, 1, 0); 
       // Бит 0 (DR) - попробуйте 0 или 1, если тускло
       // bitWrite(dataToSend, 0, 1); 
    }

    SPI.transfer(dataToSend);
  }
  
  digitalWrite(csPin, LOW);
}