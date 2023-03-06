// servo 
#include <Servo.h>
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

#define TRIG 21
#define ECHO 22

long readout;
float distanza;


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

//distanza ultrasuoni 
float distanzaUltrasuoni(){
  digitalWrite(TRIG, HIGH);
      delayMicroseconds(9);  //impulso da 10us
      digitalWrite(TRIG, LOW);
      readout = pulseIn(ECHO, HIGH);
      distanza = (float)readout/58;
      return distanza;
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages, float metriUltrasuoni) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    // if (chat_id != CHAT_ID){
    //   bot.sendMessage(chat_id, "Unauthorized user", "");
    //   continue;
    // }

    
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
    
    

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "/alza la sbarra\n";
      bot.sendMessage(chat_id, welcome, "");

  
    }


      

    if (text == "/paga" && metriUltrasuoni<10) {
      //sbarra che si alza 
        delay(10000);
        servo1.write(10);
      Serial.println(chat_id);
      Serial.println(from_name);
      
      
      
    }

    if (text == "/iscrizione") {
      
      
      
      
    }
      
    
  }
}

void setup() {
  Serial.begin(115200);

  // ultrasuoni 
  pinMode (TRIG, OUTPUT);
pinMode (ECHO, INPUT);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
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
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    float metriUltrasuono = distanzaUltrasuoni();
    while(numNewMessages) {
      Serial.println("got response");
      
      handleNewMessages(numNewMessages, metriUltrasuono);
  
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
     if(metriUltrasuono >10){
        servo1.write(90);
      }
  }
}



// aggiungere il secondo ultrasuoni , paga , gestione della data e ora e il costo all'ora per persona 
// optional : schermo, comunicazione tra 2 arduini , batteria 