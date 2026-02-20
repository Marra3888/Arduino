#define AP_SSID "TP-Link_22DC"
#define AP_PASS "Mar4enko2704"
#define GH_INCLUDE_PORTAL

#include <Wire.h>
#include <ClosedCube_HDC1080.h>
#include <Adafruit_BMP280.h>
#include <RTClib.h>

#include <ESP8266WiFi.h>
#include <WifiUDP.h>
#include <String.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <GyverHub.h>
GyverHub hub("MyDevices", "Display Clock", "");

ClosedCube_HDC1080 hdc1080;
Adafruit_BMP280 bmp; // I2C
RTC_DS3231 rtc;
gh::Log hlog;

unsigned BMPstatus;
unsigned RTCstatus;

#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "ua.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)
// Set up the NTP UDP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

String date_time_set;
unsigned long myTime;
String data_temp;
String data_hup;
String data_pres;
String data_data;
String data_time;



void build(gh::Builder& b) {
    b.Title("Home Made");
    b.Label("ESP8266").noTab(1).noLabel(1).size(3).fontSize(20).color(0x37A93C);
    b.Label("Bayer Display Clock").noTab(1).noLabel(1).size(3).fontSize(20).color(0x37A93C);
    {
        gh::Row r(b);
        b.Label_("w1").label(F("Date")).fontSize(20);
        b.Label_("w2").label(F("Time")).fontSize(20);
    }
    b.Label_("M1").label(F("Temperature:"));
    b.Label_("M2").label(F("Humidity:"));
    b.Label_("M3").label(F("Pressure:"));
}

void setup() {
    delay(1000);
    Serial.begin(9600);
    Serial.print("\f");
    delay(50);
    timeClient.begin();   // Start the NTP UDP client

    Serial.print("Connecting to ");
    Serial.print(AP_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(AP_SSID, AP_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\f");
    delay(50);
    Serial.println("Connected! ");
    Serial.print(WiFi.localIP());

    hub.onBuild(build);
    hub.begin();
    delay(2000);

    RTCstatus = rtc.begin();
    if (!RTCstatus) {
      Serial.print("\f");
      delay(50);
      Serial.println(F("  Real time clock         ERROR!"));
      delay(1000);
    }
    if (rtc.lostPower()) {
      Serial.print("\f");
      delay(50);
      Serial.println("RTC lost power,     let's set the time!");
      RTC_Time_Set();
    }
      // RTC_Time_Set();


    hdc1080.begin(0x40);

    BMPstatus = bmp.begin();
    if (!BMPstatus) {
      Serial.print("\f");
      delay(50);
      Serial.println(F("   BMP280 sensor         ERROR!"));
      delay(1000);
    }
      bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    myTime = millis();
    
    LCDprint();
    GetData1();
    GetData2();
}

void loop() {
    hub.tick();
    
    if ((millis() - myTime) > 500)
    {
        myTime = millis();
        LCDprint();
    }

    static gh::Timer tmr1(1000);   // период 1 секунда
    static gh::Timer tmr2(5000);   // период 5 секунд
    if (tmr1) 
    {                    // каждую секунду будем обновлять время
        GetData2();
    }
    if (tmr2) 
    {                    // обновление других данных
        GetData1();
    }
}

void GetData1()
{
    data_temp = "";
    data_temp += hdc1080.readTemperature();
    data_temp += " *C";
    data_hup = "";
    data_hup += hdc1080.readHumidity();
    data_hup += " %";
    data_pres = "";
    data_pres += (bmp.readPressure()/133.3);
    data_pres += " mm.";
    

    DateTime now = rtc.now();
    data_data = ""; 
    if(now.day() < 10){
      data_data += "0";
    }
    data_data += now.day();
    data_data += ".";
    if(now.month() < 10){
      data_data += "0";
    }
    data_data += now.month();
    data_data += ".";
    data_data += now.year();
    hub.update("w1").value(data_data);
    hub.update("M1").value(data_temp);
    hub.update("M2").value(data_hup);
    hub.update("M3").value(data_pres);
}

void GetData2()
{
    DateTime now = rtc.now();
    data_time = "";
    if(now.hour() < 10)
    {
      data_time += "0";
    }
    data_time += now.hour();
    data_time += ":";
    if(now.minute() < 10)
    {
      data_time += "0";
    }
    data_time += now.minute();
    data_time += ":";
    if(now.second() < 10)
    {
      data_time += "0";
    }
    data_time += now.second();
    hub.update("w2").value(data_time);
}

void LCDprint()
{
  // Serial.print("\f");
  // delay(50);
  // Serial.print("\14");
  
  DateTime now = rtc.now();
  // Serial.print(" ");
  if(now.hour() < 10)
  {
    Serial.print("0");
  }
  Serial.print(now.hour(), DEC);
  Serial.print(':');

  if(now.minute() < 10)
  {
    Serial.print("0");
  }
  Serial.print(now.minute(), DEC);
  Serial.print(':');

  if(now.second() < 10)
  {
    Serial.print("0");
  }
  Serial.print(now.second(), DEC);

  Serial.print("  ");


    if(now.day() < 10)
      {
        Serial.print("0");
      }
    Serial.print(now.day(), DEC);
    Serial.print('.');



  if(now.month() < 10)
  {
    Serial.print("0");
  }  
  Serial.print(now.month(), DEC);
  Serial.print('.');

  Serial.print(now.year(), DEC);
  

  

  // Serial.print(" ");

  Serial.print(hdc1080.readTemperature(), 1);
  Serial.print("\770");
  Serial.print("C ");
  Serial.print(hdc1080.readHumidity(), 0);
  Serial.print("% ");

  Serial.print(bmp.readPressure()/133.3); // мм. ртутного столбика
  Serial.println("mm");
}

void RTC_Time_Set()
{
  delay(1000);
  timeClient.update();
  unsigned long epochTime =  timeClient.getEpochTime();
  time_t local, utc;
  utc = epochTime;
  TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, +120};  //UTC - +2 hours - change this as needed
  TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, 0};      //UTC - 6 hours - change this as needed
  Timezone usEastern(usEDT, usEST);
  local = usEastern.toLocal(utc);
  rtc.adjust(DateTime(year(local), month(local), day(local), hour(local), minute(local), second(local)));
}





