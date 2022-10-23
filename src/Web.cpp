#include <Web.h>
#include <Led.h>

    //Variables------------------------------------------------------------------------------

    const char* SSID_1 = "Weffling";
    const char* SSID_2 = "Weffling_Outdoor";
    const char* SSID_PASSWORD = "86508912437821699676";

    // const char* mqtt_Broker_IP = "192.168.178.59" ;
    // const char* mqtt_User = "Mosquitto";
    // const char* mqtt_Pw   = "-Sam!993+";

    const char* mqtt_Broker_IP = "192.168.178.45" ;
    const char* mqtt_User = "";
    const char* mqtt_Pw   = "";

    const char* mqtt_ClientId = "Schrank_Licht";


    const char* NTPServerName = "pool.ntp.org";
    const int NTP_PACKET_SIZE = 48;

    byte NTPBuffer[NTP_PACKET_SIZE] ;

    ESP8266WiFiMulti wifiMulti;

    WiFiUDP UDP;
    IPAddress timeServerIP;


    ESP8266WebServer server(80);
    WiFiClient ESP_Bett_Rechts;
    PubSubClient client(ESP_Bett_Rechts);

    byte MQTT_Message_Received = 0;
    uint32_t MQTT_Code = 0;

//Check Wlan Connection---------------------------------------------------------------------------
    uint8_t checkWlanConnection(){
        uint8_t Wlan_Status = WiFi.isConnected();

        // if(Wlan_Status)
        //     digitalWrite(LED_BUILTIN , 0);
        // else
        //     digitalWrite(LED_BUILTIN , 1);
        return Wlan_Status;
    }

//Connect Wifi------------------------------------------------------------------------------------
    void connectWifi(){
        Serial.println("Connecting to Wlan :");
        while (wifiMulti.run() != WL_CONNECTED){
            delay(200);
            Serial.print(".");
        }
        Serial.println("\n");
        Serial.print("Connected to :");
        Serial.print(WiFi.SSID());
        Serial.print(" Ip-Adress :");
        Serial.println(WiFi.localIP());
    }

//Arduino OTA---------------------------------------------------------------------------------------
    void setupOTA(){
        ArduinoOTA.setHostname("ESP_Schrank_Licht");
        ArduinoOTA.setPassword("ESP_Schrank_Licht");

        ArduinoOTA.onStart([]() {
            Serial.println("ArduinoOTA Start");
        });

        ArduinoOTA.onEnd([]() {
            Serial.println("ArduinoOTA End");
        });

        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total){
            Serial.printf("Progress : %u%%\r", (progress/(total/100)));
        });

        ArduinoOTA.onError([](ota_error_t error){
            Serial.printf("Error[%u]:", error);
            if(error == OTA_AUTH_ERROR) 
            Serial.println("Auth. Failed");
            else if(error == OTA_BEGIN_ERROR)
            Serial.println("Begin Failed");
            else if(error == OTA_CONNECT_ERROR)
            Serial.println("Connect Failed");
            else if(error == OTA_RECEIVE_ERROR)
            Serial.println("Receive Failed");
            else if(error == OTA_END_ERROR)
            Serial.println("End failed");
        });

        ArduinoOTA.begin();
        Serial.println("ArduinoOTA ready");
    }

