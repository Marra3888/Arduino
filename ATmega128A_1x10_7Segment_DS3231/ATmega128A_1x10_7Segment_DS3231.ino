#include "SevSeg.h"
#include <Arduino.h>
#include <Wire.h>
#include <DS3231.h>

#define F_ 		   (key==12)
#define SET_ 	   (key==8)
#define PLUS 	   (key==0)
#define MINUS	   (key==4)
#define Nreg_st	16

uint8_t fl_err_eep = 0, key=17, ocr1[5]={4, 0, 4, 0, 0}, call_nomer, kolvo_bud = 12, anti_drebezg[3];
uint8_t regim = 0, time_view_buf=1, regim_state_time[Nreg_st + 1], signal, calls[12][3];
uint16_t mask;

//const char letter[] = {"A6 43c1d"};
char letter_to_display[11] = {};
//char decPlace = 9;
SevSeg sevseg; //Instantiate a seven segment controller object
RTClib RTC;
DateTime now;
//DS3231 dsrtc;
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  byte numDigits = 10;   
  byte digitPins[] = {49, 48, 7, 6, 5, 4, 3, 2, 1, 0}; //Digits: 1,2,3,4 <--put one resistor (ex: 220 Ohms, or 330 Ohms, etc, on each digit pin)
  byte segmentPins[] = {16, 17, 18, 19, 20, 21, 22, 23}; //Segments: A,B,C,D,E,F,G,Period

//  sevseg.begin(COMMON_ANODE, numDigits, digitPins, segmentPins);
    SevSeg();
    sevseg.begin(N_TRANSISTORS, numDigits, digitPins, segmentPins, true, false, false);

//  sevseg.setBrightness(10); //Note: 100 brightness simply corresponds to a delay of 2000us after lighting each segment. A brightness of 0 
                            //is a delay of 1us; it doesn't really affect brightness as much as it affects update rate (frequency).
                            //Therefore, for a 4-digit 7-segment + pd, COMMON_ANODE display, the max update rate for a "brightness" of 100 is 1/(2000us*8) = 62.5Hz.
                            //I am choosing a "brightness" of 10 because it increases the max update rate to approx. 1/(200us*8) = 625Hz.
                            //This is preferable, as it decreases aliasing when recording the display with a video camera....I think.
                            
  Wire.begin();
/*
   dsrtc.setDate(29);
 dsrtc.setMonth(5);
  dsrtc.setYear(19);
  dsrtc.setHour(16);
  dsrtc.setMinute(8);
  */
  regim = 0;
  // sevseg.setBrightness(50);
}

void loop()
{
  now = RTC.now();

//  key_action();
//  Display();
//  data_lcd();




  if(now.second()== 0)
  display_date(0);
  else
  if(now.second()> 5)
  display_time(1);
  

    sevseg.setChars(letter_to_display);

  sevseg.refreshDisplay(); // Must run repeatedly; don't use blocking code (ex: delay()) in the loop() function or this won't work right
}
//------------------------------------------------------------------------------------------------------------------------------------------------
void display_time(byte i) 
{
  
  for(byte f = 0; f < i; f++) letter_to_display[f] = ' ';
  
  strcpy(letter_to_display+i, p2dig(now.hour(), DEC));
  strcpy(letter_to_display+(i+2), "-");
  strcpy(letter_to_display+(i+3), p2dig(now.minute(), DEC));
  strcpy(letter_to_display+(i+5), "-");
  strcpy(letter_to_display+(i+6), p2dig(now.second(), DEC));
//  strcpy(letter_to_display+(i+8), "-");
//  strcpy(letter_to_display+(i+9), "-");

}
//--------------------------------------------------------------------------------------------------------------------
void display_date(byte i) 
{
  
  for(byte f = 0; f < i; f++) letter_to_display[f] = ' ';
  
  strcpy(letter_to_display+i, p2dig(now.day(), DEC));
  strcpy(letter_to_display+(i+2), "-");
  strcpy(letter_to_display+(i+3), p2dig(now.month(), DEC));
  strcpy(letter_to_display+(i+5), "-");
  strcpy(letter_to_display+(i+6), "20");
  strcpy(letter_to_display+(i+8), p2dig(now.year()%100, DEC));
//  strcpy(letter_to_display+(i+8), "-");
//  strcpy(letter_to_display+(i+9), "-");
//  decPlace = 25;
}
//---------------------------------------------------------------------------------------------------------
char *p2dig(uint8_t v, uint8_t mode2)
// 2 digits leading zero
{
  static char s[3];
  uint8_t i = 0;
  uint8_t n = 0;
 
  switch(mode2)
  {
    case HEX: n = 16;  break;
    case DEC: n = 10;  break;
  }

  if (v < n) s[i++] = '0';
  itoa(v, &s[i], n);
  
  return(s);
}
//##########################################################################################################################################################################
void reg_see_init(void)
{
	time_view_buf=3;
	regim_state_time[Nreg_st]=0;
}
//**************************************
void inc_dec_var(unsigned char *var, unsigned char limit){
		if (PLUS){
			if ((*var)<limit)	(*var)++; 
			else *var=0;
			}
		if (MINUS){
			if (*var) (*var)--;
			else *var=limit;
			}
}
//**************************************
void mask_next_regim(unsigned int m, unsigned char next_F,unsigned char SET)
{
 	 if (!anti_drebezg[0])  mask=m; 
	 if (F_)   regim=next_F;
	 if (SET_) regim=SET;
	 key=17;
}

