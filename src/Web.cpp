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
    WebSocketsServer webSocket(81);
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


        if(!strcmp(topic , "Kummer/Licht/Ir/Mode")){
             uint8_t mode = smsg.toInt();
            setLed_Mode(mode);
        }    

        if(!strcmp(topic , "Kummer/Licht/Ir/RGBW_HEX")){
            uint32_t hex_val = strtoul(msg , '\0' , 16);
            RGBW((hex_val & 0xFF000000) >> 24 ,(hex_val & 0x00FF0000) >> 16 , (hex_val & 0x0000FF00) >> 8 , (hex_val & 0x000000FF));
            client.publish("Kummer/Licht/Bett_Links/RGBW_HEX", msg ,true );
        }

        if(!strcmp(topic , "Kummer/Licht/Website/RGBW_HEX")){
            setLed_Mode(1);
            uint32_t hex_val = strtoul(msg , '\0' , 16);
            RGBW((hex_val & 0xFF000000) >> 24 ,(hex_val & 0x00FF0000) >> 16 , (hex_val & 0x0000FF00) >> 8 , (hex_val & 0x000000FF));
        }

        if(!strcmp(topic , "Kummer/Licht/Website/Mode")){
            uint32_t mode = strtoul(msg , '\0' , 10);
            setLed_Mode(mode);
        }

        if(!strcmp(topic , "Kummer/Licht/Website/Brightness")){
            uint32_t brightness = strtoul(msg , '\0' , 10);
            setBrightnessAbs(brightness);
        }
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
            client.subscribe("Kummer/Licht/Ir/#");
            client.subscribe("Kummer/Licht/Website/#");
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

//Get Content Type Webserver------------------------------------------------------------------------
    String getContentType(String filename) { 
        if (filename.endsWith(".html")) return "text/html";
            else if (filename.endsWith(".css")) return "text/css";
            else if (filename.endsWith(".js")) return "application/javascript";
            else if (filename.endsWith(".ico")) return "image/x-icon";
        return "text/plain";
    }

//Handle File Read Webserver--------------------------------------------------------------------------
    bool handleFileRead(String path) { // send the right file to the client (if it exists)
        Serial.println("handleFileRead: " + path);
        if (path.endsWith("/"))
            path += "index.html";         // If a folder is requested, send the index file
        String contentType = getContentType(path);            // Get the MIME type
        if (SPIFFS.exists(path)) {                            // If the file exists
            File file = SPIFFS.open(path, "r");                 // Open it
            size_t sent = server.streamFile(file, contentType); // And send it to the client
            file.close();                                       // Then close the file again
            return true;
        }
        Serial.println("\tFile Not Found");
        return false;                                         // If the file doesn't exist, return false
    }

//Websocket Event-----------------------------------------------------------------------------------------
    void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
        switch (type) {
            case WStype_DISCONNECTED:             // if the websocket is disconnected
                Serial.printf("[%u] Disconnected!\n", num);
                break;
            case WStype_CONNECTED:{             // if a new websocket connection is established
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                }
                break;
            case WStype_TEXT: // if new text data is received
                Serial.printf("[%u] get Text: %s\n", num, payload);
                break;
        }
    }

//Start Websocket----------------------------------------------------------------------------------------------
    void startWebSocket() { 
        webSocket.begin();                          // start the websocket server
        webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
        Serial.println("WebSocket server started.");
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
