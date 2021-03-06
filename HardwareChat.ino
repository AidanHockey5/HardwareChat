//2018 Aidan Lawrence, www.AidanLawrence.com
//A simple chat service that you can access via your internet-enabled Arduino hardware!
//CONNECTIONS ARE NOT SECURE AND ALL DATA IS PUBLIC! DO NOT SHARE ANY SENSITIVE INFORMATION.
//IP Addresses are not shared to any other client.
//Yes, of course you can modify this script and have some fun, just be cool about it when connecting to the server, please!
//Have fun and be nice!
//https://github.com/AidanHockey5/HardwareChat

//!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!ADD YOUR CREDENTIALS!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define AP_SSID  "ADD_YOUR_WIFI_SSID_HERE"
#define AP_PASS  "ADD_YOUR_WIFI_PASSWORD_HERE"
#define USERNAME "PUT_YOUR_USER_NAME_HERE" //Please keep your username under 16 characters 

#define MQTT_SERVER "mqtt.aidanlawrence.com"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
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
String cID = USERNAME;

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
  randomSeed(micros());
  Serial.print("Your username will be: "); Serial.println(USERNAME);
  ConnectToWiFi();
  cID += String(random(0, 2147483647));
  Serial.println(cID);
  client.begin(MQTT_SERVER, net);
  client.onMessage(Incomming);
  ConnectToBroker();
}

bool ConnectToWiFi()
{
  Serial.print("Connecting to AP: "); Serial.println(AP_SSID);
  if (WiFi.status() != WL_CONNECTED) 
    WiFi.begin(AP_SSID, AP_PASS);
  delay(1000);
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
  while(!client.connect(&cID[0u], &cID[0u]))
  {
    Serial.print(".");
    delay(500);
  }
  client.subscribe("/online/#");
  client.subscribe("/chat"); //Subscribe to the main chat
  client.subscribe("/"+usernameAllCaps); //Subscribe to private messages (NOTE, THESE ARE NOT SECURE AT ALL! Anyone can see these PMs if they try!)
  return true;
}

void Incomming(String &topic, String &payload)
{
  if(topic == "/PING")
    return;
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
uint64_t dcTime = 0;
void loop() 
{
  client.loop();
  if(!client.connected())
  {
    ConnectToBroker();
    dcTime = millis();
  }
  #if defined(ESP8266)
  delay(10);
  #endif

  if(!joined && millis() >= dcTime+2000) //Joined. Wait a little bit to make sure, then subscribe to channels and publish presence 
  {
    Serial.println();
    Serial.println("Connected! Have fun and be nice!");
    Serial.println("Type !list to see who else is online!");
    client.publish("/chat", String(USERNAME)+" has joined the chat.", false, 2);
    client.publish("/online/"+String(USERNAME)+"/", "1", true, 2); //Mark user as "online"
    joined = true;
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