void next_regim(void){
	if (PLUS) {
		if (regim<218) 
		regim++;
		}
	if (MINUS) {
		if (regim>210)
		regim--;
		}
}
//----------------------------------------------------------------------------------------------------
// void key_action(void)
// {//mask_next_regim( maska regima, if (key F_), if (key SET_) )
//   unsigned char i, bit_N, a;
//   uint8_t gl_ind_str = 2;
//   uint8_t mask = 0;
  
//   if (fl_err_eep)
//   {
//     if ((F_)||(SET_)||(MINUS)||(PLUS))    regim = 214;//219
//   }
//   else if (!regim) 
//       {
//         if (F_)   
//         {
//           regim = 210;//CLOCK
//         }
      
//         else  if (SET_)
//               {
//                 regim = 31;//CALLS
//               }
//             else  if (MINUS) 
//                   {
//                     regim = 75;//31//CAL. bIP C
//                           //  reg_see_init();
//                   }
//                 else  if (PLUS) 
//                       {
//                         reg_see_init();
//                         regim = 91;//SEE
//                       }
//         return;
//       }

// //------------------------------------------------------------------------------    
//   if (regim == 53 || regim == 54)
//     {
//       if (ocr1[4] == 0) signal = 15;
//     }
    
// /*  else  if ((signal) && (key != 17))
//           {
//         //    signal_0();
//         //    if (SET_)
//         //      time_flag_fl_otsrochka_signala = false;
//               //if (regim == 150) //при срабатывании будильника
//           //  regim = 0;
//             return;
//           }
//   */

// //------------------------------------------------------------------------------
// //---------------------------------- CALLS -------------------------------------
//    if (regim == 31) 
//     {//set number calss
//       inc_dec_var(&call_nomer, kolvo_bud-1);
//       mask_next_regim(0,0b10000000000000000000,0,32);
//       return;
//     }
//    if (regim == 32) 
//     {//on off calls
//       inc_dec_var_bit(&calls[call_nomer][2], 0);
//       mask_next_regim(0,0b1100000000000000000,31,33);
//       return;
//     }
//    if (regim == 33) 
//     {//set hour calss
//       inc_dec_var(&calls[call_nomer][c_hour],23);
//       mask_next_regim(0,0b1100000000000000,31,34);
//       return;
//     }
//    if (regim == 34) 
//     {//set min calls
//       inc_dec_var(&calls[call_nomer][c_min],59);
//       mask_next_regim(0,0b1100000000000,31,35);
//       return;// 
//     }
//   for(i = 35, a = 1; i < 42 ; i++, a++)
//     {
//         if (regim == i) 
//         {//set day calss
//           inc_dec_var_bit(&calls[call_nomer][2], a);
//           bit_N = _BV(8-a);
//           mask_next_regim(1,bit_N,31,i+1);
//       break;
//         }
//     }
//   if (regim == 42) 
//     {//save current budilnik in eeprom
//       for( a = EEPROM_calls+call_nomer*3, i = 0 ;  i < 3  ;a++, i++) 
//         EEPROM_write(a, calls[call_nomer][i]);
//       if (call_nomer < 3) regim = 31;//31
//       else regim = 45;
//       return;
//     }
//   if (regim == 45) 
//     {//set hour
//       inc_dec_var(&calls_pin[call_nomer-3][c_hour],23); 
//       mask_next_regim(1,0b110000000,31,46);//SET.o.00-00
//       return;
//     }
//   if (regim == 46) 
//     {// set min
//       inc_dec_var(&calls_pin[call_nomer-3][c_min],59);
//       mask_next_regim(1,0b110000,31,47);//
//       return;
//     }
//   if (regim == 47) 
//     {// set sek
//       inc_dec_var(&calls_pin[call_nomer-3][c_sek],59);
//       mask_next_regim(1,0b110,31,31);//
//       if (regim == 31)
//         {
//         //  for( a = EEPROM_calls_pin+(call_nomer-3)*3, i = 0 ;  i < 3  ;a++, i++) 
//         //    EEPROM_write( a, calls_pin[call_nomer-3][i]);
//         }
//       return;
//     }
// //-------------------------------------- end calls ---------------------------------------------- 
// #define r_clock 21
// //----------------------------------------- CLOCK -------------------------------------------------
//   if (regim == 210) 
//     {//121
//       next_regim();
//       mask_next_regim(2,0xFFFFF,0,r_clock); 
//       return; 
//     }//clock 4ACb|
//   // 12-00-00
//   if (regim == r_clock) 
//     {//set hour sek
//     //  if (MINUS)
//     //    {
//     //        if (hour[h_sek] > 31)  hour[h_min]++;
//     //          hour[h_sek] = 0;
//     //          clock_ = 0;
//     //      }
//       inc_dec_var(&hour[h_sek],59);
//       if(clock_ != hour[h_sek])
//       {
//         clock_ = hour[h_sek];
//         rtc_write(0, dec2bcd(hour[h_sek]));
//       }
//       mask_next_regim(0,0b1100000000000,210,r_clock+1);
//       return;
//     }
//   if (regim == r_clock + 1) 
//     {//set min 
      
