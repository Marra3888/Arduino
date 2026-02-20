#include "Arduino.h"
#include "LC75847.h"
#include <SPI.h>
#include "character.h"

LC75847::LC75847() 
{

}

/*
* Function Name: begin
 * Description : Sürücünün adres ve slaveSelect pin bilgilerini alır, SPI ayarını yapar ve başlatır.
 * Input : int chipEnabledPin(sürücünün slaveSelect pininin bağlı olduğu pin no), 
 *         int address(sürücünün adresi).
 * Output : None
 */
void LC75847::begin(int chipEnabledPin, int address) {
  _chipEnabledPin = chipEnabledPin;
  _address = address;
  pinMode(_chipEnabledPin, OUTPUT);
  SPISettings(38000, MSBFIRST, SPI_MODE0);
  SPI.begin();
  //_setSymbol();
}

/*
* Function Name: textSize
 * Description : Lcd'ye gönderilen metnin karakter uzunluğunu hesaplar ve _textLenght global değişkenine girer. 
 * Input : char array
 * Output : None
 */
void LC75847::_textSize(char text[]) {
  _textLenght = 0;
  while (text[_textLenght] != '\0') {
    _textLenght++;
  }
}

/*
* Function Name: speed
 * Description : Kullanıcının lcd animasyon hızını girebilmesini sağlar.  
 * Input : int
 * Output : None
 */
void LC75847::speed(int textSpeed) { _textSpeed = textSpeed; }

/*
* Function Name: clear
 * Description : Karakter datalarını taşıyan _container dizisini, metnin ekrandaki dönüş sayısını tutan _loopNumber int değişkenini ve 
 *               animasyon sayısını tutan _sScroll int değişkenlerini sıfırlar ve ekrana boş bir container basar.
 *               (Sadece karakterleri ekrandan siler, semboller silinmez.)
 * Input : None
 * Output : None
 */
void LC75847::clear() 
{
  memset(Display_Data_0, 0, 14);
  memset(Display_Data_1, 0, 14);
  memset(Display_Data_2, 0, 14);
  memset(Display_Data_3, 0, 14);
  _print();
}

/*
* Function Name: reset
 * Description : Ekrana gönderilen dataları taşıyan tüm değişkenler ve döngü değişkenleri sıfırlanır.
 *               (Karakterler ve semboller dahil.)
 * Input : None
 * Output : None
 */
void LC75847::reset() {
/*  memset(_screen, 0, 15);
  memset(_container, 0, _containerSize);
  memset(_symbols, 0, 5);
  memset(_symbolChart, false, 4);
  _symbolInt = false;
  _symbolMute = false;
  _symbolBluetooth = false;
  _symbolTrack = false;
  _symbolDisc = false;
  _symbolRdm = false;
  _symbolRpt = false;
  _symbolMp3 = false;
  _symbolFolderIcon = false;
  _symbolMusic = false;
  _symbolAudioFileIcon = false;
  _symbolBookIcon = false;
  _sScroll = 0;
  _tScroll = 0;
  _loopNumber = 0;
  _textLenght = 0;
  _textSpeed = 300;
  _volumeLevel = 0;*/
  _print();
  }

/*
* Function Name: display
 * Description : LCD POWER SAVING(BU DATA) mode is turn on with 0. 
 * Input : None
 * Output : None
 */
void LC75847::display() 
{
//  bitWrite(_symbols[4], 1, 0);
  _print();
}

/*
* Function Name: noDisplay
 * Description : LCD POWER SAVING(BU DATA) mode is turn on with 1. 
 * Input : None
 * Output : None
 */
void LC75847::noDisplay() {
  bitWrite(_symbols[4], 1, 1);
  _print();
  }

/*
* Function Name: volumeStartValue
 * Description : Lcd volume göstergelerine yapılacak girişin, başlangıç değerini belirler (Default = 0)
 * Input : int startValue
 * Output : None
 */
void LC75847::volumeStartValue(int startValue){
  _volumeStartValue = startValue;
}

/*
* Function Name: volumeEndValue
 * Description : Lcd volume göstergelerine yapılacak girişin, bitiş değerini belirler. (Default = 8)
 * Input : int endValue
 * Output : None
 */
void LC75847::volumeEndValue(int endValue){
  _volumeEndValue = endValue;
}

/*
* Function Name: levelDirection
 * Description : Lcd volume göstergelerinin animasyon yönünü belirler. (Default = LEFT)
 * Input : Direction d
 * Output : None
 */
void LC75847::levelDirection(enum direction d){
  _levelDirection = d;
}

