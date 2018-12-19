//2018 Aidan Lawrence, www.AidanLawrence.com
//A simple chat service that you can access via your internet-enabled Arduino hardware!
//CONNECTIONS ARE NOT SECURE AND ALL DATA IS PUBLIC! DO NOT SHARE ANY SENSITIVE INFORMATION.
//IP Addresses are not shared to any other client.
//Yes, of course you can modify this script and have some fun, just be cool about it when connecting to the server, please!
//Have fun and be nice!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!ADD YOUR WIFI CREDENTIALS!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define AP_SSID "ADD_YOUR_WIFI_SSID_HERE"
#define AP_PASS "ADD_YOUR_WIFI_PASSWORD_HERE"

#define MQTT_SERVER "mqtt.aidanlawrence.com"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "MQTT.h"

WiFiClient net;
MQTTClient client(256);
String userName = "";

void setup() 
{
  Serial.begin(115200);
  SetCredentials();
  ConnectToWiFi();
  client.begin(MQTT_SERVER, net);
  client.onMessage(Incomming);
  ConnectToBroker();
}

void SetCredentials()
{
  uint16_t count = 0;
  while(!Serial.available())
  {
    if(count % 5000 == 0)
      Serial.println("Please enter your username [16 chars max]");
    count++;
    delay(1);
  }
  while(Serial.available())
  {
    userName = Serial.readString(); 
    delay(1);
  }
  if(userName.length() > 16)
    userName.remove(16);
  userName.trim();
  Serial.print("Your username will be: "); Serial.println(userName);
}

bool ConnectToWiFi()
{
  Serial.print("Connecting to AP: "); Serial.println(AP_SSID);
  //WiFi.mode(WIFI_STA);
  if (WiFi.status() != WL_CONNECTED) 
    WiFi.begin(AP_SSID, AP_PASS);
  Serial.println();
  Serial.println("Connected to WiFi AP");
  return true;
}

bool ConnectToBroker()
{
  Serial.print("Attempting to connect to: "); Serial.println(MQTT_SERVER);
  while(!client.connect(&userName[0u], &userName[0u]))
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected! Have fun and be nice!");
  client.subscribe("/chat"); //Subscribe to the main chat
  client.subscribe("/"+userName); //Subscribe to private messages (NOTE, THESE ARE NOT SECURE AT ALL! Anyone can see these PMs if they try!)
  
  client.publish("/chat", userName+" has joined the chat.", false, 2);
  return true;
}

void Incomming(String &topic, String &payload)
{
  Serial.println(payload);
}

String sendBuf = "";
void loop() 
{
  client.loop();
  #if defined(ESP8266)
  delay(10);
  #endif
  if(!client.connected())
  {
    ConnectToBroker();
  }
  while(Serial.available())
  {
    sendBuf = Serial.readString();
    yield();
  }
  if(sendBuf != "")
  {
    client.publish("/chat", "["+userName+"] "+sendBuf, false, 1);
    sendBuf = "";
  }
}