//       inc_dec_var(&hour[h_min],59);
//       if(clock_ != hour[h_min])
//       {
//         clock_ = hour[h_min];
//         rtc_write(1, dec2bcd(hour[h_min]));
//       }
//       mask_next_regim(0,0b1100000000000000,210,r_clock+2);
      
//       return;
//     }
//   if (regim == r_clock+2) 
//     {//set hour
//       inc_dec_var(&hour[h_hour],23);
//       if(clock_ != hour[h_hour])
//       {
//         clock_ = hour[h_hour];
//         rtc_write(2, dec2bcd(hour[h_hour]));
//       }
//       mask_next_regim(0,0b1100000000000000000,210,r_clock+3);
//       return;
//     }
//   //28.07.08 ПН
//   if (regim == r_clock+6) 
//     {//set day week 
//       inc_dec_var(&hour[h_num], num_in_month[hour[h_month]]);//-1
//       if(clock_ != hour[h_num])
//       {
//         clock_ = hour[h_num];
//         rtc_write(4, dec2bcd(hour[h_num]));
//       }
//       mask_next_regim(1,0b1100000000,210,r_clock+7);
//       return;
//     }
//   if (regim == r_clock+5) 
//     {// set month hour
//       inc_dec_var(&hour[h_month],12);//-1
//       if(clock_ != hour[h_month])
//       {
//         clock_ = hour[h_month];
//         rtc_write(5, dec2bcd(hour[h_month]));
//       }
//       mask_next_regim(1,0b11000000,210,r_clock+6);
//       return;
//     } 
//   if (regim == r_clock+4) 
//     {// set year
//       inc_dec_var(&hour[h_year],99);
//       if(clock_ != hour[h_year])
//       {
//         clock_ = hour[h_year];
//         rtc_write(6, dec2bcd(hour[h_year]));
//       }
//       mask_next_regim(1,0b110000,210,r_clock+5);
//       return;
//     } 
//   if (regim == r_clock+3) 
//     {//set day
//       inc_dec_var(&hour[h_day],7);
//       if(hour[h_day] == 0) hour[h_day] = 1;
//       if(clock_ != hour[h_day])
//       {
//         clock_ = hour[h_day];
//         rtc_write(3, dec2bcd(hour[h_day]));
//       }
//       mask_next_regim(1,0b110,210,r_clock+4);
//       return;
//     }
//   if (regim == r_clock+7) 
//     {//_pin-0_ _c                  
//       if (PLUS) hour_bit = 1;
//       if (MINUS) hour_bit = 0;
//       mask_next_regim(0,0b1100000000000,210,r_clock+8);
//       return;
//     }
//   // коррекция
//   if (regim == r_clock+8) 
//     {//set corection znak
//       inc_dec_var_bit(&corr_flag, fl_corr_znak_pl);
//       mask_next_regim(1,0b100000,210,r_clock+9);
//       return;
//     }
//   if (regim == r_clock+9) 
//     {//set corection
//       inc_dec_var(&correction,40);//ограничение 40
//       if(clock_ != correction)
//       {
//       //  clock_ = correction;
//       //  rtc_write(16, dec2bcd(correction));
//       //  correction = bcd2dec(rtc_read(16));
//       }
//       mask_next_regim(1,0b11110,0,210);
//       if (regim == 210) 
//         {
//           EEPROM_write(EEPROM_correction, correction);
//           EEPROM_write(EEPROM_corr_flag, corr_flag);
//         }
//       return;
//     }
// //---------------------------------- end cklock ---------------------------------------------------

