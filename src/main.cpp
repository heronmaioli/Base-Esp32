#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <SimpleDHT.h>
#include <WebSocketsClient.h>


WiFiUDP udp;
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000); 


uint8_t chipid[6];
const char * MAC_ID;
char buffer[40]; 

SimpleDHT22 dht;
WebSocketsClient webSocket;


String hora;

char path[] = "/";
char host[] = "192.168.0.12";


unsigned long currentMillis;
unsigned long sensorReadPrevMillis = 0;
unsigned long pingWsPrevMillis = 0;
unsigned long lightPrevMillis = 0;

float temperature, humidity;



//Provide your own WiFi credentials
const char* ssid = "CLARO_=(_8^(1)";
const char* password = "EFAFB9DA";



 #define DHTPIN 26
 #define LEDPIN 14
 #define VENTPIN 25
 #define INEXPIN 33
 #define OUTEXPIN 32
 boolean lightLabel = false;

 String highHour = "05:00:00";
 String lowHour = "22:54:00";
 String lightState = "AUTO";
 boolean ventState = false;
 boolean inExaust = false;
 boolean outExaust = false;





void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	switch(type) {

		case WStype_DISCONNECTED:    
			Serial.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED: {
			Serial.printf("[WSc] Connected to url: %s\n", payload);
      char str1[20];
      char str2[20];
      strcpy (str1,"CON:");
      strcpy (str2, MAC_ID);
      strncat (str1, str2, 15);
      puts (str1);			
			webSocket.sendTXT(str1);
		}    
			break;

	  case WStype_TEXT:

			Serial.printf("%s\n", payload);
      String str = (char*)payload;

      if (str == "LON") {
        lightState = "ON";
        digitalWrite(LEDPIN, 0);
        lightLabel = true;
      }
      if (str == "LOF") {
        lightState = "OFF";
        digitalWrite(LEDPIN, 1);
        lightLabel = false;
      }
      if (str == "LAU") {
        lightState = "AUTO";
      }			
      if (str == "VON") {
        ventState = true;
        digitalWrite(VENTPIN, 1);
      }
      if (str == "VOF") {
        ventState = false;
        digitalWrite(VENTPIN, 0);
      }
      if (str == "ION") {
        inExaust = true;
        digitalWrite(INEXPIN, 1);
      }
      if (str == "IOF") {
        inExaust = false;
        digitalWrite(INEXPIN, 0);
      }
      if (str == "OON") {
        outExaust = true;
        digitalWrite(OUTEXPIN, 1);
      }
      if (str == "OOF") {
        outExaust = false;
        digitalWrite(OUTEXPIN, 0);
      }
    	break;	  
    }

}

//___________________________________________________

void bootCheck() {

char boardIdJSON[128];

if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("https://jsonplaceholder.typicode.com/posts");
    http.addHeader("COntent-Type", "application/json");

    const size_t CAPACITY = JSON_OBJECT_SIZE(2);
    StaticJsonDocument<CAPACITY> doc;
    JsonObject object = doc.to<JsonObject>();   
    
    object["boardId"] = MAC_ID;

    serializeJson(doc, boardIdJSON);
    int httpCode = http.POST(String(boardIdJSON));
    if (httpCode > 0) {

      String payload = http.getString();
      Serial.println("\nStatuscode: " + String(httpCode));
      Serial.println(payload);

      DynamicJsonDocument doc1(1024);
      deserializeJson(doc1, payload);
      // highHour = doc1["highHour"];
      // lowHour   = doc1["lowHour"];
      // lightState   = doc1["lightState"];
      // ventState    = doc1["ventState"];
      // outExaust   = doc1["outExaust"];
      // intExaust   = doc1["intExaust"];
      http.end();
    }
  }
 };
 
//___________________________________________________

void lightTimer() {

unsigned long lightInterval = 2000;

 if (currentMillis >= (lightPrevMillis + lightInterval)){
  lightPrevMillis = currentMillis;
  String nowTime = ntp.getFormattedTime(); 


 if (nowTime >= highHour && nowTime <= lowHour && lightLabel != true && lightState == "AUTO"){
    Serial.println("Turn ON auto");
    digitalWrite(LEDPIN, 0);
    lightLabel = true;
 }

 if (nowTime >= lowHour && lightLabel != false && lightState == "AUTO") {
    Serial.println("Turn OFF auto");
    digitalWrite(LEDPIN, 1);   
    lightLabel = false;
 }

if (lightState == "ON" && lightLabel != true){
  Serial.println("Turn ON manual");
  digitalWrite(LEDPIN, 0);
  lightLabel = true;
}

if (lightState == "OFF" && lightLabel != false){
  Serial.println("Turn OFF manual");
  digitalWrite(LEDPIN, 1);
  lightLabel = false;
}
}};

//___________________________________________________

 void readSensors() { 

   char jsonOutput[128];
   unsigned  long readInterval = 20000;

  if (currentMillis >= (sensorReadPrevMillis + readInterval)){
    sensorReadPrevMillis = currentMillis;
    dht.read2(DHTPIN, &temperature, &humidity, NULL);

    HTTPClient http;
    http.begin("https://jsonplaceholder.typicode.com/posts/1");
    http.addHeader("COntent-Type", "application/json");

    const size_t CAPACITY = JSON_OBJECT_SIZE(3);
    StaticJsonDocument<CAPACITY> doc;
    JsonObject object = doc.to<JsonObject>();   
    
    object["boardId"] = MAC_ID;
    object["temperature"] = temperature;
    object["humidity"] = humidity;

    serializeJson(doc, jsonOutput);
    int httpCode = http.PUT(String(jsonOutput));
    if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("\nStatuscode: " + String(httpCode));
    Serial.println(payload);
    http.end();
  }  
}
 };

//___________________________________________________

void pingMessage() {
   unsigned  long socketInterval = 4000;

  if ((WiFi.status() == WL_CONNECTED) && currentMillis >= (pingWsPrevMillis + socketInterval) ) {
     pingWsPrevMillis = currentMillis;      
    webSocket.sendTXT("ping");    
  }  
}

//___________________________________________________

void setup(void) {
  //For displaying the joke on Serial Monitor
  Serial.begin(9600);
  pinMode(DHTPIN,INPUT);
  pinMode(LEDPIN,OUTPUT);
  pinMode(VENTPIN,OUTPUT);
  pinMode(INEXPIN,OUTPUT);
  pinMode(OUTEXPIN,OUTPUT);

  
  //Initiate WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");  

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());    

  esp_efuse_read_mac(chipid);    
  sprintf(buffer, "%02x%02x%02x%02x%02x%02x",chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]);
  MAC_ID = buffer;

  delay(1000);
  bootCheck();     

  delay(1000);
  
  webSocket.begin("192.168.0.12", 8080, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(4000);

  
  ntp.begin();
  ntp.forceUpdate();   
    

    delay(500);
    
}
 
 //__________________________________________________

void loop() {  

  webSocket.loop();
  readSensors();
  pingMessage();
  lightTimer();
  currentMillis=millis();  
}