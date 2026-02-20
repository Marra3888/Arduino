// Build project 09.10.2025  12:40

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <OneWire.h>
#include <DallasTemperature.h>


LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 10 //вывод, к которому подключён DS18B20
#define TEMPERATURE_PRECISION 12 // точность измерений (9 ... 12)

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found

DeviceAddress Thermometer; // We'll use this variable to store a found device address


// function to print the temperature for a device
void printTemperature( )
{
  // float tempC = sensors.getTempC(deviceAddress);
  float tempC = sensors.getTempCByIndex(0);
  lcd.setCursor(0, 0);
  lcd.print("  Temperature");
  lcd.setCursor(5, 1);
  lcd.print(tempC, 2);
  lcd.print(" C");
}

void setup() 
{
  Wire.begin();
  Wire.beginTransmission(0x27);
  lcd.begin(16, 2);  // initialize the lcd

 // Start up the library
  sensors.begin();

  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();

  // lcd.createChar(1, dotOff);
  // lcd.createChar(2, dotOn);
  lcd.setBacklight(255);
  lcd.home();
  // lcd.clear();
  // lcd.print("Hello LCD");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Found device");
  lcd.setCursor(0, 1);
  lcd.print(numberOfDevices, DEC);

//  if (sensors.getAddress(Thermometer, numberOfDevices))
//     {
//       lcd.clear();
//       lcd.setCursor(0, 0);
//       lcd.print("Found device ");
//       lcd.setCursor(0, 1);
//       lcd.print(numberOfDevices, DEC);
      delay(3000);
//       // lcd.clear();
//       // lcd.setCursor(0, 0);
//       // lcd.print(" with address: ");
//       // lcd.setCursor(0, 1);
//       // lcd.print(tempDeviceAddress);
//       // // Serial.println();
//       // delay(3000);
//       lcd.clear();
//       lcd.setCursor(0, 0);
//       lcd.print("Setting resolution to ");
//       lcd.setCursor(0, 1);
//       lcd.print(TEMPERATURE_PRECISION, DEC);

//       // set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
//       sensors.setResolution(Thermometer, TEMPERATURE_PRECISION);

//       delay(3000);
      lcd.clear();
//       lcd.setCursor(0, 0);
//       lcd.print("Resolution actually set to: ");
//       lcd.setCursor(0, 1);
//       lcd.print(sensors.getResolution(Thermometer), DEC);
//       delay(3000);
      
//     } else {
//       // lcd.print("Found ghost device at ");
//       // lcd.print(i, DEC);
//       // lcd.print(" but could not detect address. Check power and cabling");
//     }

  sensors.getAddress(Thermometer, 0); // адрес DS18B20 (поиск по индексу)
  sensors.setResolution(Thermometer, TEMPERATURE_PRECISION);// установка точности измерения 9...12 разрядов

}

void loop() 
{
    sensors.requestTemperatures(); // Send the command to get temperatures
    //  if (sensors.getAddress(tempDeviceAddress, numberOfDevices))
    // {
    //        // It responds almost immediately. Let's print out the data
    delay(500);
      printTemperature(); // Use a simple function to print out the data
    // }

}
