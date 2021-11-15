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
float temperature, humidity;


unsigned long currentMillis;
unsigned long sensorReadPrevMillis = 0;
unsigned long sensorWsPrevMillis = 0;


//Provide your own WiFi credentials
const char* ssid = "CLARO_=(_8^(1)";
const char* password = "EFAFB9DA";



#define LED 2 
#define DHTPIN 26


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

	switch(type) {

		case WStype_DISCONNECTED:    
			Serial.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED: {
			Serial.printf("[WSc] Connected to url: %s\n", payload);
			// send message to server when Connected
			webSocket.sendTXT("Connected");
		}
			break;
		case WStype_TEXT:
			Serial.printf("%s\n", payload);
			// send message to server
			webSocket.sendTXT("message here");
			break;		
  
    }

}
//___________________________________________________

void bootCheck(const char * id) {

char jsonOutput[128];

if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("https://jsonplaceholder.typicode.com/posts");
    http.addHeader("COntent-Type", "application/json");

    const size_t CAPACITY = JSON_OBJECT_SIZE(1);
    StaticJsonDocument<CAPACITY> doc;
    JsonObject object = doc.to<JsonObject>();   
    
    object["boardId"] = id;

    serializeJson(doc, jsonOutput);
    int httpCode = http.POST(String(jsonOutput));
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("\nStatuscode: " + String(httpCode));
      Serial.println(payload);
      http.end();
    }
  }
 };
 
//___________________________________________________

 void readSensors(const char * id) { 

    char jsonOutput[128];
   unsigned  long socketInterval = 2000;   
   unsigned  long readInterval = 20000;

  if (currentMillis >= (sensorReadPrevMillis + readInterval)){
    sensorReadPrevMillis = currentMillis;
    dht.read2(DHTPIN, &temperature, &humidity, NULL);
  } 

   if ((WiFi.status() == WL_CONNECTED) && currentMillis >= (sensorWsPrevMillis + socketInterval) ) {
     sensorWsPrevMillis = currentMillis;     

    const size_t CAPACITY = JSON_OBJECT_SIZE(3);
    StaticJsonDocument<CAPACITY> doc;
    JsonObject object = doc.to<JsonObject>();      
    object["boardId"] = id;
    object["temperature"] = temperature;
    object["humidity"] = humidity;
    serializeJson(doc, jsonOutput); 
    Serial.println(temperature);
    Serial.println(humidity);
    webSocket.sendTXT(jsonOutput);    
  }  
}

//____________________________________________________

void setup(void) {
  //For displaying the joke on Serial Monitor
  Serial.begin(9600);
  pinMode(DHTPIN,INPUT);
  
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

  delay(500);
  
  webSocket.begin("192.168.0.12", 8080, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(4000);

  ntp.begin();
  ntp.forceUpdate();   
    
    esp_efuse_read_mac(chipid);    
    sprintf(buffer, "%02x%02x%02x%02x%02x%02x",chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]);
    MAC_ID = buffer;

    delay(500);
    
    bootCheck(MAC_ID);     
}
 
 //___________________________________________________

void loop() {  

  webSocket.loop();
  readSensors(MAC_ID);
  currentMillis=millis();  
}