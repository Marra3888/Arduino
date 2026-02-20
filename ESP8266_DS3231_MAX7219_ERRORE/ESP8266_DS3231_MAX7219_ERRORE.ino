
#include <Wire.h>

// DS3231 I2C address
const uint8_t DS3231_WRITE_ADDR = 0xD0; // 0b11010000
const uint8_t DS3231_READ_ADDR = 0xD1;  // 0b11010001

// MAX7219 register addresses
const uint8_t MAX7219_TEST = 0x0F;
const uint8_t MAX7219_BRIGHTNESS = 0x0A;
const uint8_t MAX7219_SCAN_LIMIT = 0x0B;
const uint8_t MAX7219_DECODE_MODE = 0x09;
const uint8_t MAX7219_SHUTDOWN = 0x0C;

// Max and min values for RTC settings
const uint8_t mxbt[] = {0, 23, 59, 31, 12, 99, 59, 23, 59, 15, 1};
const uint8_t mnbt[] = {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0};
const uint8_t stadrs[] = {0, 0x02, 0x01, 0x04, 0x05, 0x06, 0x00, 0x09, 0x08, 0x0B, 0x0D};

// Digit patterns for 7-segment display (0-9, blank, dp, C, `, B, r)
const uint8_t alfabet[16][8] = {
    {30, 63, 51, 51, 51, 51, 63, 30}, // 0
    {6, 14, 30, 6, 6, 6, 6, 15},      // 1
    {30, 63, 35, 6, 12, 24, 63, 63},  // 2
    {30, 63, 35, 14, 14, 35, 63, 30}, // 3
    {6, 14, 30, 54, 54, 63, 6, 15},   // 4
    {62, 63, 48, 62, 51, 3, 63, 30},  // 5
    {30, 63, 48, 62, 51, 51, 63, 30}, // 6
    {63, 63, 3, 6, 12, 12, 12, 12},   // 7
    {30, 63, 51, 30, 51, 51, 63, 30}, // 8
    {30, 63, 51, 51, 31, 3, 63, 30},  // 9
    {0, 0, 0, 0, 0, 0, 0, 0},         // blank
    {0, 3, 3, 0, 0, 3, 3, 0},         // dp
    {120, 124, 68, 64, 64, 68, 124, 120}, // C
    {56, 109, 109, 109, 57, 1, 1, 0}, // `
    {62, 63, 51, 62, 51, 51, 63, 62}, // B
    {0, 0, 27, 31, 28, 24, 24, 24}    // r
};

// Pin definitions for ESP8266 (NodeMCU v2)
const uint8_t CS_PIN = D6;    // GPIO12, Chip Select for MAX7219
const uint8_t DIN_PIN = D8;   // GPIO15, Data In for MAX7219
const uint8_t CLK_PIN = D7;   // GPIO13, Clock for MAX7219
const uint8_t ALARM_PIN = D5; // GPIO14, Alarm output
const uint8_t BUTTON_MODE = D2; // GPIO4, Button for mode
const uint8_t BUTTON_UP = D1;   // GPIO5, Button for up (interrupt-capable)
const uint8_t BUTTON_DOWN = D3; // GPIO0, Button for down
const uint8_t BUTTON_ALARM = D4; // GPIO2, Button for alarm on/off

// Variables
uint8_t i, j, x, y, yy, bright, m, s_, z_, u, dp, str;
uint8_t k, k0, k1, k4, k5, al;
bool flgal, flsh, x0, x1, x5, flgint, pflgint;
uint8_t ma[8], sa[8], za[8], uua[8], utmp[8], ztmp[8], stmp[8], mtmp[8];
uint8_t dsp[32], th_dsp[32], h_dsp[32];
uint8_t s0, s1, m0, m1, h0, h1, ah0, ah1, am0, am1;
uint8_t d0, d1, mo0, mo1, year0, year1, th0, th1, br0, br1;
uint8_t btset, btmod, btvar, btmp, grc;

// Software SPI to send a byte to MAX7219
void softwareSPI(uint8_t data) {
    for (int8_t bit = 7; bit >= 0; bit--) {
        digitalWrite(CLK_PIN, LOW);
        digitalWrite(DIN_PIN, (data >> bit) & 0x01);
        delayMicroseconds(1); // Задержка для стабильности
        digitalWrite(CLK_PIN, HIGH);
        delayMicroseconds(1); // Задержка для стабильности
    }
}

