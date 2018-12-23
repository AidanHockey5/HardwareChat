# Hardware Chat!

A global chat client that you can use to connect to people around the world through your internet-enabled microcontroller!

1) Download project
2) Open project (HardwareChat.ino) in the Arduino IDE
3) Pick your microcontroller (Supports ESP8266 boards and Adafruit Feather M0 Wifi (ATWINC1500))
4) Enter your WiFi SSID and Password and Username at the top of the script!
5) Upload the firmware
6) Open up your serial console at 115200 baud, No line ending


All messages are public, even the "private" ones. There is no encryption. Do not use for sensitive information (duh)

To send a private message, use this format (names are not case-sensitive):

`!pm targetName your message goes here.`

To see who else is online, type:

`!list`

# What is MQTT? 

Adafruit has a super great guide on the subject!
https://learn.adafruit.com/adafruit-io/mqtt-api
https://youtu.be/shqLy8XjqAQ?t=669

# You can also create your own MQTT servers! 

I recommend using the Mosquitto broker. It's free, easy to install, and works well.

https://mosquitto.org/ 

Installing Mosquitto on an Ubuntu server is simple! Just run this command:

`sudo apt-get install mosquitto mosquitto-clients`

... and forward port `1883`. That's it!

For more information and how to enable encryption, read this article.
https://www.digitalocean.com/community/tutorials/how-to-install-and-secure-the-mosquitto-mqtt-messaging-broker-on-ubuntu-16-04


# Disclaimers 

This project is just for fun. Everything you post through the chat client is unencrypted and not secure. Do not post any sensitive information, duh.

IP addresses are not transmitted between clients, so there isn't any need to worry about somebody getting ahold of yours should you connect.

I am not personally responsible for any user-generated content on the chat server. 
