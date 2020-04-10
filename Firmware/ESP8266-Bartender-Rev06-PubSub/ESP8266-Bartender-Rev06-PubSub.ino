/***********************************************************************************************
   AUTHOR                  : Edward Daniel Nichols
   LAST CONTRIBUTION DATE  : April 9th, 2020
   Rev06
   Changelog:
   - Using PubSub instead of Adafruit's Library!
   - Integrated subscription data
***********************************************************************************************/
#include "WifiFunctions.h"
#include "BartenderFunctions.h"
#include <PubSubClient.h>

const char* mqtt_server = "192.168.1.78";
PubSubClient client(espClient);

#define MQTT_TIMEOUT          333
#define MQTT_BUFFER_SIZE      128
#define MQTT_MAX_PACKET_SIZE  256
#define BART_HEARTBEAT        "bart/heartbeat"      // Last-will topic, connection status with MQTT broker: "online", "offline"
#define BART_STATUS           "bart/status"         // General status topic: "idle", "disp", "err"
#define BART_SR_ALIGNMENT     "bart/sr/alignment"   // Ultrasonic sensor: "OK", "NO"
#define BART_SR_DISTANCE      "bart/sr/dist"        // Ultrasonic sensor: distance measured, cm
#define APP_MQTT              "app/#"               // Input from app, topic-levels wildcarded
#define BOT_MQTT              "bot/#"               // Input from butler bot, topic-levels wildcarded

unsigned long lastMsg = 0;
char msg[MQTT_BUFFER_SIZE];

/***********************************************************************************************/
void setup() {
  Serial.begin(115200);
  Serial.println("ESP8266 MQTT Bartender - Rev06" );
  bartCfgInfo();                          // Print useful info about ESP stack, config
  bartCfg();                              // Set pinmodes & etc. for pumps, sensors, MUX
  setup_wifi();                           // Connect to WiFi, hardcoded
  client.setServer(mqtt_server, 1883);    // Set MQTT broker base, configuration only
  client.setCallback(callback);           // Point client to callback function
};

/************************************************************************************************/
int value = 0;
void loop() {
  if (!client.connected()) {              // Check for client connection to MQTT
    reconnect();                          // Connect to MQTT, make subscriptions, etc.
  }
  client.loop();                          // "Listen" to MQTT socket, process incoming

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf(msg, MQTT_BUFFER_SIZE, "idle #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(BART_STATUS, msg);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  };
  Serial.println();

  // Parse data from payload
  if ((char)payload[0] == '1') {
    digitalWrite(LED, LOW);
  } else {
    digitalWrite(LED, HIGH);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.printf("Attempting MQTT connection...");
    String clientId = "bartender";
    bool MQTT_connection = client.connect(clientId.c_str(), NULL, NULL, BART_HEARTBEAT, 2, 1, "offline");
    if (MQTT_connection) {
      Serial.println("\t\t[DONE]");
      client.publish(BART_HEARTBEAT, "online");
      client.subscribe(APP_MQTT);
      client.subscribe(BOT_MQTT);
    } else {
      Serial.print("\t\t[FAIL]\n");
      Serial.printf("MQTT Error State: %d \n", client.state());
      Serial.println("Retry in 5 seconds");
      delay(5000);
    }
  }
}
