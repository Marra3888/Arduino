/****************************************************/
/* This is only one example of code structure       */
/* OFFCOURSE this code can be optimized, but        */
/* the idea is let it so simple to be easy catch    */
/* where can do changes and look to the results     */
/****************************************************/


//set your clock speed
#define F_CPU 16000000UL
#include <util/delay.h>

// Standard Input/Output functions 1284
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VFD_in 8    // If 0 write LCD, if 1 read of LCD
#define VFD_clk 9   // if 0 is a command, if 1 is a data0
#define VFD_stb 10  // Must be pulsed to LCD fetch data of bus


#define AdjustPins PIND  // before is C, but I'm use port C to VFC Controle signals


unsigned char digit = 0;
unsigned char grid = 0;
unsigned char gridSegments = 0b00000011;  // Here I define the number of GRIDs and Segments I'm using Only have option of x10 or x11.

boolean flag = true;

unsigned int segOR[14];

void clear_VFD(void);

void SM1628B_init(void) {
  delayMicroseconds(200);  //power_up delay
  // Note: Allways the first byte in the input data after the STB go to LOW is interpret as command!!!

  // Configure VFD display (grids)
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628 is driver until 7 grids
  delayMicroseconds(1);
  // Write to memory display, increment address, normal operation
  cmd_with_stb(0b01000000);  //(BIN(01000000));
  delayMicroseconds(1);
  // Address 00H - 15H ( total of 11*2Bytes=176 Bits)
  cmd_with_stb(0b11000000);  //(BIN(01100110));
  delayMicroseconds(1);
  // set DIMM/PWM to value
  cmd_with_stb((0b10001000) | 7);  //0 min - 7 max  )(0b01010000)
  delayMicroseconds(1);
}
void cmd_without_stb(unsigned char a) {
  // send without stb
  unsigned char data = 170;  //value to transmit, binary 10101010
  unsigned char mask = 1;    //our bitmask

  data = a;
  //This don't send the strobe signal, to be used in burst data send
  for (mask = 0b00000001; mask > 0; mask <<= 1) {  //iterate through bit mask
    digitalWrite(VFD_clk, LOW);
    if (data & mask) {  // if bitwise AND resolves to true
      digitalWrite(VFD_in, HIGH);
      //Serial.print("1");
    } else {  //if bitwise and resolves to false
      digitalWrite(VFD_in, LOW);
      //Serial.print("0");
    }
    delayMicroseconds(5);
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(5);
  }
  //digitalWrite(VFD_clk, LOW);
  //Serial.println(" 8");
}
void cmd_with_stb(unsigned char a) {
  // send with stb
  unsigned char data = 170;  //value to transmit, binary 10101010
  unsigned char mask = 1;    //our bitmask

  data = a;

  //This send the strobe signal
  //Note: The first byte input at in after the STB go LOW is interpreted as a command!!!
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(1);
  for (mask = 00000001; mask > 0; mask <<= 1) {  //iterate through bit mask
    digitalWrite(VFD_clk, LOW);
    delayMicroseconds(1);
    if (data & mask) {  // if bitwise AND resolves to true
      digitalWrite(VFD_in, HIGH);
    } else {  //if bitwise and resolves to false
      digitalWrite(VFD_in, LOW);
    }
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(1);
  }
  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(1);
}
void tstDSP() {
  //LED's on Panel:  cont 40% 50% 60% 70% dry HI LO PWR WF TIME
  //Position on Byte:  Y   1   3   9   7   X   5  8  6  4   2
  //
  //The reference of display on panel is:2BHG40396B22, I don't find datasheet, any
  //way I desassembly and check it. This is a Anode Common (+)display
  //They used the SG7, SG8, SG9 as grid of digit1, 2 and 3.
  //This means they need use the GRD's to power up the segments of display
  //by this reason you will not find any logic to the sequence of bits to
  //control the bright of segments.
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void tstLED1() {
  //LED's on Panel:  cont 40% 50% 60% 70% dry HI LO PWR WF TIME
  //Position on Byte:  Y   1   3   9   7   X   5  8  6  4   2
  //
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00100000);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void tstLED2() {
  //LED's on Panel:  cont 40% 50% 60% 70% dry HI LO PWR WF TIME
  //Position on Byte:  Y   1   3   9   7   X   5  8  6  4   2
  //
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00100000);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void tstLED3() {
  //LED's on Panel:  cont 40% 50% 60% 70% dry HI LO PWR WF TIME
  //Position on Byte:  Y   1   3   9   7   X   5  8  6  4   2
  //
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00100000);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void tstLED4() {
  //LED's on Panel:  cont 40% 50% 60% 70% dry HI LO PWR WF TIME
  //Position on Byte:  Y   1   3   9   7   X   5  8  6  4   2
  //
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00100000);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void tstLED5() {
  //LED's on Panel:  cont 40% 50% 60% 70% dry HI LO PWR WF TIME
  //Position on Byte:  Y   1   3   9   7   X   5  8  6  4   2
  //
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00100000);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void tstLED6() {
  //LED's on Panel:  cont 40% 50% 60% 70% dry HI LO PWR WF TIME
  //Position on Byte:  Y   1   3   9   7   X   5  8  6  4   2
  //
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00100000);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void tstLED7() {
  //LED's on Panel:  cont 40% 50% 60% 70% dry HI LO PWR WF TIME
  //Position on Byte:  Y   1   3   9   7   X   5  8  6  4   2
  //
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00100000);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void tstLED8() {
  //LED's on Panel:  cont 40% 50% 60% 70% dry HI LO PWR WF TIME
  //Position on Byte:  Y   1   3   9   7   X   5  8  6  4   2
  //
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00010000);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void tstLED9() {
  //LED's on Panel:  cont 40% 50% 60% 70% dry HI LO PWR WF TIME
  //Position on Byte:  Y   1   3   9   7   X   5  8  6  4   2
  //
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00010000);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void tstLEDX() {
  //LED's on Panel:  cont 40% 50% 60% 70% dry HI LO PWR WF TIME
  //Position on Byte:  Y   1   3   9   7   X   5  8  6  4   2
  //
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00010000);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}

