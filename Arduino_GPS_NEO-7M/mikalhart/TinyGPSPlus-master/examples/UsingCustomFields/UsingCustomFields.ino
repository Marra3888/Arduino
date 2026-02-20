#include <TinyGPS++.h>
#include <SoftwareSerial.h>

/*
   This sample demonstrates TinyGPS++'s capacity for extracting custom
   fields from any NMEA sentence.  TinyGPS++ has built-in facilities for
   extracting latitude, longitude, altitude, etc., from the $GPGGA and 
   $GPRMC sentences.  But with the TinyGPSCustom type, you can extract
   other NMEA fields, even from non-standard NMEA sentences.

   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(RX) and 3(TX).
*/
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

/*
   By declaring TinyGPSCustom objects like this, we announce that we
   are interested in the 15th, 16th, and 17th fields in the $GPGSA 
   sentence, respectively the PDOP (F("positional dilution of precision")),
   HDOP (F("horizontal...")), and VDOP (F("vertical...")).

   (Counting starts with the field immediately following the sentence name, 
   i.e. $GPGSA.  For more information on NMEA sentences, consult your
   GPS module's documentation and/or http://aprs.gids.nl/nmea/.)

   If your GPS module doesn't support the $GPGSA sentence, then you 
   won't get any output from this program.
*/

TinyGPSCustom pdop(gps, "GPGSA", 15); // $GPGSA sentence, 15th element
TinyGPSCustom hdop(gps, "GPGSA", 16); // $GPGSA sentence, 16th element
TinyGPSCustom vdop(gps, "GPGSA", 17); // $GPGSA sentence, 17th element

void setup() 
{
  Serial.begin(115200);
  ss.begin(GPSBaud);

  Serial.println(F("UsingCustomFields.ino"));
  Serial.println(F("Demonstrating how to extract any NMEA field using TinyGPSCustom"));
  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
}

void loop() 
{/*
  // Every time anything is updated, print everything.
  if (gps.altitude.isUpdated() || gps.satellites.isUpdated() ||
    pdop.isUpdated() || hdop.isUpdated() || vdop.isUpdated())
  {
    Serial.print(F("ALT="));   Serial.print(gps.altitude.meters()); 
    Serial.print(F(" PDOP=")); Serial.print(pdop.value()); 
    Serial.print(F(" HDOP=")); Serial.print(hdop.value()); 
    Serial.print(F(" VDOP=")); Serial.print(vdop.value());
    Serial.print(F(" SATS=")); Serial.println(gps.satellites.value());
  }
*/
if (gps.location.isUpdated())
  {
Serial.print(F("Широта в градусах ="));  Serial.println(gps.location.lat(), 6); // Latitude in degrees (double)
Serial.print(F("Долгота в градусах ="));Serial.println(gps.location.lng(), 6); // Longitude in degrees (double)
Serial.print(F("="));Serial.print(gps.location.rawLat().negative ? "-" : "+");
Serial.print(F("Сырая широта в целых градусах ="));Serial.println(gps.location.rawLat().deg); // Raw latitude in whole degrees
Serial.print(F("Сырая широта в целых миллиардных долях ="));Serial.println(gps.location.rawLat().billionths);// ... and billionths (u16/u32)
Serial.print(F(" ="));Serial.print(gps.location.rawLng().negative ? "-" : "+");
Serial.print(F("Сырая долгота в целых градусах ="));Serial.println(gps.location.rawLng().deg); // Raw longitude in whole degrees
Serial.print(F("Сырая долгота в целых миллиардных долях ="));Serial.println(gps.location.rawLng().billionths);// ... and billionths (u16/u32)
Serial.print(F("Необработанная дата в формате DDMMYY ="));Serial.println(gps.date.value()); // Raw date in DDMMYY format (u32)
Serial.print(F("Год ="));Serial.println(gps.date.year()); // Year (2000+) (u16)
Serial.print(F("Месяц ="));Serial.println(gps.date.month()); // Month (1-12) (u8)
Serial.print(F("Число ="));Serial.println(gps.date.day()); // Day (1-31) (u8)
Serial.print(F("Сырое время в формате HHMMSSCC ="));Serial.println(gps.time.value()); // Raw time in HHMMSSCC format (u32)
Serial.print(F("Час ="));Serial.println(gps.time.hour()); // Hour (0-23) (u8)
Serial.print(F("Минуты ="));Serial.println(gps.time.minute()); // Minute (0-59) (u8)
Serial.print(F("Секунды ="));Serial.println(gps.time.second()); // Second (0-59) (u8)
Serial.print(F("Сотые секунды ="));Serial.println(gps.time.centisecond()); // 100ths of a second (0-99) (u8)
Serial.print(F("Необработанная скорость в сотых долях узла ="));Serial.println(gps.speed.value()); // Raw speed in 100ths of a knot (i32)
Serial.print(F("Скорость в узлах ="));Serial.println(gps.speed.knots()); // Speed in knots (double)
Serial.print(F("Скорость в милях в час ="));Serial.println(gps.speed.mph()); // Speed in miles per hour (double)
Serial.print(F("Скорость в метрах в секунду ="));Serial.println(gps.speed.mps()); // Speed in meters per second (double)
Serial.print(F("Скорость в километрах в час ="));Serial.println(gps.speed.kmph()); // Speed in kilometers per hour (double)
Serial.print(F("Сырой курс в сотых степени ="));Serial.println(gps.course.value()); // Raw course in 100ths of a degree (i32)
Serial.print(F("Курс в градусах ="));Serial.println(gps.course.deg()); // Course in degrees (double)
Serial.print(F("Необработанная высота в сантиметрах ="));Serial.println(gps.altitude.value()); // Raw altitude in centimeters (i32)
Serial.print(F("Высота в метрах ="));Serial.println(gps.altitude.meters()); // Altitude in meters (double)
Serial.print(F("Высота в милях ="));Serial.println(gps.altitude.miles()); // Altitude in miles (double)
Serial.print(F("Высота в километрах ="));Serial.println(gps.altitude.kilometers()); // Altitude in kilometers (double)
Serial.print(F("Высота в футах ="));Serial.println(gps.altitude.feet()); // Altitude in feet (double)
Serial.print(F("Количество используемых спутников ="));Serial.println(gps.satellites.value()); // Number of satellites in use (u32)
Serial.print(F("Точность по горизонтали ="));Serial.println(gps.hdop.value()); // Horizontal Dim. of Precision (100ths-i32)
Serial.println();
  }
  while (ss.available() > 0)
    gps.encode(ss.read());
}