// //------------------------------------ SEE ---------------------------------------------------3//122
//   if (regim == 8)
//     {//122
//       reg_see_init();
//       next_regim();
//       mask_next_regim(2,0xFFFFF,0,91);//_УСt_SEE_
//       return;
//     }
//   if (regim == 91) 
//     {//F1 r.1-000 
//       inc_dec_var(&regim_state_time[Nreg_st], Nreg_st/2-1);
//       mask_next_regim(0,0b1000000000000000000,0,92);//BIT(8)
//       return;
//     } 
//   if (regim == 92) 
//     {//F1 r.1-000 
//       inc_dec_var(&regim_state_time[regim_state_time[Nreg_st]*2],11);//8
//       mask_next_regim(0,0b1000000000000000,94,93);//91
//       return;
//     } 
//    if (regim == 93) 
//     {//r.1-000 F1
//       inc_dec_var(&regim_state_time[regim_state_time[Nreg_st]*2+1],255);
//       mask_next_regim(0,0b11100000000000,94,94);//94
//       return;
//     }

//   if (regim == 94) 
//     {
//       for(i = EEPROM_regim_state_time; i < Nreg_st+EEPROM_regim_state_time ; i++)
//         EEPROM_write(i, regim_state_time[i-EEPROM_regim_state_time]);
//         regim = 0;
//       return;
//     }
// //-----------------------------------------END SEE------------------------------------------
// //--------------------------------------- yct bud ----------------------------------------4//123
//   if (regim == 211) //214
//     {//123
//       next_regim();
//       mask_next_regim(2,0xFFFFF,0,75);//yct bud 
//       return;
//     }
//   if (regim == 75) 
//     {//CAL. bIP C
//       inc_dec_var(&signal_bud, 2);
//       mask_next_regim(0,0b1110000000000000,211,76);//214
//       return;
//     }
//   if (regim == 76) 
//     {//otsroch. on
//       time_flag_fl_otsrochka_signala = false;//если отсрочка была а fl_otsrochka_true=0 потом 1 сработает сигнал, а так нет
//       if (PLUS) time_flag_fl_otsrochka_true = true;
//       if (MINUS) time_flag_fl_otsrochka_true = false;
//   //    inc_dec_var_bit(&time_flag_fl_otsrochka_true, 0);
//       mask_next_regim(1,0b110,75,77);
//     if (regim == 77) 
//       {//сигнал шим или вкл вывода
//         EEPROM_write(EEPROM_sig_pin_or_bip, signal_bud );
//         EEPROM_write(EEPROM_otsrochka_true, (time_flag_fl_otsrochka_true));
//         //debugeep clear_adres_eeprom();
//         regim = 0;
//       }
//       return;         
//     }
// //---------------------------------- end yct bud --------------------------------------------------------------------------------

// //------------------------------------- light ----------------------------------------------------5//124
//   if (regim == 212) //215
//     {//125
//       next_regim();
//       mask_next_regim(2,0xFFFFF,0,11); 
//       return; 
//     }//light
//   if (regim == 11) 
//     {
//       inc_dec_var(&day_light,8);
//       if (!day_light) day_light = 1;
//       mask_next_regim(0,0b10000000000000000000,212,14);//215
//       return;
//     }
//   if (regim == 14) 
//     {
//       inc_dec_var(&night_light,8);
//       if (!night_light) night_light = 1;
//       mask_next_regim(1,0b1000,11,15);//noch L.5//215
//       if (regim == 15) 
//         {//night_light
//           EEPROM_write(EEPROM_light, day_light);
//           EEPROM_write(EEPROM_light+1, night_light);
//         //regim = 215;
//         }
//       return;
//     }
//   if (regim == 15) 
//     {
//       inc_dec_var(&tim_c_light,23);
//       mask_next_regim(1,0b11000000,14,16);//215
//       return;
//     }
//   if (regim == 16) 
//     {
//       inc_dec_var(&tim_do_light,23);
//       mask_next_regim(1,0b110,15,212);//215
//       if (regim == 212) //215
//         {//night_light
//           EEPROM_write(EEPROM_light+2, tim_c_light);
//           EEPROM_write(EEPROM_light+3, tim_do_light);
//         }
//       return;
//     }
// //------------------------------------end  light ----------------------------------------------------------
// //-------------------------------------- SIGHAL -------------------------------------------6//125
//   if (regim == 213)//216
//     {//124
//       signal = 0;
//       next_regim();
//       mask_next_regim(2,0xFFFFF,0,52);//SIGHAL
//       return;
//     }
//   if (regim == 52)
//     {// S. Bud. But // regim 52-53-54 budil button 
//       inc_dec_var(&ocr1[4],1);
//       mask_next_regim(0,0b11111100000000000000,213,53);//53budil 54button 
//       signal = 0;
//       return;
//     }
//   if (regim == 53)
//     {//H000 L000
//       inc_dec_var(&ocr1[ocr1[4]*2],0xFF);//ocr1ahi
//       mask_next_regim(1,0b111000000,55,54);
//       return;
//     }
//   if (regim == 54) 
//     {//H000 L000 // 0 0 0 0 0 0 4
//       inc_dec_var(&ocr1[ocr1[4]*2+1],0xFF);//ocr1ahi
//       mask_next_regim(1,0b1110,55,53);
//       return;
//     }
//   if (regim == 55) 
//     {
//       EEPROM_write(EEPROM_verifu_but, 0xAA);
//       for(i = EEPROM_ocr1a; i < 4+EEPROM_ocr1a ; i++)
//       EEPROM_write(i, ocr1[i-EEPROM_ocr1a]);
//       regim = 52;
//       return;
//     }
// //------------------------------------ end  signal--------------------------------------------------