void tstLEDY() {
  //LED's on Panel:  cont 40% 50% 60% 70% dry HI LO PWR WF TIME
  //Position on Byte:  Y   1   3   9   7   X   5  8  6  4   2
  //
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00010000);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void tstLEDZ() {
  //LED's on Panel:  cont 40% 50% 60% 70% dry HI LO PWR WF TIME ION
  //Position on Byte:  Y   1   3   9   7   X   5  8  6  4   2    Z
  //
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000);
  cmd_without_stb(0b00000000);
  //                AAYZ       //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00010000);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void digit0(uint8_t grid) {
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void digit1(byte grid) {
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void digit2(byte grid) {
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void digit3(byte grid) {
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void digit4(byte grid) {
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void digit5(byte grid) {
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void digit6(byte grid) {
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void digit7(byte grid) {
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void digit8(byte grid) {
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void digit9(byte grid) {
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b01000000 << grid);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void digitA(byte grid) {
  cmd_with_stb(gridSegments);  // cmd 1 // SM1628B is a driver of 7 grids
  cmd_with_stb(0b01000000);    // cmd 2 //Normal operation; Set pulse as 1/16
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b11000000);  // Address of grids of display... (4 bits) they have done the swap of this pins with segments
  //                 GG12          //1-led, 2-led G0, G1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 FF3          //3-led, F0, F1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 CC54        //4-led, 5-led C0, C1,x,x,x,x                            t of digit only draw 1
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 EE76        //6-led, 7-led E0, E1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 DD98        //8-led, 9-led D0, D1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                 BBX        //10-led, B0, B1,x,x,x,x                                  t of digit only draw 1
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);
  //                  AAY        //11-led, A0, B1,x,x,x,x
  cmd_without_stb(0b00000000 << grid);
  cmd_without_stb(0b00000000);

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4  //10xx+ON/Off+dimming, dimming is last 3 bits.
  delay(500);
}
void clear_VFD(void) {
  /*
Here I clean all registers 
Could be done only on the number of grid
to be more fast. The 12 * 3 bytes = 36 registers
*/
  for (int n = 0; n < 14; n++) {  // important be 10, if not, bright the half of wells./this on the VFD of 6 grids)
    //cmd_with_stb(gridSegments); // cmd 1 // SM1628B is fixed to 5 grids
    cmd_with_stb(0b01000000);  //       cmd 2 //Normal operation; Set pulse as 1/16
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(1);
    cmd_without_stb((0b11000000) | n);  // cmd 3 //wich define the start address (00H to 15H)
    cmd_without_stb(0b00000000);        // Data to fill table of 6 grids * 15 segm = 80 bits on the table
    //
    //cmd_with_stb((0b10001000) | 7); //cmd 4
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(1);
  }
}
void allOn(void) {
  /*
Here I clean all registers 
Could be done only on the number of grid
to be more fast. The 12 * 3 bytes = 36 registers
*/
  for (int n = 0; n < 14; n++) {  // important be 10, if not, bright the half of wells./this on the VFD of 6 grids)
    //cmd_with_stb(gridSegments); // cmd 1 // SM1628B is fixed to 5 grids
    cmd_with_stb(0b01000000);  //       cmd 2 //Normal operation; Set pulse as 1/16
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(1);
    cmd_without_stb((0b11000000) | n);  // cmd 3 //wich define the start address (00H to 15H)
    cmd_without_stb(0b11111111);        // Data to fill table of 6 grids * 15 segm = 80 bits on the table
    //
    //cmd_with_stb((0b10001000) | 7); //cmd 4
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(1);
  }
}
void setup() {
  // put your setup code here, to run once:
  pinMode(VFD_in, OUTPUT);
  pinMode(VFD_clk, OUTPUT);
  pinMode(VFD_stb, OUTPUT);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  SM1628B_init();
  clear_VFD();
}
void readButtons() {
  //Take special attention to the initialize digital pin LED_BUILTIN as an output.
  //
  int ledPin = 13;  // LED connected to digital pin 13
  int inPin = 8;    // pushbutton connected to digital pin 7
  int val = 0;      // variable to store the read value
  int dataIn = 0;

  byte array[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  byte together = 0;

  unsigned char data = 0;  //value to transmit, binary 10101010
  unsigned char mask = 1;  //our bitmask

  array[0] = 1;

  unsigned char btn1 = 0x41;

  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(2);
  cmd_without_stb(0b01000010);  // cmd 2 //10=Read Keys; 00=Wr DSP;
  delayMicroseconds(2);
  // cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 15H)
  // send without stb

  pinMode(8, INPUT);  // Important this point! Here I'm changing the direction of the pin to INPUT data.
  delayMicroseconds(2);
  //PORTD != B01010100; // this will set only the pins you want and leave the rest alone at
  //their current value (0 or 1), be careful setting an input pin though as you may turn
  //on or off the pull up resistor
  //This don't send the strobe signal, to be used in burst data send
  for (int z = 0; z < 5; z++) {
    //for (mask=00000001; mask > 0; mask <<= 1) { //iterate through bit mask
    for (int h = 8; h > 0; h--) {
      digitalWrite(VFD_clk, HIGH);  // Remember wich the read data happen when the clk go from LOW to HIGH! Reverse from write data to out.
      delayMicroseconds(2);
      val = digitalRead(inPin);
      //digitalWrite(ledPin, val);    // sets the LED to the button's value
      if (val & mask) {  // if bitwise AND resolves to true
                         //Serial.print(val);
        //data =data | (1 << mask);
        array[h] = 1;
      } else {  //if bitwise and resolves to false
                //Serial.print(val);
        // data = data | (1 << mask);
        array[h] = 0;
      }
      digitalWrite(VFD_clk, LOW);
      delayMicroseconds(2);
    }

    Serial.print(z);  // All the lines of print is only used to debug, comment it, please!
    Serial.print(" - ");

    for (int bits = 7; bits > -1; bits--) {
      Serial.print(array[bits]);
    }


    if (z == 0) {
      if (array[5] == 1) {
        flag = !flag;  // 
      }
    }


    if (z == 0) {
      if (array[7] == 1) {
        digitalWrite(10, !digitalRead(10));
      }
    }

    if (z == 3) {
      if (array[7] == 1) {
        //digitalWrite(VFD_onRed, !digitalRead(VFD_onRed));
        //digitalWrite(VFD_onGreen, !digitalRead(VFD_onGreen));
      }
    }
    Serial.println();
  }                  // End of "for" of "z"
  Serial.println();  // This line is only used to debug, please comment it!

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(2);
  cmd_with_stb((0b10001000) | 7);  //cmd 4
  delayMicroseconds(2);
  pinMode(8, OUTPUT);  // Important this point! Here I'm changing the direction of the pin to OUTPUT data.
  delay(1);
}
void loop() {
  //Note I keep the commands allways as format binary to be possible change a bit and see the result
  //offcourse a array or a matrix array of bytes will turn the length of code very short and allow to
  //use the function ADD or OR at send command like is done at example done with the shift.
  byte grid = 0x00;
  for(uint8_t s = 0x00; s<3; s++){
      allOn();
      delay(2000);
      clear_VFD();
      delay(350);
  }
 
  tstDSP();
  for (uint8_t g = 0x00; g < 2; g++) {
    digit0(g);
    digit1(g);
    digit2(g);
    digit3(g);
    digit4(g);
    digit5(g);
    digit6(g);
    digit7(g);
    digit8(g);
    digit9(g);
    digitA(g);
  }
  tstLED1();
  tstLED2();
  tstLED3();
  tstLED4();
  tstLED5();
  tstLED6();
  tstLED7();
  tstLED8();
  tstLED9();
  tstLEDX();
  tstLEDY();
  tstLEDZ();  //This is the last code done to solve the missing of led marked as "ION".

  //// I decide not use the dinamic refreshing, but you can do it!
  //                readButtons();
  //                delay(100);
}
