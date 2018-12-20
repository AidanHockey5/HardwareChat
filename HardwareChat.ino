//2018 Aidan Lawrence, www.AidanLawrence.com
//A simple chat service that you can access via your internet-enabled Arduino hardware!
//CONNECTIONS ARE NOT SECURE AND ALL DATA IS PUBLIC! DO NOT SHARE ANY SENSITIVE INFORMATION.
//IP Addresses are not shared to any other client.
//Yes, of course you can modify this script and have some fun, just be cool about it when connecting to the server, please!
//Have fun and be nice!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!ADD YOUR CREDENTIALS!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define AP_SSID  "ADD_YOUR_WIFI_SSID_HERE"
#define AP_PASS  "ADD_YOUR_WIFI_PASSWORD_HERE"
#define USERNAME "PUT_YOUR_USER_NAME_HERE" //Please keep your username under 16 characters 

#define MQTT_SERVER "mqtt.aidanlawrence.com"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#elif defined(ARDUINO_SAMD_ZERO)
#include <SPI.h>
#include <WiFi101.h>
#endif

#include <WiFiClient.h>
#include "MQTT.h"

WiFiClient net;
MQTTClient client(256);
String usernameAllCaps = USERNAME;
bool joined = false;
uint32_t onlineCount = 0;
String onlineUserList = "";

void setup() 
{
  #if defined(ARDUINO_SAMD_ZERO)
  WiFi.setPins(8,7,4,2);
  #endif
  
  usernameAllCaps.toUpperCase();
  Serial.begin(115200);
  uint16_t count = 0;
  while(!Serial.available())
  {
    if(count % 3000 == 0)
      Serial.println("Enter any character to connect to the chat server");
    count++;
    delay(1);
  }
  while(Serial.available()){Serial.read(); yield();}
  randomSeed(millis());
  Serial.print("Your username will be: "); Serial.println(USERNAME);
  ConnectToWiFi();
  client.begin(MQTT_SERVER, net);
  client.onMessage(Incomming);
  ConnectToBroker();
}

bool ConnectToWiFi()
{
  Serial.print("Connecting to AP: "); Serial.println(AP_SSID);
  if (WiFi.status() != WL_CONNECTED) 
    WiFi.begin(AP_SSID, AP_PASS);
  Serial.println();
  Serial.println("Connected to WiFi AP");
  return true;
}

bool ConnectToBroker()
{
  onlineUserList = "";
  onlineCount = 0;
  Serial.print("Attempting to connect to: "); Serial.println(MQTT_SERVER);
  String willTopic = "/online/"+String(USERNAME)+"/";
  char *wt = &willTopic[0u];
  client.setWill(wt, "0", true, 2); //Mark user as offline if the client DCs
  String cID = USERNAME;
  cID += String(rand());
  while(!client.connect(&cID[0u], USERNAME))
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected! Have fun and be nice!");
  Serial.println("Type !list to see who else is online!");
  client.subscribe("/online/#");
  client.subscribe("/chat"); //Subscribe to the main chat
  client.subscribe("/"+usernameAllCaps); //Subscribe to private messages (NOTE, THESE ARE NOT SECURE AT ALL! Anyone can see these PMs if they try!)
  client.publish("/chat", String(USERNAME)+" has joined the chat.", false, 2);
  client.publish("/online/"+String(USERNAME)+"/", "1", true, 2); //Mark user as "online"
  return true;
}

void Incomming(String &topic, String &payload)
{
  if(topic.startsWith("/online"))
  {
    topic.replace("/online/", "");
    topic.remove(topic.length()-1); //remove the trailing '/' at the end of the topic.
    if(payload == "1")
    {
      if(onlineCount < 4294967295)
        onlineCount++;
      if(onlineUserList.indexOf(topic) > 0) //If that user is already listed as online, don't bother adding them to the list.
        return; 
      onlineUserList += topic + ", ";
    }
    else if(payload == "0")
    {
      if(onlineCount > 1)
        onlineCount--;
      onlineUserList.replace(topic+", ", "");
    }
    return;
  }
  Serial.println(payload);
}

String sendBuf = "";
void loop() 
{
  client.loop();
  #if defined(ESP8266) || defined(ESP32)
  delay(10);
  #endif
  if(!client.connected())
  {
    ConnectToBroker();
  }

  //Wait for serial input
  while(Serial.available())
  {
    sendBuf = Serial.readString();
    yield();
  }

  //Private messaging
  if(sendBuf.startsWith("!pm"))
  {
    sendBuf.replace("!pm ", "");
    String sendTo = "";
    int i = 0;
    while(true)
    {
      sendTo += sendBuf[i];
      i++;
      if(sendBuf[i] == ' ')
        break;
      yield();
    }
    sendBuf.replace(sendTo+" ", "");
    sendTo.toUpperCase();
    String pmString = "[PM]["+String(USERNAME)+"] "+sendBuf;
    client.publish("/"+sendTo, pmString, false, 1);
    Serial.println(pmString);
    sendBuf = "";
  }
  else if(sendBuf.startsWith("!list"))
  {
    Serial.print("Users online: "); Serial.println(onlineCount);
    Serial.println(onlineUserList);
    sendBuf = "";
  }

  //Public messaging 
  if(sendBuf != "")
  {
    client.publish("/chat", "["+String(USERNAME)+"] "+sendBuf, false, 1);
    sendBuf = "";
  }
}
