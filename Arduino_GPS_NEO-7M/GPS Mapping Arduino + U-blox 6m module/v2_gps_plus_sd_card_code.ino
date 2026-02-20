#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
File myFile;
SoftwareSerial GPSModule(7, 8); // RX, TX
int updates;
int pos;
int stringplace = 0;
boolean updateCount = false;
unsigned long previousMillis = 0;
const long interval = 10000;
String nmea[16];
//String labels[16] {"Time: ", "Latitude: ", "Hemisphere: ", "Longitude: ", "Hemisphere: ", "Quality: ", "Number of Sata: ", "HD: ", "Altitude: "};
void setup() {

  Serial.begin(57600);

  GPSModule.begin(9600);

  delay(2000);

  if (!SD.begin(4)) {
    //Serial.println("initialization failed!");
    return;
  }
  //Serial.println("initialization done.");

  delay(100);
}
/
//eg3. $GPGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
//1    = UTC of Position
//2    = Latitude
//3    = N or S
//4    = Longitude
//5    = E or W
//6    = GPS quality indicator (0=invalid; 1=GPS fix; 2=Diff. GPS fix)
//7    = Number of satellites in use [not those in view]
//8    = Horizontal dilution of position
//9    = Antenna altitude above/below mean sea level (geoid)
//10   = Meters  (Antenna height unit)
//11   = Geoidal separation (Diff. between WGS-84 earth ellipsoid and
//       mean sea level.  -=geoid is below WGS-84 ellipsoid)
//12   = Meters  (Units of geoidal separation)
//13   = Age in seconds since last update from diff. reference station
//14   = Diff. reference station ID#
//15   = Checksum
//
//eg4. $GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a*hh
//1    = UTC of position fix
//2    = Data status (V=navigation receiver warning)
//3    = Latitude of fix
//4    = N or S
//5    = Longitude of fix
//6    = E or W
//7    = Speed over ground in knots
//8    = Track made good in degrees True
//9    = UT date
//10   = Magnetic variation degrees (Easterly var. subtracts from true course)
//11   = E or W
//12   = Checksum
String ConvertLat() {
  String posneg = "";
  if (nmea[2] == "S") {
    posneg = "-";
  }
  String latfirst;
  float latsecond;
  for (int i = 0; i < nmea[1].length(); i++) {
    if (nmea[1].substring(i, i + 1) == ".") {
      latfirst = nmea[1].substring(0, i - 2);
      latsecond = nmea[1].substring(i - 2).toFloat();
    }
  }
  latsecond = latsecond / 60;
  String CalcLat = "";

  char charVal[9];
  dtostrf(latsecond, 4, 6, charVal);
  for (int i = 0; i < sizeof(charVal); i++)
  {
    CalcLat += charVal[i];
  }
  latfirst += CalcLat.substring(1);
  latfirst = posneg += latfirst;
  return latfirst;
}

String ConvertLng() {
  String posneg = "";
  if (nmea[4] == "W") {
    posneg = "-";
  }

  String lngfirst;
  float lngsecond;
  for (int i = 0; i < nmea[3].length(); i++) {
    if (nmea[3].substring(i, i + 1) == ".") {
      lngfirst = nmea[3].substring(0, i - 2);
      //Serial.println(lngfirst);
      lngsecond = nmea[3].substring(i - 2).toFloat();
      //Serial.println(lngsecond);

    }
  }
  lngsecond = lngsecond / 60;
  String CalcLng = "";
  char charVal[9];
  dtostrf(lngsecond, 4, 6, charVal);
  for (int i = 0; i < sizeof(charVal); i++)
  {
    CalcLng += charVal[i];
  }
  lngfirst += CalcLng.substring(1);
  lngfirst = posneg += lngfirst;
  return lngfirst;
}


void writeDataToCard() {

  myFile = SD.open("TEST2.CSV", FILE_WRITE);
  Serial.println("");

     Serial.println("");
    Serial.print(" Time: ");
    Serial.print(nmea[0]);
    Serial.print(" Lat: ");
    Serial.print(nmea[1]);
    Serial.print(" Lng: ");
    Serial.print(nmea[3]);
    Serial.print(" Fix: ");
    Serial.print(nmea[5]);
    Serial.print(" Sats: ");
    Serial.print(nmea[6]);
    Serial.print(" Alt: ");
    Serial.print(nmea[8]);
    Serial.println(" ");
String dataString = "";
  dataString += nmea[0];
  dataString += ",";
  dataString += nmea[1];
  dataString += ",";
  dataString += nmea[3];
  dataString += ",";
  dataString += nmea[5];
  dataString += ",";
  dataString += nmea[6];
  dataString += ",";
  dataString += nmea[8];
  dataString += "\n";
  myFile.print(dataString);
  myFile.close();
}


void loop() {
  
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (updateCount == false) {
      updateCount = true;
      writeDataToCard();


    } else {
      updateCount = false;
    }


  }
  Serial.flush();
  GPSModule.flush();
  while (GPSModule.available() > 0)
  {
    GPSModule.read();

  }
  if (GPSModule.find("$GPGGA,")) {
  
    String tempMsg = GPSModule.readStringUntil('\n');

    for (int i = 0; i < tempMsg.length(); i++) {
      if (tempMsg.substring(i, i + 1) == ",") {
        nmea[pos] = tempMsg.substring(stringplace, i);
        //Serial.println(nmea[pos]);
        stringplace = i + 1;
        pos++;
      }
      if (i == tempMsg.length() - 1) {
        nmea[pos] = tempMsg.substring(stringplace, i);
      }
    }

    
    nmea[1] = ConvertLat();
    nmea[3] = ConvertLng();




  
    //1    = UTC of Position
    //2    = Latitude
    //3    = N or S
    //4    = Longitude
    //5    = E or W
    //6    = GPS quality indicator (0=invalid; 1=GPS fix; 2=Diff. GPS fix)
    //7    = Number of satellites in use [not those in view]
    //8    = Horizontal dilution of position
    //9    = Antenna altitude above/below mean sea level (geoid)
    //10   = Meters  (Antenna height unit)
    //11   = Geoidal separation (Diff. between WGS-84 earth ellipsoid and
    //       mean sea level.  -=geoid is below WGS-84 ellipsoid)
    //12   = Meters  (Units of geoidal separation)
    //13   = Age in seconds since last update from diff. reference station
    //14   = Diff. reference station ID#
    //15   = Checksum




  }
  else {

    

  }
  stringplace = 0;
  pos = 0;


}



