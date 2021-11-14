#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <SimpleDHT.h>

WiFiUDP udp;
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000); 

uint8_t chipid[6];
const char * MAC_ID;
char buffer[40]; 

SimpleDHT22 dht;

String hora;



//Provide your own WiFi credentials
const char* ssid = "CLARO_=(_8^(1)";
const char* password = "EFAFB9DA";

#define LED 2 
#define DHTPIN 26

//____________________________________________________

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

   float temperature, humidity;
   dht.read2(DHTPIN, &temperature, &humidity, NULL);  

   char jsonOutput[128];
  


   if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("https://jsonplaceholder.typicode.com/posts/1");
    http.addHeader("COntent-Type", "application/json");

    const size_t CAPACITY = JSON_OBJECT_SIZE(3);
    StaticJsonDocument<CAPACITY> doc;
    JsonObject object = doc.to<JsonObject>();   
    
    object["boardId"] = id;
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
}

//____________________________________________________

void setup(void) {
  //For displaying the joke on Serial Monitor
  Serial.begin(9600);
  pinMode(LED,OUTPUT);
  
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
  readSensors(MAC_ID);
  delay(2000);
}