// //---------------------------------------- DEFAULT --------------------------------------------8//127
//   if (regim == 214) //219
//     {//127  dEFAULt P //CLEAR CALLS
//     if (SET_)
//       {
//           bit_N = 0;
//         viewstate[0] = 0;//data
//         viewstate[1] = 3;//hour min sec
//       for(i = EEPROM_calls; i < kolvo_bud*3+EEPROM_calls; i++)
//         {
//             EEPROM_write(i, bit_N); //
//         }
//     //  for(i = EEPROM_calls_pin; i < sizeof(calls_pin)+EEPROM_calls_pin; i++) 
//     //    EEPROM_write(i, 0);
//       for(i = EEPROM_regim_state_time; i < Nreg_st+EEPROM_regim_state_time; i++)
//         EEPROM_write(i, 0);
//       for(i = EEPROM_ocr1a; i < 4+EEPROM_ocr1a; i++)
//         EEPROM_write(i, 0);
//     //  for(i = EEPROM_t_stat; i < 8*2+EEPROM_t_stat; i++)
//     //    EEPROM_write(i, 0);
      
//     //  EEPROM_write(EEPROM_t_st_count_porog, t_st_count_porog = 15);
//     //  EEPROM_write(EEPROM_t_press_count_porog, t_press_count_porog = 30);
//     //  EEPROM_write(EEPROM_press_koef, press_koef = 27);
//       EEPROM_write(EEPROM_light, 8);
//       EEPROM_write(EEPROM_light+1, 8);
//       EEPROM_write(EEPROM_light+2, tim_c_light = 23);
//       EEPROM_write(EEPROM_light+3, tim_do_light = 6);
//       EEPROM_write(EEPROM_regim_set, 4);
//       EEPROM_write(EEPROM_regim_set+1, 8);
//       EEPROM_write(EEPROM_sig_pin_or_bip, 0);
//       EEPROM_write(EEPROM_otsrochka_true, 0);
//     //  EEPROM_write(EEPROM_vid_t, 0);
//       EEPROM_write(EEPROM_correction, 0);
//       EEPROM_write(EEPROM_corr_flag, 0);
//       read_data_EEPROM();
//       reg_see_init();
//       fl_err_eep = 0;
//         }
//     next_regim();
//     mask_next_regim(2,0xFFFFF,0,0);
//     return;
//     }
// }
//--------------------------------------------------------------------------------------------------------------
// void Display(void)
// {
// //  for(uint8_t i = 0; i < 10; i++)
// //    {
//       if (znmesto <= 9)
//       {
//         display_buffer[znmesto] = lcd_buffer[0][znmesto];
//     //    display_buffer[znmesto] = lcd_buffer[1][znmesto - 10];
//       }
//       else
//       {
//         display_buffer[znmesto] = lcd_buffer[1][znmesto - 10];
//     //    display_buffer[znmesto + 10] = lcd_buffer[1][znmesto - 10];
//       }
      
// //    }
//   on_display();
// }
//-------------------------------------------------------------------------------------------------------------
// void on_display(void)
// {
// //  static unsigned char tenm = 0;
//   static uint32_t tenms = 0;
  