/*
* Function Name: volumeLeftLevel
 * Description : Lcd sol volume göstergelerinin değerini belirler. Değer aralığı volumeStartValue ve volumeEndValue metodları ile belirlenebilir.
 * Input : int levelValue (Sol volume seviye değeri).
 * Output : None
 */
void LC75847::volumeLevel(int levelValue) {
  float convertedLevelValue =
      ceil((levelValue - _volumeStartValue) /
           ceil((_volumeEndValue - _volumeStartValue) / 7));
  
  if (_levelDirection == IN) {
    _symbols[3] = B11111100;
    _symbols[3] <<= 7 - (int)convertedLevelValue;
    // bitWrite(_symbols[1], 7, _iconRock);
    _volumeLevel = (int)convertedLevelValue;
    if (_volumeLevel == 0) {
      bitWrite(_symbols[2], 0, 0);
    } else {
      bitWrite(_symbols[2], 0, 1);
    }
  }

  if (_levelDirection == OUT) {
    _symbols[3] = B11111100;
    _symbols[2] = B00000001;
    _symbols[3] >>= 7 - (int)convertedLevelValue;
    _symbols[2] >>= 7 - (int)convertedLevelValue;
    // bitWrite(_symbols[1], 7, _iconRock);
    _volumeLevel = (int)convertedLevelValue;
    if (_volumeLevel == 0) {
      bitWrite(_symbols[3], 2, 0);
    } else {
      bitWrite(_symbols[3], 2, 1);
    }
  }
  _print();
}

/*
* Function Name: sChart (Scroll Chart)
 * Description : Lcd chart sembollerini animasyonlu olarak yakar.
 * Input : boolean chartValue (0 = Animasyon kapalı, 1 = Animasyon açık).
 * Output : None
 */
void LC75847::sChart(boolean chartValue) {
  if (chartValue) {
    if (_chartScroll > 3) {
      _chartScroll = 1;
    }
    memset(_symbolChart, false, 4);
    _symbolChart[0] = true;
    _symbolChart[_chartScroll] = true;
    _chartScroll++;
    _setSymbols();
  _print();  
  }
}

/*
* Function Name: textLoopCount
 * Description : Girilen metnin ekrandaki tekrar sayısını döndürür.
 * Input : None
 * Output : int
 */
int LC75847::textLoopCount() { return _loopNumber; }

/*
* Function Name: _textLoop
 * Description : Gösterilen metnin, tekrar sayısını işler.
 * Input : None
 * Output : None
 */
void LC75847::_textLoop() {
  if (_sScroll > _textLenght) {
    _sScroll = 0;
    _loopNumber++;
  }
}

/*
* Function Name: text
 * Description : Metni yazdırır.
 * Input : char text[] - Yazdırılacak metni taşıyan char dizisi.
 * Output : None
 */
void LC75847::text(char text[]) {
  _textSize(text);
    char _screenText[8];
  for (int i = 0; i < 8; i++) {
    _screenText[i] = text[i];
    if (i >= _textLenght) {
      _screenText[i] = ' ';
    }
  }
  _letters(_screenText);
  _setLetters();
  _setSymbols();
  _print();
}

/*
 * Function Name: sText (Scroll Text)
 * Description : Metni soldan sağa doğru kaydırarak yazdırır.
 * Input : char text[] - Yazdırılacak metni taşıyan char dizisi.
 * Output : None
 */
void LC75847::sText(char text[]) {
  _textSize(text);
  char _screenText[8];
  for (int i = 0; i < 8; i++) {
    //int c = text[i + _tScroll] - 32;
    _screenText[i] = text[i + _tScroll];
    if ((i + _tScroll) >= _textLenght) {
      _screenText[i] = ' ';
    }
  }
  if (_tScroll == _textLenght) {
    _tScroll = 0;
  } else {
    _tScroll++;
  }
  _letters(_screenText);
  _setLetters();
  _textLoop();
  _setSymbols();
  _print();
}

/*
 * Function Name: _setSymbols
 * Description : Sembol durumlarını, _screen byte dizisine işler.
 * Input : None
 * Output : None
 */
