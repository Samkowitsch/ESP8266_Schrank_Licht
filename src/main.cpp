#include <Arduino.h>


#include <Led.h>
#include <Web.h>


#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define IR_PIN D5

uint8_t color_1 = 160;

uint16_t Fortschritt = 0;
uint8_t mode = 0;


void setup() {

  pinMode(DATA_PIN, OUTPUT);
  pinMode(LED_BUILTIN , OUTPUT);
  analogWriteRange(256);
  digitalWrite(LED_BUILTIN , 1);
  
  Serial.begin(115200);
  Serial.println("Starting");

  setupFastLed();

  RGBW(125,0,0,0);


  wifiMulti.addAP(SSID_1,SSID_PASSWORD);
  wifiMulti.addAP(SSID_2,SSID_PASSWORD);

  connectWifi();
  checkWlanConnection();
  
  setupOTA();

  //SPIFFS.begin();

  /*server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });


  server.begin();
  startWebSocket();

  //startUDP();
  //connectNTP();

  */

  client.setServer(mqtt_Broker_IP,1883);
  client.setCallback(CallbackMQTT);

  connectMQTT();

  RGBW(0,0,0,0);
}

uint8_t getSerial(){
  char buffer = 0;

  if(Serial.available())
    buffer = Serial.read();

  if(buffer != 255)
    Serial.print(buffer , DEC);

  return buffer - 48;
}

void loop(){
  
  if(!checkWlanConnection())
    connectWifi();

  client.loop();

  ArduinoOTA.handle();

  if(!client.connected())
    connectMQTT();

  ledMode();
  

}