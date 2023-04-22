#include <SoftwareSerial.h>
#include <Wire.h>
#include <RTClib.h>
// servo 
#include <Servo.h>
// telegram

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   
#include <ArduinoJson.h>



#define SOFTRX 17
#define SOFTTX 16

SoftwareSerial SerialX(SOFTRX, SOFTTX);



// libreria per data e ora 
RTC_DS3231 rtc;
static const int servoPin = 13;
Servo servo1;



// aggiunta password e id wi-fi
const char* ssid = "iPhone";
const char* password = "andrea01";

// Initialize Telegram BOT
#define BOTtoken "6079102505:AAEUnmQ9noONaKM22EZ0CZKTM3ZJO54FvD4"  // your Bot Token (Get from Botfather)




// sensore distanza

#define TRIG_ENTRY 21
#define ECHO_ENTRY 22

#define TRIG_EXIT 12
#define ECHO_EXIT 14
#define N_PARCHEGGI 1




struct Cliente{
  String nome;
  long date;
  String id;
};



#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// controlla se c'e' un messaggio ogni 1 s
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 2;
bool ledState = LOW;
Cliente clienti[N_PARCHEGGI];
int contatore =0;
int nPosti = 1;

//controllo per vedere se una persona e' gia dentro
int controlloNomi(String id){
  for(int i=0 ; i< contatore ; i ++){
    if(clienti[i].id == id){
        bot.sendMessage(id, "sei gia' dentro ", "");
        return 0;
    }
  }
  return 1;
  
}// controllo che prima di pagare guarda se quel id e' entrato  
int controlloDentro(String id){
  for(int i=0 ; i< contatore ; i ++){
    if(clienti[i].id == id){
        return 1;
    }
  }
  bot.sendMessage(id, "prima di pagare devi entrare !! ", "");
  return 0;
  
}



// metodo che ci permette di vedere se ci sono posti disponibili

int checkPosti( String id ,int nPosti){
  
    if(nPosti>0){
       
        return 1;
        
    }
   
  bot.sendMessage(id, "NON PUOI ENTRARE , PARCHEGGIO PIENO !!", "");

  return 0;
  
}

//controllo se la distanza dal sensore e' minore di 10 m 
int controlloIngUsc(String id,float dist ){
  if  (dist<10){
    return 1;
  }
  bot.sendMessage(id, "non sei davanti al sensore", "");
  return 0;
}

// // VEDERE SE ARRIVANO I MESSAGGI PER L'AGGIORNAMENTO POSTI 
// void messaggiArrivo(){
//   if(SerialX.availabile()){
    
//     SerialX.write(nPosti);
//   }
// }





// serve per vedere quanto e' la distanza dal ultrasuono di ingresso
float distanzaUltrasuoniIngresso(){
  float readout=0;
  float distanza=0;
  digitalWrite(TRIG_ENTRY, HIGH);
      delayMicroseconds(9);  
      digitalWrite(TRIG_ENTRY, LOW);
      readout = pulseIn(ECHO_ENTRY, HIGH);
      distanza = (float)readout/58;
      return distanza;
}
// per vedere la distanza dal ultrasuono in uscita 
float distanzaUltrasuoniUscita(){
  float readout = 0;
  float distanza =0;
  digitalWrite(TRIG_EXIT, HIGH);
      delayMicroseconds(9);  
      digitalWrite(TRIG_EXIT, LOW);
      readout = pulseIn(ECHO_EXIT, HIGH);
      distanza = (float)readout/58;
      return distanza;
}





// ci permette di ricevere messaggi dal bot di telegram
void handleNewMessages(int numNewMessages, float metriUltrasuoniIngresso , float metriUltrasuoniUscita) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
  
    String chat_id = String(bot.messages[i].chat_id);
    
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
    
  
    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "/alza la sbarra\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    // per entrare nel parcheggio
    if (text == "/entra"   && controlloIngUsc(chat_id, metriUltrasuoniIngresso)==1 && checkPosti(chat_id,nPosti)==1  && controlloNomi(chat_id)==1) {
      
      
        delay(1000);
        
        String benvenuto = "*benvenuto " + ((String)from_name);
        SerialX.write(benvenuto.c_str());

        clienti[contatore].nome = from_name;
        clienti[contatore].date = millis();
        clienti[contatore].id = chat_id;
        
        nPosti= nPosti -1;
        contatore= contatore +1;
       
        

        

        delay(1000);
        servo1.write(130);
            
    }

     if (text == "/visualizza") {
       delay(1000)
              String tariffa = "parcheggi liberi  " + ((String)nPosti) + "$\n";
              bot.sendMessage(chat_id, tariffa, "");

        delay(1000);
        
            
    }
// uscire dal parcheggio
    if (text == "/paga" && controlloIngUsc(chat_id, metriUltrasuoniUscita)==1  && controlloDentro(chat_id)==1) {
      //sbarra che si alza 
        delay(1000);
        for(i=0; i<contatore ; i++){
          if( from_name == clienti[i].nome ){
              long permanenzaSec = (millis()-clienti[i].date)/1000;
              float prezzo = permanenzaSec * 0.005; // 0.5 centesimi al secondo
              String tariffa = "il pedaggio per la sosta e' " + ((String)prezzo) + "$\n";
              bot.sendMessage(chat_id, tariffa, "");
              String tar = "#"+((String)prezzo);
              SerialX.write(tar.c_str());
              
          }
        }
        servo1.write(130);
// viene utilizzato per andare a rimuovere gli elementi 
        int nuova_dimensione = contatore - 1;
        for (int i = 0; i < contatore; i++) {
          if (clienti[i].nome == from_name) {
            for (int j = i; j < contatore - 1; j++) {
              clienti[j] = clienti[j+1];
             }
            
            break;
          }
        }
        contatore = nuova_dimensione;
        nPosti= nPosti+1;

      
      
    }
      
    
  }
}

void setup() {
  Serial.begin(9600);

  

  // comunciazione seriale 
  pinMode(SOFTTX, OUTPUT);
  pinMode(SOFTRX, INPUT);
  SerialX.begin(9600);
  
  // serve per la data 
  Wire.begin();
  rtc.begin();
  
  // Imposta la data e l'ora del RTC
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  /////////

  // ultrasuoni 
  pinMode (TRIG_ENTRY, OUTPUT);
pinMode (ECHO_ENTRY, INPUT);

  pinMode (TRIG_EXIT, OUTPUT);
pinMode (ECHO_EXIT, INPUT);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");     
    client.setTrustAnchors(&cert); 
  #endif

  
  servo1.attach(servoPin);
  servo1.write(55);
  
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    SerialX.write("@connecting to wifi ");
    // doppio 
    Serial.println("Connecting to WiFi..");
  }
  delay(2000);
  SerialX.write("@connesso");
  Serial.println(WiFi.localIP());
  
}

void loop() {

    float metriUltrasuonoIngresso = distanzaUltrasuoniIngresso();
    float metriUltrasuonoUscita = distanzaUltrasuoniUscita();
    
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  
    while(numNewMessages) {
      Serial.println("got response");
      
      handleNewMessages(numNewMessages, metriUltrasuonoIngresso,  metriUltrasuonoUscita);
  
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
     
  }
  if(metriUltrasuonoUscita  >10 &&  metriUltrasuonoIngresso > 10){
        servo1.write(55);
    }

    delay(100);
}




// optional : schermo, comunicazione tra 2 arduini , batteria 