void LC75847::_setSymbols() 
{
  bitWrite(_screen[0], 7, _symbolChart[0]);
  bitWrite(_screen[0], 6, _symbolChart[1]);
  bitWrite(_screen[1], 0, _symbolChart[2]);
  bitWrite(_screen[2], 7, _symbolChart[3]);
  bitWrite(_screen[3], 1, _symbolInt);
  bitWrite(_screen[3], 0, _symbolMute);
  bitWrite(_screen[5], 2, _symbolBluetooth);
  bitWrite(_screen[5], 1, _symbolTrack);
  bitWrite(_screen[7], 3, _symbolDisc);
  bitWrite(_screen[7], 2, _symbolRdm);
  bitWrite(_screen[9], 4, _symbolRpt);
  bitWrite(_screen[9], 3, _symbolMp3);
  bitWrite(_screen[11], 5, _symbolFolderIcon);
  bitWrite(_screen[11], 4, _symbolMusic);
  bitWrite(_screen[13], 6, _symbolAudioFileIcon);
  bitWrite(_screen[13], 5, _symbolBookIcon);
//  bitSet(value, bit);
//  bitClear();
}

/*
 * Function Name: _setLetters
 * Description : Girilen metnin lcd boarduna özgü çevrilmiş datalarını, SPI byte data paketlerine uygun hale getirmek için,
 *               gerekli bit kaydırma işlemlerini yapar. Sonuç _screen byte dizisine atanır.
 * Input : None
 * Output : None
 */
void LC75847::_setLetters() {
  byte j = 1;
  _n = 0;
  for (int i = 0; i <= 14; i += 2) {
    _screen[i] = (_container[_n] << j) | (_container[_n + 1] >> (8 - j));
    if (i < 2) {
      _screen[i + 1] = _container[_n + 1] << j;
    } else if (i >= _containerSize) {
      _screen[i] = _container[_n + 1];
      _screen[i + 1] = 0;
    } else {
      _screen[i + 1] =
          (_container[_n + 1] << j) | (_container[_n + 2] >> (7 - j));
    }
    if ((_n + 2) > _containerSize) {
      _n = 0;
      _screen[i + 1] =
          _container[_containerSize] << j | _container[0] >> (7 - j);
    } else {
      _n += 2;
    }
    j++;
  }
  _sScroll++;
}

/*
 * Function Name: _print
 * Description : Hazırlanan _address int değişkenini, _screen ve _symbols byte
 *               dizilerini Arduino SPI kütüphanesini kullanarak sürücüye gönderir. 
 * Input : byte pScreen[] 
 * Output : None
 */
void LC75847::_print() 
{
  digitalWrite(_chipEnabledPin, LOW);
  // Address Data (A1- A8)
  SPI.transfer(_address);
  digitalWrite(_chipEnabledPin, HIGH);

  // Character Segment Data (D1 - D108 + "0 0 0 0 0 0 P0 P1 P2 P3 CT0 CT1 CT2 DR DT FC SC BU (0 0)") 128 BIT
  for (byte i = 0; i < 16; i++) 
  {
    SPI.transfer(Display_Data_0[i]);
  }

  digitalWrite(_chipEnabledPin, LOW);
  SPI.transfer(_address);
  digitalWrite(_chipEnabledPin, HIGH);  

  // Character Segment Data (D109 - D212) 104 bit (Fixet Data) 22BIT (0 1) 2BIT = 128 BIT
  for (byte i = 0; i < 16; i++) 
  {
    SPI.transfer(Display_Data_1[i]);
  }

  digitalWrite(_chipEnabledPin, LOW);
  SPI.transfer(_address);
  digitalWrite(_chipEnabledPin, HIGH);
  
  // Character Segment Data (D213 - D316) 104 bit (Fixet Data) 22BIT (1 0) 2BIT = 128 BIT
  for (byte i = 0; i < 16; i++) 
  {
    SPI.transfer(Display_Data_2[i]);
  }  
      
  digitalWrite(_chipEnabledPin, LOW);
  SPI.transfer(_address);
  digitalWrite(_chipEnabledPin, HIGH);
  
  // Character Segment Data (D317 - D420) 104 bit (Fixet Data) 22BIT (1 0) 2BIT = 128 BIT
  for (byte i = 0; i < 16; i++) 
  {
    SPI.transfer(Display_Data_3[i]);
  }  
  digitalWrite(_chipEnabledPin, LOW);
  delay(_textSpeed);
}


/*
 * Function Name: symbol
 * Description : Kullanıcının istediği sembolleri yakar.
 * Input : enum SymbolName name(sembol tanımı), 
 *         boolean status(sembolün görünürlüğü, 0 = Kapalı, 1 = Açık)
 * Output : None
 */
