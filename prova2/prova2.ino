/*
 * 047 - Arduino cookbook
 * 
 * sketch da caricare sul secondo arduino che fa da ricevitore 
 * dati sulla seriale software ai pin 10 e 11
 * 
 * Incrociate i collegamenti! tx su rx e rx su tx
 * 
 */

#include <SoftwareSerial.h>

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
}

void loop() {


  if (  set==0 &&  millis() >= tempo ){
    Serial.print("POSTI LIBERI  ");
    Serial.println(nPostiDisponibili);
    set=1;
  }
 

 if (SerialS.available() ) {
   Serial.println(SerialS.readString());
   set=0;
   tempo = millis() + 10000;

  }
}

