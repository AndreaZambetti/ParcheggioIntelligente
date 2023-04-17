#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define SDA 13                    
#define SCL 14                    


LiquidCrystal_I2C lcd(0x27,20,4); 


#include <SoftwareSerial.h>
// aggiunta led 
#include "Timer.h" 
Timer timer; 

float number = 0; 
int num1 = 0;
int num2 = 0;
int num3 = 0;
int num4 = 0;
int timer_event = 0;

int D1 = 22;
int D2 = 21;
int D3 = 19;
int D4 = 4;

int latchPin = 5;
int clockPin = 18;
int dataPin = 23;

int count = 0;
int numbers[4] ;
int cathodePins[] = {22, 21, 19, 4};

byte table[10] {B11111100, B01100000, B11011010, B11110010, B01100110, B10110110, B10111110, B11100000, B11111110, B11110110};
//
#define SOFTRX 17
#define SOFTTX 16

int nPostiDisponibili =5;
int tempo = 20000;
int set =0;



SoftwareSerial SerialS(SOFTRX, SOFTTX);

void setup() {  
  pinMode(SOFTTX, OUTPUT);
  pinMode(SOFTRX, INPUT);
  
  delay(3000);
  
  Serial.begin(9600);
  Serial.println("Pronto a ricevere da arduino 1");
  
  SerialS.begin(9600);


// aggiunta led  
  pinMode(D4, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  digitalWrite(D4, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D1, HIGH);
  
  //

  //lcd 
  Wire.begin(SDA, SCL);           
    lcd.init();                     
    lcd.backlight();                
   
    lcd.setCursor(0, 0);
    // print the number of seconds since reset:
    lcd.print("PARCHEGGIO ESSELUNGA ");
    lcd.setCursor(0, 1);
     
  
}

void loop() {


  if (  set==0 &&  millis() >= tempo ){
    Serial.print("POSTI LIBERI  ");
    Serial.println(nPostiDisponibili);
    clearAll();
    lcd.setCursor(0, 1);
    lcd.print(" POSTI LIBERI  ");
    lcd.print(nPostiDisponibili);
    set=1;
  }

  
 

//  if (SerialS.available() ) {
//    Serial.println(SerialS.readString());
//    set=0;
//    tempo = millis() + 10000;

//   }


timer.update();
if(SerialS.available()) {
    String  stringa = SerialS.readString();
    char inChar = stringa.charAt(0);
      switch (inChar) {
        case '@':
          // stampiamo sul lcd 
          Serial.println(stringa.substring(1));
          lcd.clear();
          lcd.setCursor(0, 3);
          lcd.print(stringa.substring(1));
          
          set=0;
          tempo = millis() + 30000;
          break;
        case '#':
         // valore numerico stampare sul 7 segmenti
         
          timer.stop(timer_event); 
          screenOff();
         Serial.println(stringa.substring(1));
         number = (float)stringa.substring(1).toFloat(); 
         nPostiDisponibili = nPostiDisponibili +  1;
         // prova 
          separate(number);
          timer_event = timer.every(1, Display); 
          
          set=0;
          tempo = millis();
          break;
        case '*':  // scrittura lcd
          Serial.println(stringa.substring(1)); 
          nPostiDisponibili = nPostiDisponibili-1;
          //lcd
          clearAll();
           lcd.setCursor(0, 1);
          lcd.print(stringa.substring(1));
          lcd.setCursor(0, 2);
          lcd.print("proseguire avanti !!");

          set=0;
          tempo = millis() + 10000;
          break;
        default:
          // Carattere iniziale sconosciuto
          Serial.println("Errore: carattere iniziale sconosciuto");
          break;
      }

      
    
  }






}


/// aggiunta led 
void separate(long num) { 
  num1 = num / 1000;
  numbers[0] = num1;
  int num1_remove = num - (num1 * 1000);
  num2 = num1_remove / 100;
  numbers[1] = num2;
  int num2_remove = num1_remove - (num2 * 100);
  num3 = num2_remove / 10;
  numbers[2] = num3;
  num4 = num2_remove - (num3 * 10);
  numbers[3] = num4;
}

void Display() {
  screenOff(); 
  digitalWrite(latchPin, LOW); 
  shiftOut(dataPin, clockPin,LSBFIRST, table[numbers[count]]); 
  digitalWrite(cathodePins[count], LOW); 
  digitalWrite(latchPin, HIGH); 
  count++; 
  if (count == 4) { 
    count = 0;
  }
}

void screenOff() { 
  digitalWrite(D4, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D1, HIGH);
}


void clearAll(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PARCHEGGIO ESSELUNGA");

  lcd.setCursor(0, 3);
  lcd.print("connesso");

  

  
}