void LC75847::symbol(enum symbolName name, boolean status) 
{
  switch (name) 
  {
  case CHART_DOT:
    _symbolChart[0] = status;
    bitWrite(_screen[0], 7, status);
    break;
  case CHART_1:
    _symbolChart[1] = status;
    bitWrite(_screen[0], 6, status);
    break;
  case CHART_2:
    _symbolChart[2] = status;
    bitWrite(_screen[1], 0, status);
    break;
  case CHART_3:
    _symbolChart[3] = status;
    bitWrite(_screen[2], 7, status);
    break;

  case INT:
    _symbolInt = status;
    bitWrite(_screen[3], 0, status);
    break;
  case MUTE:
    _symbolMute = status;
    bitWrite(_screen[3], 1, status);
    break;
  case BLUETOOTH:
    _symbolBluetooth = status;
    bitWrite(_screen[5], 2, status);
    break;
  case TRACK:
    _symbolTrack = status;
    bitWrite(_screen[5], 1, status);
    break;
  case DISC:
    _symbolDisc = status;
    bitWrite(_screen[7], 3, status);
    break;
  case RDM:
    _symbolRdm = status;
    bitWrite(_screen[7], 2, status);
    break;
  case RPT:
    _symbolRpt = status;
    bitWrite(_screen[9], 4, status);
    break;
  case MP3:
    _symbolMp3 = status;
    bitWrite(_screen[9], 3, status);
    break;
  case FOLDER_ICON:
    _symbolFolderIcon = status;
    bitWrite(_screen[11], 5, status);
    break;
  case MUSIC:
    _symbolMusic = status;
    bitWrite(_screen[11], 4, status);
    break;
  case AUDIO_FILE_ICON:
    _symbolAudioFileIcon = status;
    bitWrite(_screen[13], 6, status);
    break;
  case BOOK_ICON:
    _symbolBookIcon = status;
    bitWrite(_screen[13], 5, status);
    break;
  case LOUD:
    bitWrite(_symbols[0], 7, status);
    break;
  case PERSON_ICON:
    bitWrite(_symbols[0], 6, status);
    break;
  case CD_ICON:
    bitWrite(_symbols[0], 5, status);
    break;
  case PTY:
    bitWrite(_symbols[0], 4, status);
    break;
  case AF:
    bitWrite(_symbols[0], 3, status);
    break;
  case DOT:
    bitWrite(_symbols[0], 2, status);
    break;
  case EON:
    bitWrite(_symbols[0], 1, status);
    break;
  case TA:
    bitWrite(_symbols[0], 0, status);
    break;
  case TP:
    bitWrite(_symbols[1], 7, status);
    break;
  case EQ:
    bitWrite(_symbols[1], 6, status);
    break;
  case POP:
    bitWrite(_symbols[1], 5, status);
    break;
  case FLAT:
    bitWrite(_symbols[1], 3, status);
    break;
  case EQ_BORDER:
    bitWrite(_symbols[1], 2, status);
    break;
  case ROCK:
    bitWrite(_symbols[1], 1, status);
    break;
  case CLAS:
    bitWrite(_symbols[1], 0, status);
    break;

  case VOLUME_1:
    bitWrite(_symbols[2], 0, status);
    break;
  case VOLUME_2:
    bitWrite(_symbols[3], 7, status);
    break;
  case VOLUME_3:
    bitWrite(_symbols[3], 6, status);
    break;
  case VOLUME_4:
    bitWrite(_symbols[3], 5, status);
    break;
  case VOLUME_5:
    bitWrite(_symbols[3], 4, status);
    break;
  case VOLUME_6:
    bitWrite(_symbols[3], 3, status);
    break;
  case VOLUME_7:
    bitWrite(_symbols[3], 2, status);
    break;
  default:;
    break;
  }
  _print();
}

/*
 * Function Name: volumeChart
 * Description : Volume sembollerinin integer değerlerle yanıp-sönmesini sağlar.
 * Input : int volumeChartNo(volume sembol numarası), 
 *         boolean status(sembolün görünürlüğü, 0 = Kapalı, 1 = Açık)
 * Output : None
 */
void LC75847::volumeChart(int volumeChartNo, boolean status) {
  switch (volumeChartNo) {
  case 1:
    bitWrite(_symbols[2], 0, status);
    break; // VOLUME_1
  case 2:
    bitWrite(_symbols[3], 7, status);
    break; // VOLUME_2
  case 3:
    bitWrite(_symbols[3], 6, status);
    break; // VOLUME_3
  case 4:
    bitWrite(_symbols[3], 5, status);
    break; // VOLUME_4
  case 5:
    bitWrite(_symbols[3], 4, status);
    break; // VOLUME_5
  case 6:
    bitWrite(_symbols[3], 3, status);
    break; // VOLUME_6
  case 7:
    bitWrite(_symbols[3], 2, status);
    break; // VOLUME_7

  case 8:
    _symbolChart[0] = status;
    bitWrite(_screen[0], 7, status);
    break; // CHART_DOT
  case 9:
    _symbolChart[1] = status;
    bitWrite(_screen[0], 6, status);
    break; // CHART_1
  case 10:
    _symbolChart[2] = status;
    bitWrite(_screen[1], 0, status);
    break; // CHART_2
  case 11:
    _symbolChart[3] = status;
    bitWrite(_screen[2], 7, status);
    break; // CHART_3

  default:;
    break;
  }
  _print();
}