// //  tenm++;                  // Read DS3231 every 100 x 10ms = 1 sec
// //  if (tenm > 1)
// //  {
  
    
//     display_(display_buffer[(uint8_t)tenms], _register[tenms]);

//     tenms++;                  // Read DS3231 every 100 x 10ms = 1 sec
//     if (tenms > 19)
//     {

//     tenms = 0;
//     }
// //    tenm = 0;
// //  }
// }
//----------------------------------------------------------------------------------------------------------------
// void display_(uint8_t zz, uint32_t d)
// {

// //  b = segs[zz];
//   b = zz;
//   b = (b<<24);

//   b += d;

//   shift_out(b);

// }
//---------------------------------------------------------------------------------------------------------------
// void data_lcd(void)
// {
// unsigned char i,a,b;
//    cls_lcd_buffer();
  
//   if (regim)
//     {
//       time_view_buf[0] = 0;
//       time_view_buf[1] = 0;
//     }
  
//   if (!regim)
//     {
//       indikator_manager();
//       if (otsrochka()) 
//       if (!(time_flag_fl_sek_1_switch)) 
//             lcd_buffer[0][9]^=tochka;
//       return;
//     }
// //------------------------------LIGHT-----------------------------------------------------  
//   if  (regim == 212)//215
//     {//0, 0, L, I, G, H, t, 0, 0 //2,5
//       load_str_in_lcdbuffer(0,&str_light[0]);
//       load_str_in_lcdbuffer(1,&str_setup[0]);
//       return;
//     }
// //--------------------------------------light---------------------------------------------
//   if  ((regim>=11) && (regim<=14))
//     {
//       lcd_buffer[0][0] = font[f_L] | tochka;
//         for(i = 0; i < day_light; i++)
//         lcd_buffer[0][i+1] = font[f_o];
//       load_str_in_lcdbuffer(1,&str_light_noch[0]);
//       lcd_buffer[1][6] = font[night_light];
//       num2_bcd_in_lcd(1,8,light_swith[i]);
      
//       if(regim == 11)  lcd_light = day_light;
//       else 
//         if(regim == 14) lcd_light = night_light;
//       else  lcd_light = day_light;
//       return;
//     }
// //--------------------------------------Ночь - --------------------------------------------
//   if  ((regim > 14) && (regim < 17))
//     {//H, O, 4, b, -, 0, 0 //0,6 
//       load_str_in_lcdbuffer(0,&str_light_noch[0]);
//       lcd_buffer[1][1] = font[f_c] | tochka;
//       num2_bcd_in_lcd(1,2,tim_c_light);
//       lcd_buffer[1][5] = font[f_d];
//       lcd_buffer[1][6] = font[f_o] | tochka;
//       num2_bcd_in_lcd(1,7,tim_do_light);
//       return;
//     }
// //---------------------------------- end light------------------------------------------------
  
// //--------------------------------ЧАСЫ--------------------------------------------------------
//   if (regim == 210)
//     {//0, 0, 4, A, C, b, I, 0, 0// 2,5
//       load_str_in_lcdbuffer(0,&str_clock[0]);//Часы
//       load_str_in_lcdbuffer(1,&str_setup[0]);//Setup
//       return;
//       }
//   if ((regim >= r_clock) && (regim <= r_clock+6))
//     {//2
//         clock_in_lcd1(0);//hour-min-sek корекция часов clock_in_lcd0();
//       num_month_year_day(1);
//       return;
//       }
//   if ((regim >= r_clock+7) && (regim <= r_clock+9))
//     {//C, o, r, r., //0x39, 0x9C, 0x14, 0x54,
//       load_str_in_lcdbuffer(0,&str_hour[0]);
//       on__(0,7,hour_bit);
//       load_str_in_lcdbuffer(1,&str_corr[0]);
//       if (corr_flag & _BV(fl_corr_znak_pl)) lcd_buffer[1][4] = font[f_seg_d]; 
//           else     lcd_buffer[1][4] = font[f_seg_g];   
//       lcd_buffer[1][5] = font[n0]^tochka; 
//       result_in_bcd(correction);
//       lcd_buffer[1][6] = font[bcd_num2];
//       lcd_buffer[1][7] = font[bcd_num3];
//       lcd_buffer[1][8] = font[bcd_num4];
//       return;
//     } 
// //--------------------------------------CALLS--------------------------------------------------------------------------
//   if ((regim > 30) && (regim < 42))
//     {
//         lcd_buffer[0][0] = font[call_nomer];
//       lcd_hh_mm(0,&calls[call_nomer] [c_hour]);
//       on__(0,1, calls[call_nomer][2]);
//       if (call_nomer > 2) lcd_buffer[0][3] = font[f_seg_a];
//       if (call_nomer > 6) lcd_buffer[0][3] = (font[f_seg_a] | font[f_seg_b]);
//         if (call_nomer > 10) lcd_buffer[0][3] = (font[f_seg_a] | font[f_seg_b] | font[f_seg_g]);
//       if (call_nomer > 14) lcd_buffer[0][3] = (font[f_seg_a] | font[f_seg_b] | font[f_seg_g] | font[f_seg_f]);
//       lcd_buffer[1][0] = font[f_d]^tochka;
//       for (i = 1; i < 8; i++)
//         {
//           if ((calls[call_nomer] [2]) & _BV(i))
//             lcd_buffer[1][i+1] = font[i]; 
//           else lcd_buffer[1][i+1] = font[f_seg_d];
//         }
//       return;
//     } 
//   if ((regim > 44) && (regim < 48))
//     {
//       load_str_in_lcdbuffer(0,&str_bud_long[0]);
//       num2_bcd_in_lcd(1,1,calls_pin[call_nomer-3][c_hour]);
//       lcd_buffer[1][3] = font[f_seg_g];
//       num2_bcd_in_lcd(1,4,calls_pin[call_nomer-3][c_min]);
//       lcd_buffer[1][6] = font[f_seg_g];
//       num2_bcd_in_lcd(1,7,calls_pin[call_nomer-3][c_sek]);
//       return;
//     } 