// Read from DS3231
uint8_t I2CR(uint8_t address) {
    Wire.beginTransmission(DS3231_WRITE_ADDR >> 1);
    Wire.write(address);
    if (Wire.endTransmission() != 0) {
        Serial.println("I2C write address failed!");
        return 0;
    }
    Wire.requestFrom(DS3231_READ_ADDR >> 1, 1);
    if (Wire.available()) {
        return Wire.read();
    }
    Serial.println("I2C read failed!");
    return 0;
}

// Write to DS3231
void I2CW(uint8_t address, uint8_t value) {
    Wire.beginTransmission(DS3231_WRITE_ADDR >> 1);
    Wire.write(address);
    Wire.write(value);
    if (Wire.endTransmission() != 0) {
        Serial.println("I2C write failed!");
    }
}

// Read RTC data
void read_RTC() {
    btmp = I2CR(0x00); // seconds
    s0 = btmp % 16;
    s1 = btmp / 16;
    btmp = I2CR(0x01); // minutes
    m0 = btmp % 16;
    m1 = btmp / 16;
    btmp = I2CR(0x02); // hours
    h0 = btmp % 16;
    h1 = btmp / 16;
    btmp = I2CR(0x04); // day
    d0 = btmp % 16;
    d1 = btmp / 16;
    btmp = I2CR(0x05); // month
    mo0 = btmp % 16;
    mo1 = btmp / 16;
    btmp = I2CR(0x06); // year
    year0 = btmp % 16;
    year1 = btmp / 16;
    btmp = I2CR(0x08); // alarm minutes
    am0 = btmp % 16;
    am1 = btmp / 16;
    btmp = I2CR(0x09); // alarm hours
    ah0 = btmp % 16;
    ah1 = btmp / 16;
    btmp = I2CR(0x11); // temperature
    th0 = btmp % 10;
    th1 = btmp / 10;
    bright = I2CR(0x0B); // brightness
    br0 = bright % 16;
    br1 = bright / 16;
    al = I2CR(0x0C); // alarm on/off
    flgal = (I2CR(0x0F) & 0x01); // alarm flag
    grc = I2CR(0x0D); // temperature on/off
    Serial.print("Time: "); Serial.print(h1); Serial.print(h0); Serial.print(":"); Serial.print(m1); Serial.print(m0); Serial.print(":"); Serial.print(s1); Serial.println(s0);
    Serial.print("Date: "); Serial.print(d1); Serial.print(d0); Serial.print("."); Serial.print(mo1); Serial.print(mo0); Serial.print(".20"); Serial.print(year1); Serial.println(year0);
    Serial.print("Temp: "); Serial.print(th1); Serial.println(th0);
}

// Send command to MAX7219
void maxCMD(uint8_t address, uint8_t value) {
    digitalWrite(CS_PIN, LOW);
    for (i = 0; i < 4; i++) {
        softwareSPI(address);
        softwareSPI(value);
    }
    digitalWrite(CS_PIN, HIGH);
}

// Initialize MAX7219
void max7219_init() {
    maxCMD(MAX7219_TEST, 0x01);
    delay(100);
    maxCMD(MAX7219_TEST, 0x00);
    maxCMD(MAX7219_DECODE_MODE, 0x00);
    maxCMD(MAX7219_BRIGHTNESS, bright ? bright : 0x0F); // Используем максимум, если bright = 0
    maxCMD(MAX7219_SCAN_LIMIT, 0x07); // 8 цифр
    maxCMD(MAX7219_SHUTDOWN, 0x01);
    Serial.println("MAX7219 initialized");
}

// Prepare hour display
void prepare_to_show_hour(uint8_t m, uint8_t s_, uint8_t z_, uint8_t u, uint8_t dp1, uint8_t al1) {
    for (i = 0; i < 8; i++) {
        ma[i] = alfabet[m][i];
        sa[i] = alfabet[s_][i];
        za[i] = alfabet[z_][i];
        uua[i] = alfabet[u][i];
        utmp[i] = uua[i];
        utmp[i] |= (za[i] & 0x01) << 7;
        ztmp[i] = za[i] >> 1;
        ztmp[i] |= (alfabet[10 + dp1][i] & 0x03) << 6;
        stmp[i] = sa[i] << 1;
        mtmp[i] = ma[i];
        if (i == 0) mtmp[i] |= al1 << 7;
        h_dsp[i] = utmp[i];
        h_dsp[i + 8] = ztmp[i];
        h_dsp[i + 16] = stmp[i];
        h_dsp[i + 24] = mtmp[i];
    }
}