/*
 * Function Name: _letters
 * Description : Girilen metindeki karakterlerin ascii karşılıkları ile character.h dosyasında bulunan character14SEG iki boyutlu dizisini 
 *               karşılaştırır ve aldığı değerleri _container byte dizisine aktarır. 
 * Input : char gk[](Girilen metnin ilk 8 karakteri)
 * Output : None
 */
void LC75847::_letters(char gk[]) {
  int d = 0;
  for (int i = 0; i < 8; i++) {
    int c = gk[i] - 32;
    if (c >= 0 && c <= 94) {
//      _container[d] = pgm_read_byte(&character14SEG[c][0]);
//      _container[d + 1] = pgm_read_byte(&character14SEG[c][1]);
    } else {
      _container[d] = 0;
      _container[d + 1] = 0;
    }
    d += 2;
  }
}


void LC75847::Clock_Display(byte hhour, byte lhour, byte hminute, byte lminute, bool tochka)
{
//Position 1 (Low minute)
bool  _bit = false;

  Display_Data_0[0] = character7SEG[lminute][0][0];
//  Serial.print("Number ");
//  Serial.println(lminute);
  for(byte a = 4; a <= 7; a++)
  {
    _bit = bitRead(character7SEG[lminute][0][1], a);
    (_bit) ? Display_Data_0[1] |= (1 << a) : Display_Data_0[1] &= ~(1 << a);     
  }
//  Serial.println(Display_Data_0[1], BIN);
//  Serial.print("   ");
//  Serial.println(Display_Data_0[2], BIN);


//Position 1 (High minute)
  for(byte a = 0; a <= 3; a++)
    {
      _bit = bitRead(character7SEG[hminute][1][0], a);
//      (_bit) ? Display_Data_0[1] |= (1 << a) : Display_Data_0[1] &= ~(1 << a); 
      bitWrite(Display_Data_0[1], a, _bit);   
    }

    
  for(byte a = 3; a <= 7; a++)
    {
      _bit = bitRead(character7SEG[hminute][1][1], a);
//      (_bit) ? Display_Data_0[2] |= (1 << a) : Display_Data_0[2] &= ~(1 << a);
      bitWrite(Display_Data_0[2], a, _bit);     
    }

    
//Position 1 (Low hour)    
  for(byte a = 0; a <= 2; a++)
    {
      _bit = bitRead(character7SEG[lhour][2][0], a);
//      (_bit) ? Display_Data_0[2] |= (1 << a) : Display_Data_0[2] &= ~(1 << a);
      bitWrite(Display_Data_0[2], a, _bit);
      if(a == 2) 
      {
//        (tochka) ?  Display_Data_0[2] |= (1 << a) : Display_Data_0[2] &= ~(1 << a);
      (tochka) ? bitWrite(Display_Data_0[2], a, true) :  bitWrite(Display_Data_0[2], a, false);
      }        
    }
  for(byte a = 1; a <= 7; a++)
    {
      _bit = bitRead(character7SEG[lhour][2][1], a);
//      (_bit) ? Display_Data_0[3] |= (1 << a) : Display_Data_0[3] &= ~(1 << a); 
      bitWrite(Display_Data_0[3], a, _bit);    
    }

        
//Position 1 (High hour) 
     
  if(hhour == 1 || hhour == 2) 
  {
    for(byte a = 5; a <= 7; a++)
      {
        _bit = bitRead(character7SEG[hhour][3][0], a);
//        (_bit) ? Display_Data_0[4] |= (1 << a) : Display_Data_0[4] &= ~(1 << a);
      bitWrite(Display_Data_0[4], a, _bit);     
      }
  }
  if(hhour == 0) 
  {
    for(byte a = 5; a <= 7; a++)
      {
        _bit = bitRead(character7SEG[hhour][3][0], a);
//        (_bit) ? Display_Data_0[4] |= (1 << a) : Display_Data_0[4] &= ~(1 << a);
      bitWrite(Display_Data_0[4], a, _bit);     
      }
  } 
        
  _print();
}

//bool LC75847::get_bit(byte num, byte position)
//{
//   bool bit = num & (1 << position);
//    return bit;
//}

