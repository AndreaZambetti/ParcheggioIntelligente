#include <SoftwareSerial.h>
int v = 0;

#define SOFTRX 17
#define SOFTTX 16

SoftwareSerial SerialX(SOFTRX, SOFTTX);


// servo 
#include <Servo.h>
#include <Wire.h>
#include <RTClib.h>
// libreria per data e ora 
RTC_DS3231 rtc;
static const int servoPin = 13;
Servo servo1;
// telegram

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "iPhone";
const char* password = "andrea01";

// Initialize Telegram BOT
#define BOTtoken "6079102505:AAEUnmQ9noONaKM22EZ0CZKTM3ZJO54FvD4"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you


// sensore distanza

#define TRIG_ENTRY 21
#define ECHO_ENTRY 22

#define TRIG_EXIT 12
#define ECHO_EXIT 14
#define N_PARCHEGGI 3


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

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 2;
bool ledState = LOW;
Cliente clienti[N_PARCHEGGI];
int contatore =0;
int nPosti = 3;

// questo controllo sarebbe da fare con gli id 
int controlloNomi(String id){
  for(int i=0 ; i< contatore ; i ++){
    if(clienti[i].id == id){
        return 0;
    }
  }return 1;
  
}



//distanza ultrasuoni 
float distanzaUltrasuoniIngresso(){
  float readout=0;
  float distanza=0;
  digitalWrite(TRIG_ENTRY, HIGH);
      delayMicroseconds(9);  //impulso da 10us
      digitalWrite(TRIG_ENTRY, LOW);
      readout = pulseIn(ECHO_ENTRY, HIGH);
      distanza = (float)readout/58;
      return distanza;
}

float distanzaUltrasuoniUscita(){
  float readout = 0;
  float distanza =0;
  digitalWrite(TRIG_EXIT, HIGH);
      delayMicroseconds(9);  //impulso da 10us
      digitalWrite(TRIG_EXIT, LOW);
      readout = pulseIn(ECHO_EXIT, HIGH);
      distanza = (float)readout/58;
      return distanza;
}





// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages, float metriUltrasuoniIngresso , float metriUltrasuoniUscita) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
    
  
    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "/alza la sbarra\n";
      bot.sendMessage(chat_id, welcome, "");
    }


    if (text == "/entra"   && metriUltrasuoniIngresso<10 && nPosti>0  && controlloNomi(chat_id)==1) {
      
      //sbarra che si alza 
        delay(1000);
        servo1.write(10);

        clienti[contatore].nome = from_name;
        clienti[contatore].date = millis();
        clienti[contatore].id = chat_id;
        
        nPosti= nPosti -1;
        contatore= contatore +1;
        for(i=0; i<contatore ; i++){
        Serial.print(clienti[i].nome);
        Serial.print("data");
        Serial.println(clienti[i].date);

        SerialX.write(5);
 
        }
            
    }

    if (text == "/paga" && metriUltrasuoniUscita <10) {
      //sbarra che si alza 
        delay(1000);
        servo1.write(10);
        for(i=0; i<contatore ; i++){
          if( from_name == clienti[i].nome ){
              long permanenzaSec = (millis()-clienti[i].date)/1000;
              float prezzo = permanenzaSec * 0.005; // 0.5 centesimi al secondo
              String tariffa = "il pedaggio per la sosta e' " + ((String)prezzo) + "$\n";
              bot.sendMessage(chat_id, tariffa, "");
              SerialX.write(50);
          }
        }
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
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  
  servo1.attach(servoPin);
  servo1.write(90);
  
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
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
        servo1.write(90);
    }

    delay(100);
}




// optional : schermo, comunicazione tra 2 arduini , batteria 