// Prepare temperature display
void prepare_to_show_temp(uint8_t z_, uint8_t u) {
    for (i = 0; i < 8; i++) {
        za[i] = alfabet[z_][i];
        uua[i] = alfabet[u][i];
        mtmp[i] = za[i] >> 1;
        stmp[i] = uua[i];
        stmp[i] |= (za[i] & 0x20) << 2;
        ztmp[i] = alfabet[13][i];
        utmp[i] = alfabet[12][i] << 1;
        th_dsp[i] = utmp[i];
        th_dsp[i + 8] = ztmp[i];
        th_dsp[i + 16] = stmp[i];
        th_dsp[i + 24] = mtmp[i];
    }
}

// Scroll from temperature to hour
void temp2hour() {
    for (i = 0; i < 8; i++) {
        th_dsp[i + 24] = (th_dsp[i + 24] << 1) | ((th_dsp[i + 16] >> 7) & 0x01);
        th_dsp[i + 16] = (th_dsp[i + 16] << 1) | ((th_dsp[i + 8] >> 7) & 0x01);
        th_dsp[i + 8] = (th_dsp[i + 8] << 1) | ((th_dsp[i] >> 7) & 0x01);
        th_dsp[i] = (th_dsp[i] << 1) | ((h_dsp[i + 24] >> 7) & 0x01);
        h_dsp[i + 24] = (h_dsp[i + 24] << 1) | ((h_dsp[i + 16] >> 7) & 0x01);
        h_dsp[i + 16] = (h_dsp[i + 16] << 1) | ((h_dsp[i + 8] >> 7) & 0x01);
        h_dsp[i + 8] = (h_dsp[i + 8] << 1) | ((h_dsp[i] >> 7) & 0x01);
        h_dsp[i] = h_dsp[i] << 1;
    }
    for (i = 0; i < 32; i++) {
        dsp[i] = th_dsp[i];
    }
}

// Scroll from hour to temperature
void hour2temp() {
    for (i = 0; i < 8; i++) {
        h_dsp[i + 24] = (h_dsp[i + 24] << 1) | ((h_dsp[i + 16] >> 7) & 0x01);
        h_dsp[i + 16] = (h_dsp[i + 16] << 1) | ((h_dsp[i + 8] >> 7) & 0x01);
        h_dsp[i + 8] = (h_dsp[i + 8] << 1) | ((h_dsp[i] >> 7) & 0x01);
        h_dsp[i] = (h_dsp[i] << 1) | ((th_dsp[i + 24] >> 7) & 0x01);
        th_dsp[i + 24] = (th_dsp[i + 24] << 1) | ((th_dsp[i + 16] >> 7) & 0x01);
        th_dsp[i + 16] = (th_dsp[i + 16] << 1) | ((th_dsp[i + 8] >> 7) & 0x01);
        th_dsp[i + 8] = (th_dsp[i + 8] << 1) | ((th_dsp[i] >> 7) & 0x01);
        th_dsp[i] = th_dsp[i] << 1;
    }
    for (i = 0; i < 32; i++) {
        dsp[i] = h_dsp[i];
    }
}

// Handle button up/down
void up_down() {
    if (x0) {
        if (btset == 6) {
            I2CW(btmod, 0);
        } else {
            btvar = I2CR(btmod);
            btvar = ((btvar >> 4) * 10) + (btvar & 0x0F); // BCD to decimal
            if (btvar == mnbt[btset]) {
                btvar = mxbt[btset];
            } else {
                btvar--;
            }
            I2CW(btmod, ((btvar / 10) << 4) | (btvar % 10)); // Decimal to BCD
        }
        x0 = false;
    }
    if (x1) {
        if (btset == 6) {
            I2CW(btmod, 0);
        } else {
            btvar = I2CR(btmod);
            btvar = ((btvar >> 4) * 10) + (btvar & 0x0F);
            if (btvar == mxbt[btset]) {
                btvar = mnbt[btset];
            } else {
                btvar++;
            }
            I2CW(btmod, ((btvar / 10) << 4) | (btvar % 10));
        }
        x1 = false;
    }
}

// Prepare data for display
void prepare_2_show_data() {
    for (i = 0; i < 32; i++) {
        dsp[i] = h_dsp[i];
    }
}

