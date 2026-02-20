
#define LED1 PE6

void setup() {
  pinMode(LED1, OUTPUT);
  // pinMode(PIN_PC7, OUTPUT);  //Зуммер
  }

void loop() {
  // LED1 = !LED1;
  digitalWrite(LED1, HIGH);  
  delay(100);                     
  digitalWrite(LED1, LOW);    
  delay(100);  
  // beep();                    
}

// void beep() 
//   {
//     digitalWrite(PIN_PC7, HIGH);
//     delay(200);
//     digitalWrite(PIN_PC7, LOW);
//         delay(200);

//   }