#include <SocketIoClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <SimpleDHT.h>
#include <WiFiClient.h>

WiFiUDP udp;
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);
SocketIoClient webSocket;
SimpleDHT22 dht;

uint8_t chipid[6];
const char *MAC_ID;
char buffer[40];

unsigned long currentMillis;
unsigned long sensorReadPrevMillis = 0;
unsigned long pingWsPrevMillis = 0;
unsigned long lightPrevMillis = 0;

float temperature, humidity;

#define DHTPIN 26
#define LEDPIN 14
#define VENTPIN 25
#define INEXPIN 33
#define OUTEXPIN 32

boolean lightLabel;
String highHour = "05:00:00";
String lowHour = "23:00:00";
String lightState = "AUTO";
boolean ventState = false;
boolean inExaust = false;
boolean outExaust = false;

const char *ssid = "";
const char *password = "";

void lightTimer()
{

  unsigned long lightInterval = 2000;

  if (currentMillis >= (lightPrevMillis + lightInterval))
  {
    lightPrevMillis = currentMillis;
    String nowTime = ntp.getFormattedTime();

    if (nowTime >= highHour && nowTime <= lowHour && lightLabel != true && lightState == "AUTO")
    {
      Serial.println("Turn ON auto");
      digitalWrite(LEDPIN, 0);
      lightLabel = true;
    }

    if (lightLabel != false && lightState == "AUTO" && (nowTime >= lowHour || (nowTime >= "00:00:00" && nowTime <= highHour)))
    {
      lightLabel = false;
      Serial.println("Turn OFF auto");
      digitalWrite(LEDPIN, 1);
    }

    if (lightState == "ON" && lightLabel != true)
    {
      Serial.println("Turn ON manual");
      digitalWrite(LEDPIN, 0);
      lightLabel = true;
    }

    if (lightState == "OFF" && lightLabel != false)
    {
      Serial.println("Turn OFF manual");
      digitalWrite(LEDPIN, 1);
      lightLabel = false;
    }
  }
};

void readSensors()
{

  char jsonOutput[128];
  unsigned long readInterval = 10000;

  if (currentMillis >= (sensorReadPrevMillis + readInterval))
  {
    sensorReadPrevMillis = currentMillis;
    dht.read2(DHTPIN, &temperature, &humidity, NULL);

    const size_t CAPACITY = JSON_OBJECT_SIZE(3);
    StaticJsonDocument<CAPACITY> doc;
    JsonObject object = doc.to<JsonObject>();

    object["boardId"] = MAC_ID;
    object["temperature"] = temperature;
    object["humidity"] = humidity;

    serializeJson(doc, jsonOutput);

    webSocket.emit("sensorsUpdate", jsonOutput);
  }
};

void showPayload(const char *payload, size_t length)
{
  Serial.println(payload);
}

void sendID(const char *payload, size_t length)
{
  char jsonOutput[128];
  const size_t CAPACITY = JSON_OBJECT_SIZE(1);
  StaticJsonDocument<CAPACITY> doc;
  JsonObject object = doc.to<JsonObject>();
  object["boardId"] = MAC_ID;
  serializeJson(doc, jsonOutput);
  webSocket.emit("bootcheck", jsonOutput);
}

void bootcheck(const char *payload, size_t length)
{

  Serial.println(payload);
  DynamicJsonDocument doc1(1024);
  deserializeJson(doc1, payload);
  String HighHour = doc1["highHour"];
  String LowHour = doc1["lowHour"];
  String LightState = doc1["lightState"];
  ventState = doc1["ventState"];
  outExaust = doc1["outExaust"];
  inExaust = doc1["inExaust"];
  highHour = HighHour;
  lowHour = LowHour;
  lightState = LightState;

  digitalWrite(VENTPIN, !ventState);
  digitalWrite(OUTEXPIN, !outExaust);
  digitalWrite(INEXPIN, !inExaust);
};

void newTimeSetup(const char *payload, size_t length)
{
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  String ThighHour = doc["highHour"];
  String TlowHour = doc["lowHour"];
  highHour = ThighHour;
  lowHour = TlowHour;
  Serial.println(highHour);
  Serial.println(lowHour);
}

void setLightOn(const char *payload, size_t length)
{
  lightState = "ON";
  digitalWrite(LEDPIN, 0);
  lightLabel = true;
  Serial.println("on");
}
void setLightOff(const char *payload, size_t length)
{
  lightState = "OFF";
  digitalWrite(LEDPIN, 1);
  lightLabel = false;
  Serial.println("off");
}
void setLightAuto(const char *payload, size_t length)
{
  lightState = "AUTO";
  Serial.println("auto");
}

void changeVentState(const char *payload, size_t length)
{
  ventState = !ventState;
  digitalWrite(VENTPIN, !ventState);
  Serial.println(!ventState);
}

void changeInState(const char *payload, size_t length)
{
  inExaust = !inExaust;
  digitalWrite(INEXPIN, !inExaust);
  Serial.println(!inExaust);
}
void changeOutState(const char *payload, size_t length)
{
  outExaust = !outExaust;
  digitalWrite(OUTEXPIN, !outExaust);
}

void setup()

{
  Serial.begin(9600);

  pinMode(DHTPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(VENTPIN, OUTPUT);
  pinMode(INEXPIN, OUTPUT);
  pinMode(OUTEXPIN, OUTPUT);

  esp_efuse_read_mac(chipid);
  sprintf(buffer, "%02x%02x%02x%02x%02x%02x", chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]);
  MAC_ID = buffer;

  delay(500);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("");
  Serial.println("WiFi connected");
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());

  ntp.begin();
  ntp.forceUpdate();

  delay(500);

  webSocket.begin("", 80);

  // webSocket.begin("192.168.0.12", 80);

  webSocket.on("connect", sendID);
  webSocket.on("bootcheck", bootcheck);
  webSocket.on("message", showPayload);
  webSocket.on("newTimingSetup", newTimeSetup);
  webSocket.on("setLightOn", setLightOn);
  webSocket.on("setLightOff", setLightOff);
  webSocket.on("setLightAuto", setLightAuto);
  webSocket.on("changeVentState", changeVentState);
  webSocket.on("changeInState", changeInState);
  webSocket.on("changeOutState", changeOutState);
  webSocket.on("disconnected", showPayload);
}

void loop()
{
  lightTimer();
  readSensors();
  webSocket.loop();
  currentMillis = millis();
  delay(500);
}