// //-------------------------end CALLS-------------------------------------------
// //----------------------SIGNAL-------------------------------------------------
//   if (regim == 213)//216
//     {//0, S, I, G, H, A, L, 0, 0 //1,6
//       load_str_in_lcdbuffer(0,&str_signal[0]);
//       load_str_in_lcdbuffer(1,&str_setup[0]);
//       return;
//       }
//   if ((regim >= 52) && (regim <= 54))
//     {
//       if (ocr1[4] == 0) load_str_in_lcdbuffer(0,&str_budil[0]); 
//       else  load_str_in_lcdbuffer(0,&str_button[0]);  
//       lcd_buffer[0][7] = font[f_S];
//       lcd_buffer[0][8] = font[f_L];
//       lcd_buffer[1][0] = font[f_H];
//       result_in_bcd(ocr1[ocr1[4]*2]);
//       num3_th_in_lcd_buffer(1,1);
//       lcd_buffer[1][5] = font[f_L];
//       result_in_bcd(ocr1[ocr1[4]*2+1]);
//       num3_th_in_lcd_buffer(1,6);
//       return;
//     }
// //-------------------------END--SIGNAL-----------------------------------------
// //--------------------------Default-----------------------------------------------------   
//   if (regim == 214)//219
//     {//d, E, F, A, U, L, t, 0, S, //0,9
//       load_str_in_lcdbuffer(0,&str_default[0]);
//       load_str_in_lcdbuffer(1,&str_eprom[0]);
//       return;
//       }
// //-----------------------------------YCT. bud.------------------------------------------
//   if (regim == 211)//214
//     {//  _yct_bud_ //0, y, C, t, 0, 6, u, d., 0 //1,7 
//       load_str_in_lcdbuffer(0,&str_yst_bud[0]);
//       load_str_in_lcdbuffer(1,&str_setup[0]);
//       return;
//       }
//   if ((regim >= 75) && (regim <= 76))
//     { //CAL. bIP C //C, A, L., //0x39, 0xB7, 0x59, 
//       load_str_in_lcdbuffer(0,&str_cal[0]);
//       if (signal_bud == 0)
//         {
//             a = font[f_b];
//             b = font[f_i];
//             i = font[f_P];
//           }
//         else
//         if (signal_bud == 1)
//         {
//               a = font[f_P];
//             b = font[f_seg_e];
//             i = font[f_n];
//           }
//         else
//         {
//             a = font[f_O];
//             b = font[f_b];
//             i = font[f_A];
//           }
//         lcd_buffer[0][4] = a;
//         lcd_buffer[0][5] = b;
//         lcd_buffer[0][6] = i;
//       lcd_buffer[0][8] = font[f_C]^tochka;
//       load_str_in_lcdbuffer(1,&str_otsro4[0]);
//       on__(1,7, time_flag_fl_otsrochka_true);
//       return;
//     }
// //------------------------------------------ YCT SEE ----------------------------------------------------------
//   if ((regim > 90) && (regim < 94)) 
//     {
//       lcd_buffer[0][0] = font[f_P];
//       lcd_buffer[0][1] = font[regim_state_time[Nreg_st]+1];
//       if (regim_state_time[Nreg_st] <= 3) lcd_buffer[0][2] = font[f_seg_a];
//       else lcd_buffer[0][2] = font[f_seg_d];
//       lcd_buffer[0][3] = font[f_r]^tochka;
//       lcd_buffer[0][4] = font[regim_state_time[regim_state_time[Nreg_st]*2]];
//   //    if(regim_state_time[regim_state_time[Nreg_st]*2] == 4)
//   //    {
//   //      ukaz_na_str=&str_clokk[0];
//   //      simvol=0;
//   //    }
//       lcd_buffer[0][5] = font[f_seg_g];
//       result_in_bcd(regim_state_time[regim_state_time[Nreg_st]*2+1]);
//       lcd_buffer[0][6] = font[bcd_num2];
//       lcd_buffer[0][7] = font[bcd_num3];
//       lcd_buffer[0][8] = font[bcd_num4];
//       view_in_indikator(1,regim_state_time[regim_state_time[Nreg_st]*2]);
//       return;
//     }
// //------------------------------------Regim 150 Бегущая строка------------------------------------------------
//   if (regim == 150)
//     {//бегущая строка
//       reg_see_init();
//       lcd_buffer[0][0] = font[n6] | tochka;
//       lcd_buffer[0][1] = font[f_seg_g];
//       lcd_buffer[0][2] = font[call_nomer];
//       clock_in_lcd_gl(0,4);
//       if (time_beg_str <= 0)
//         {
//           time_beg_str = 110;//45
//           simvol++;
//           if (simvol == str_length+10) simvol = 0;
//         }
//       for (a = 0, i = simvol; a < 10 ; i++, a++)
//         {
//           if ((i > 9) && (i < str_length+10))
//           lcd_buffer[1][a] = *(ukaz_na_str+i-10);//str[i-9];
//         }
//     }
// }
//---------------------------------------------------------------------------------------------------------------
// void indikator_manager(void)
// {
// unsigned char i;
//   if (!time_view_buf[0])
//     {
//       for(i = 0; i < (Nreg_st/2) ; i++)
//         {
//           if (str0 == i)
//             {
//               if (i == (Nreg_st/4-1)) str0 = 0; 
//               else str0++;
//               if (regim_state_time[i*2+1] != 0)
//                 {
//                   time_view_buf[0] = regim_state_time[i*2+1];
//                   viewstate[0] = regim_state_time[i*2];
//                 //  simvoll = 0;
//                 }
//               break;
//             }
//         }
//     }
//   if (!time_view_buf[1])
//     {
//       for(i = 0; i < (Nreg_st/2) ; i++)
//         {
//           if (str1 == i)
//             {
//               if (i == (Nreg_st/2-1)) str1 = 4; 
//               else str1++;
//               if (regim_state_time[i*2+1] != 0)
//                 {
//                   time_view_buf[1] = regim_state_time[i*2+1];
//                   viewstate[1] = regim_state_time[i*2];
//                 //  simvoll = 0;
//                 }
//               break;
//             }
//         }
//     }
//   view_in_indikator(0,viewstate[0]);
//   view_in_indikator(1,viewstate[1]);
// }
// //------------------------------------------------------------------------------------------------------------------
// void view_in_indikator(unsigned char ind_str, unsigned char view)
// {
//   if      (view == 0) num_month_year_day(ind_str);//num_month_year day
//   else  if  (view == 1) clock_day_str(ind_str);//hour-min day
//   else  if  (view == 2) num_month_year(ind_str);//num_month_year 
//   else  if  (view == 3) clock_in_lcd1(ind_str);//hour-min-sek
//   else  if  (view == 4) num_month_year_day_sdvig(ind_str);//num_month_year day бегущая строка
//   else  if  (view == 5) clock_day_str_sdvig(ind_str);//hour-min day бегущая строка
//   else  if  (view == 6) num_month_year_sdvig(ind_str);//num_month_year бегущая строка
//   else  if  (view == 7) clock_in_lcd1_sdvig(ind_str);//hour-min-sek бегущая строка
//   else  if  (view == 8) num_month_year_day_sdvig_2(ind_str);//num_month_year day бегущая строка быстрее
//   else  if  (view == 9) clock_day_str_sdvig_2(ind_str);//hour-min day бегущая строка быстрее
//   else  if  (view == 10)  num_month_year_sdvig_2(ind_str);//num_month_year бегущая строка быстрее
//   else  if  (view == 11)  clock_in_lcd1_sdvig_2(ind_str);//hour-min-sek бегущая строка быстрее
//   else  err_eprom();
// }
//---------------------------------------------------------------------------------------------------------------------
