#include <Ticker.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Timezone.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "display.h"
#include "gps_time.h"

static const uint8_t RX_PIN = 4;  // D2 - RX для GPS
static const uint8_t TX_PIN = 5;  // D1 - TX для GPS
static const uint32_t GPS_BAUD = 9600;

const char* ssid = "TP-Link_22DC";
const char* password = "Mar4enko2704";

TinyGPSPlus gps;
SoftwareSerial gpsSerial(RX_PIN, TX_PIN);
Ticker ticker;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 10);

// Часовой пояс Киева (UTC+2 зимой, UTC+3 летом)
TimeChangeRule EEST = {"EEST", Last, Sun, Mar, 2, 180}; // Летнее время: UTC+3
TimeChangeRule EET = {"EET", Last, Sun, Oct, 3, 120};   // Зимнее время: UTC+2
Timezone tz(EEST, EET);

DateTimeStruct currentTime = {0, 0, 0, 1, 1, 0, 2023};
bool scrollEnabled = false;
uint8_t scrollPosX = 0;
bool wifiConnected = false;
char timeSource = 'M';
bool blinkState = true;
volatile bool tickerFlag50ms = false;
volatile bool tickerFlag1s = false;

void setup() {
  gpsSerial.begin(GPS_BAUD);
  Serial.begin(115200);
  Serial.println("Setup started...");

  displayInit();
  Serial.println("Display initialized");

  // Тест: отображаем "1234" на дисплее
  displayClear();
  displayChar('1', 0, 0);
  displayChar('2', 6, 0);
  displayChar('3', 12, 0);
  displayChar('4', 18, 0);
  displayRefresh();
  Serial.println("Displayed '1234' for test");

  connectToWiFi();
  Serial.println("WiFi connection attempted");

  if (wifiConnected) {
    Serial.println("Pinging pool.ntp.org...");
    IPAddress resolvedIP;
    if (WiFi.hostByName("pool.ntp.org", resolvedIP)) {
      Serial.print("DNS resolution successful: ");
      Serial.println(resolvedIP);
    } else {
      Serial.println("Failed to resolve pool.ntp.org");
    }
  }

  timeClient.begin();
  Serial.println("NTP client started");
  timeInit();
  Serial.println("Time initialized");

  ticker.attach(0.05, timerCallback);
  Serial.println("Ticker attached");
  Serial.println("Setup completed");
}

void loop() {
  static uint32_t lastUpdate = 0;
  static bool timeUpdated = false;
  static uint32_t lastDisplayUpdate = 0;

  Serial.println("Loop iteration started");

  if (millis() - lastUpdate > 3000) {
    Serial.println("Checking time update...");
    if (updateTimeFromGPS(gps, gpsSerial, currentTime)) {
      timeUpdated = true;
      timeSource = 'G';
    }
    else if (wifiConnected && updateTimeFromNTP(currentTime)) {
      timeUpdated = true;
      timeSource = 'N';
    }
    else {
      timeUpdated = false;
      timeSource = 'M';
      incrementTime(currentTime); // Увеличиваем время вручную
    }
    lastUpdate = millis();
    Serial.print("Current time: ");
    Serial.print(currentTime.hours); Serial.print(":");
    Serial.print(currentTime.minutes); Serial.print(":");
    Serial.println(currentTime.seconds);
    Serial.println("Time update checked");
  }

  if (tickerFlag50ms && (millis() - lastDisplayUpdate > 500)) {
    tickerFlag50ms = false;
    Serial.println("Ticker 50ms triggered");

    if (tickerFlag1s) {
      tickerFlag1s = false;
      if (!timeUpdated) {
        incrementTime(currentTime);
      }
      checkScrollTrigger(currentTime.seconds);
      blinkState = !blinkState;
      Serial.println("Ticker 1s triggered");
    }

    updateDisplayTime(currentTime);
    Serial.println("Display updated");

    if (scrollEnabled) {
      scrollPosX++;
      if (scrollPosX >= SCROLL_END_POS) {
        scrollEnabled = false;
        scrollPosX = 0;
      }
    }
    lastDisplayUpdate = millis();
  }
}

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    wifiConnected = false;
    Serial.println("\nWiFi connection failed");
  }
}

void checkScrollTrigger(uint8_t seconds) {
  if (seconds == 45) {
    scrollEnabled = true;
  }
}

void timerCallback() {
  static uint8_t counter = 0;
  tickerFlag50ms = true;
  if (++counter >= 20) {
    tickerFlag1s = true;
    counter = 0;
  }
}