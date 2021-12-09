#include <SocketIoClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>



const char* ssid = "CLARO_=(_8^(1)";
const char* password = "EFAFB9DA";

unsigned long currentMillis;
unsigned long pingWsPrevMillis = 0;

SocketIoClient webSocket;


void teste(const char* payload, size_t length) {
  Serial.println(payload);
}

void pingMessage(const char* payload, size_t length) {    
    webSocket.emit("PONG");   
  
}

void bootcheck(const char* payload, size_t length) {    
    webSocket.emit("join", "123456");    
}


void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);  }
  Serial.print("");
  Serial.println("WiFi connected");
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());
  
   webSocket.begin("gentle-savannah-77998.herokuapp.com", 80);

   webSocket.on("message", teste);

   webSocket.on("connect", bootcheck);   

   webSocket.on("disconnected", teste);

  
}

void loop(){  
  webSocket.loop();   
  delay(500);
    
  }

