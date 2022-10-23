#ifndef _Web_H_
#define _Web_H_

    #include <ArduinoOTA.h>
    #include <ESP8266WiFi.h>
    #include <ESP8266WiFiMulti.h>
    #include <WiFiUdp.h>
    #include <ESP8266WebServer.h>
    #include <FS.h>
    #include <PubSubClient.h>


    extern const char* SSID_1;
    extern const char* SSID_2;
    extern const char* SSID_PASSWORD;

    extern const char* mqtt_Broker_IP;
    extern const char* mqtt_User;
    extern const char* mqtt_Pw;
    extern const char* mqtt_ClientId;
    
    extern const char* NTPServerName;
    extern const int NTP_PACKET_SIZE;

    extern byte NTPBuffer[];

    extern ESP8266WiFiMulti wifiMulti;
    extern WiFiUDP UDP;
    extern IPAddress timeServerIP;
    extern ESP8266WebServer server;
    extern WiFiClient ESP_Bett_Rechts;
    extern PubSubClient client;

    extern byte bMode_Led;
    extern byte MQTT_Message_Received;
    extern uint32_t MQTT_Code;


    byte checkWlanConnection();
    void connectWifi();
    void setupOTA();
    void CallbackMQTT(char* topic , byte* payload , int length);
    void connectMQTT();
    void startUDP();
    void sendNTPpacket(IPAddress& address);
    void connectNTP();
    String getContentType(String filename);
    bool handleFileRead(String path);
    uint32_t getNTPTime();
    byte UnixToSeconds(uint32_t unix);
    byte UnixToMinutes(uint32_t unix);
    byte UnixToHours(uint32_t unix);
    void getData();
    void publishDataMQTT(uint64_t code);
    void publishDataWebsocket();
    void publishData();

#endif