//Callback MQTT Receive Data from Broker--------------------------------------------------------------
    void CallbackMQTT(char* topic , byte* payload , int length){

        char msg[length+1];
        for (int i = 0; i < length; i++)
            msg[i] = (char)payload[i];
        msg[length] = '\0';

        String smsg = msg;

        if(!strcmp(topic , "Kummer/Lights/Mode")){
            t_led_mode mode = (t_led_mode)smsg.toInt();
            setLed_Mode(mode);
        }    

        if(!strcmp(topic , "Kummer/Lights/Brightness")){
            uint32_t brightness = strtoul(msg , NULL , 10);
            setBrightnessAbs(brightness);
        }
        if(!strcmp(topic , "Kummer/Lights/RGBW_HEX")){
        
            uint64_t hex_val = strtoul(msg , NULL , 16);
            RGBW((hex_val & 0xFF000000) >> 24 ,(hex_val & 0x00FF0000) >> 16 , (hex_val & 0x0000FF00) >> 8 , (hex_val & 0x000000FF));
        }

        // if(!strcmp(topic , "Kummer/Licht/Website/RGBW_HEX")){
        //     setLed_Mode(1);
        //     uint32_t hex_val = strtoul(msg , '\0' , 16);
        //     RGBW((hex_val & 0xFF000000) >> 24 ,(hex_val & 0x00FF0000) >> 16 , (hex_val & 0x0000FF00) >> 8 , (hex_val & 0x000000FF));
        // }

        // if(!strcmp(topic , "Kummer/Licht/Website/Mode")){
        //     uint32_t mode = strtoul(msg , '\0' , 10);
        //     setLed_Mode(mode);
        // }


    }


//Connect MQTT to Broker------------------------------------------------------------------------------
    void connectMQTT(){
        Serial.println("Connecting to MQTT Server:");

        while(!client.connected()){

            if(client.connect(mqtt_ClientId , mqtt_User , mqtt_Pw ,"Kummer/Licht/Schrank/Connection", 0 , true , "Disconnected" ))
                client.publish("Kummer/Licht/Schrank/Connection" ,  "Connected" , true);
            else{
            // Serial.print("\nConnection failed with state:");
            // Serial.println(client.state());
            }
        }

        if(client.connected()){
            Serial.println("Mqtt Conneced");
            client.subscribe("Kummer/Lights/#");
        }
    }

//Start UDP for Time-----------------------------------------------------------------------------------
    void startUDP(){
        Serial.println("Starting UDP!");
        UDP.begin(123);
        Serial.print("Local Port :\t");
        Serial.println(UDP.localPort());
    }

//Send NTP Packet Time--------------------------------------------------------------------------------
    void sendNTPpacket(IPAddress& address){
        memset(NTPBuffer , 0 , NTP_PACKET_SIZE);
        NTPBuffer[0] = 0b11100011;
        UDP.beginPacket(address ,123);
        UDP.write(NTPBuffer , NTP_PACKET_SIZE);
        UDP.endPacket();
    }

//Connect NTP Time------------------------------------------------------------------------------------
    void connectNTP(){
        if(!WiFi.hostByName(NTPServerName,timeServerIP)){
            Serial.println("DNS lookup failed!");
        }else{
            Serial.print("Time Server IP: ");
            Serial.println(timeServerIP);
            Serial.println("Sending NTP request..");
            sendNTPpacket(timeServerIP);
        }
    }

//Get NTP Time--------------------------------------------------------------------------------------------------
    uint32_t getNTPTime(){
        if(UDP.parsePacket() == 0)
            return 0;

        UDP.read(NTPBuffer , NTP_PACKET_SIZE);

        uint32_t NPTTime = (NTPBuffer[40] <<24) | (NTPBuffer[41] <<16) |(NTPBuffer[42]<<8) | NTPBuffer[43];
        const uint32_t UNIX_startYear = 2208988800UL;
        uint32_t UNIX_Time = NPTTime - UNIX_startYear;

        Serial.print("Unixtime :");
        Serial.println(UNIX_Time);

        UNIX_Time +=3600;
        
        return UNIX_Time;
    }

//Unix Time to normal----------------------------------------------------------------------------------------
    byte UnixToSeconds(uint32_t unix){
        return unix % 60;
    }

    byte UnixToMinutes(uint32_t unix){
        return unix / 60 % 60;
    }

    byte UnixToHours(uint32_t unix){
        return unix / 3600 % 24;
    }

//Get Data for MQTT--------------------------------------------------------------------------------------------
    void publishDataMQTT(uint64_t code){
        char c_code[8] ;
        uint32_t ul_code = code & 0xFFFFFF;
        String s_code = String(ul_code , HEX);
        s_code.toCharArray(c_code , 8);

        client.publish("Kummer/Licht/Schrank/IR_Code" , c_code , true);
    }