// Send data to MAX7219
void send_2_display() {
    for (i = 0; i < 8; i++) {
        digitalWrite(CS_PIN, LOW);
        softwareSPI(i + 1);
        softwareSPI(dsp[i + 24]);
        softwareSPI(i + 1);
        softwareSPI(dsp[i + 16]);
        softwareSPI(i + 1);
        softwareSPI(dsp[i + 8]);
        softwareSPI(i + 1);
        softwareSPI(dsp[i]);
        digitalWrite(CS_PIN, HIGH);
    }
}

// Show data
void show_data() {
    prepare_2_show_data();
    send_2_display();
}

// Alarm control
void alarm() {
    digitalWrite(ALARM_PIN, flgal && (al & 0x01) && digitalRead(BUTTON_UP));
}

// Interrupt handler for button
void handleInterrupt() {
    flgint = !flgint;
    dp = !dp;
    k = (k + 1) % 23;
}

// Timer interrupt simulation
void timerCheck() {
    static unsigned long lastMillis = 0;
    if (millis() - lastMillis >= 50) {
        lastMillis = millis();
        if (flgal && (al & 0x01) && digitalRead(BUTTON_UP)) {
            // Alarm condition
        }
        y++;
        if (y >= 20) {
            flsh = !flsh;
            y = 0;
        }
        // Button mode
        static bool lastMode = HIGH;
        bool currentMode = digitalRead(BUTTON_MODE);
        if (lastMode == HIGH && currentMode == LOW) {
            btset = (btset + 1) % 11;
            btmod = stadrs[btset];
            Serial.print("Mode changed to: "); Serial.println(btset);
        }
        lastMode = currentMode;
        // Button up
        static bool lastUp = HIGH;
        bool currentUp = digitalRead(BUTTON_UP);
        if (btset != 0 && lastUp == HIGH && currentUp == LOW) {
            x0 = true;
            Serial.println("Up pressed");
        }
        lastUp = currentUp;
        // Button down
        static bool lastDown = HIGH;
        bool currentDown = digitalRead(BUTTON_DOWN);
        if (btset != 0 && lastDown == HIGH && currentDown == LOW) {
            x1 = true;
            Serial.println("Down pressed");
        }
        lastDown = currentDown;
        // Button alarm
        static bool lastAlarm = HIGH;
        bool currentAlarm = digitalRead(BUTTON_ALARM);
        if (lastAlarm == HIGH && currentAlarm == LOW) {
            x5 = true;
            Serial.println("Alarm button pressed");
        }
        lastAlarm = currentAlarm;
    }
}

void setup() {
    Serial.begin(115200);
    // Initialize pins
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    pinMode(DIN_PIN, OUTPUT);
    digitalWrite(DIN_PIN, LOW);
    pinMode(CLK_PIN, OUTPUT);
    digitalWrite(CLK_PIN, LOW);
    pinMode(ALARM_PIN, OUTPUT);
    digitalWrite(ALARM_PIN, LOW);
    pinMode(BUTTON_MODE, INPUT_PULLUP);
    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);
    pinMode(BUTTON_ALARM, INPUT_PULLUP);
    
    // Initialize I2C
    Wire.begin();
    Wire.setClock(400000); // Установить частоту I2C на 400 кГц
    Serial.println("I2C initialized at 400 kHz");
    
    // Проверка связи с DS3231
    Wire.beginTransmission(DS3231_WRITE_ADDR >> 1);
    if (Wire.endTransmission() == 0) {
        Serial.println("DS3231 found at address 0x68");
    } else {
        Serial.println("DS3231 not found!");
    }
    
    // Initialize DS3231
    bright = I2CR(0x0B);
    al = I2CR(0x0C);
    max7219_init();
    I2CW(0x0A, 0x80); // Alarm when hours, minutes, seconds match
    I2CW(0x0E, 0x00); // Disable square wave
    I2CW(0x0F, 0x00); // Clear alarm flag
    
    // Initialize variables
    str = 0;
    j = 0;
    y = 0;
    k = 0;
    x = 0;
    x0 = false;
    x1 = false;
    x5 = false;
    flgint = false;
    pflgint = false;
    flgal = false;
    dp = 0;
    btset = 0;
    
    // Attach interrupt for BUTTON_UP (GPIO5/D1 supports interrupts on ESP8266)
    attachInterrupt(digitalPinToInterrupt(BUTTON_UP), handleInterrupt, CHANGE);
    Serial.println("Setup complete");
}

void loop() {
    timerCheck();
    
    switch (btset) {
        case 0: // Show hour & temperature
            if (flgint != pflgint) {
                pflgint = flgint;
                read_RTC();
                prepare_to_show_hour(h1, h0, m1, m0, dp, al);
                prepare_to_show_temp(th1, th0);
                if (grc == 1) {
                    if (k <= 16) {
                        if (k == 16) {
                            dp = 1;
                            prepare_to_show_hour(h1, h0, m1, m0, dp, al);
                            for (yy = 0; yy < 32; yy++) {
                                hour2temp();
                                send_2_display();
                                delay(30);
                                alarm();
                            }
                        } else {
                            prepare_2_show_data();
                            send_2_display();
                            alarm();
                        }
                    } else {
                        if (k == 22) {
                            dp = 1;
                            prepare_to_show_hour(h1, h0, m1, m0, dp, al);
                            for (yy = 0; yy < 32; yy++) {
                                temp2hour();
                                send_2_display();
                                delay(30);
                                alarm();
                            }
                        } else {
                            prepare_to_show_temp(th1, th0);
                            for (i = 0; i < 32; i++) {
                                dsp[i] = th_dsp[i];
                            }
                            send_2_display();
                            alarm();
                        }
                    }
                } else {
                    show_data();
                    alarm();
                }
            }
            break;
        case 1: // Set hours
            read_RTC();
            if (!flsh) {
                h1 = 10;
                h0 = 10;
            }
            prepare_to_show_hour(h1, h0, m1, m0, 1, 0);
            show_data();
            up_down();
            break;
        case 2: // Set minutes
            read_RTC();
            if (!flsh) {
                m1 = 10;
                m0 = 10;
            }
            prepare_to_show_hour(h1, h0, m1, m0, 1, 0);
            show_data();
            up_down();
            break;
        case 3: // Set day
            read_RTC();
            if (!flsh) {
                d1 = 10;
                d0 = 10;
            }
            prepare_to_show_hour(d1, d0, mo1, mo0, 0, 0);
            show_data();
            up_down();
            break;
        case 4: // Set month
            read_RTC();
            if (!flsh) {
                mo1 = 10;
                mo0 = 10;
            }
            prepare_to_show_hour(d1, d0, mo1, mo0, 0, 0);
            show_data();
            up_down();
            break;
        case 5: // Set year
            read_RTC();
            if (!flsh) {
                year1 = 10;
                year0 = 10;
            }
            prepare_to_show_hour(2, 0, year1, year0, 0, 0);
            show_data();
            up_down();
            break;
        case 6: // Set seconds
            read_RTC();
            if (!flsh) {
                s1 = 10;
                s0 = 10;
            }
            prepare_to_show_hour(10, 10, s1, s0, 1, 0);
            show_data();
            up_down();
            break;
        case 7: // Set alarm hours
            read_RTC();
            if (!flsh) {
                ah1 = 10;
                ah0 = 10;
            }
            prepare_to_show_hour(ah1, ah0, am1, am0, 1, 1);
            show_data();
            up_down();
            break;
        case 8: // Set alarm minutes
            read_RTC();
            if (!flsh) {
                am1 = 10;
                am0 = 10;
            }
            prepare_to_show_hour(ah1, ah0, am1, am0, 1, 1);
            show_data();
            up_down();
            break;
        case 9: // Set brightness
            read_RTC();
            if (!flsh) {
                br1 = 10;
                br0 = 10;
            }
            prepare_to_show_hour(14, 15, br1, br0, 1, 0);
            show_data();
            up_down();
            read_RTC();
            maxCMD(MAX7219_BRIGHTNESS, ((bright >> 4) * 10) + (bright & 0x0F));
            break;
        case 10: // Set temperature display
            read_RTC();
            if (!flsh) {
                grc = 10;
            }
            prepare_to_show_hour(13, 12, 0, grc, 1, 0);
            show_data();
            up_down();
            break;
    }
    
    if (x5) {
        x5 = false;
        al = (al != 0) ? 0 : 1;
        while (digitalRead(BUTTON_ALARM) == LOW) {
            prepare_to_show_hour(ah1, ah0, am1, am0, 1, 1);
            show_data();
        }
        I2CW(0x0C, ((al / 10) << 4) | (al % 10));
        I2CW(0x0F, 0x00);
    }
    
    alarm();
}
