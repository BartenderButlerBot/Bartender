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

#define SR_CHECK_PERIOD       5000                  // Period between forced update to SR alignment flag
#define SR_THRESHOLD          20.0                  // Smaller means the cup/Butler has to be closer to trigger alignment flag
#define MQTT_KEEPALIVE        180000                // How long to keep MQTT connection alive if no I/O, ms
#define MQTT_PING_PERIOD      1000                  // Period between forced update to status flag 
#define MQTT_MAX_PACKET_SIZE  256                   // Maximum size of input packet, bytes
#define MQTT_BUFFER_SIZE      128                   // Maximum size of output packet, char[]
#define BART_HEARTBEAT        "bart/heartbeat"      // Last-will topic, connection status with MQTT broker: "online", "offline"
#define BART_STATUS           "bart/status"         // General status topic: "idle", "disp", "err"
#define BART_SR_ALIGNMENT     "bart/sr/alignment"   // Ultrasonic sensor: "OK", "NO"
#define BART_SR_DISTANCE      "bart/sr/dist"        // Ultrasonic sensor: distance measured, cm
#define APP_MQTT              "app/#"               // Input from app, topic-levels wildcarded
#define BOT_MQTT              "bot/#"               // Input from butler bot, topic-levels wildcarded
#define SERIAL_TEST           "serial"

unsigned long lastPing = 0;
unsigned long lastSrChk = 0;
bool sr;
char article[MQTT_BUFFER_SIZE];
const char* mqtt_server = "192.168.1.73";
PubSubClient client(espClient);

/***********************************************************************************************/
void setup() {
  Serial.begin(115200);
  Serial.println("ESP8266 MQTT Bartender - Rev06" );
  bartCfgInfo();                                    // Print useful info about ESP stack, config
  bartCfg();                                        // Set pinmodes & etc. for pumps, sensors, MUX
  pumpSelect(99);
  setup_wifi();                                     // Connect to WiFi, hardcoded
  client.setServer(mqtt_server, 1883);              // Set MQTT broker base, configuration only
  client.setCallback(callback);                     // Point client to callback function
};

/************************************************************************************************/
void loop() {
  if (!client.connected()) {                        // Check for client connection to MQTT
    reconnect();                                    // Connect to MQTT, make subscriptions, etc.
  }
  client.loop();                                    // "Listen" to MQTT socket, process incoming (CRUCIAL)
  /*$***********************$*/
  // Important real-time code
  double sr_dist = sr_distance();
  if (sr_dist < SR_THRESHOLD ) {
    sr = true;
    String("true").toCharArray(article, MQTT_BUFFER_SIZE);
    digitalWrite(LED, LOW);
  } else {
    sr = false;
    String("false").toCharArray(article, MQTT_BUFFER_SIZE);
    digitalWrite(LED, HIGH);
  };

  unsigned long now = millis();
  /*$***********************$*/
  /*Code executed periodically*/
  if (now - lastSrChk > SR_CHECK_PERIOD) {
    lastSrChk = now;
    Serial.printf("Publish to [%s]: %s \n", BART_SR_ALIGNMENT, article);
    client.publish(BART_SR_ALIGNMENT, article, true);

    String(sr_dist).toCharArray(article, MQTT_BUFFER_SIZE);
    Serial.printf("Publish to [%s]: %s \n", SERIAL_TEST, article);
    //client.publish(BART_SR_DISTANCE, article);
  }

  if (now - lastPing > MQTT_PING_PERIOD) {
    lastPing = now;
    char localbartstat[] = "idle";                  // Inform MQTT that Bart is idle
    Serial.printf("Publish to [%s]: %s \n", BART_STATUS, localbartstat);
    client.publish(BART_STATUS, localbartstat, true);
  }
}

/************************************************************************************************/
void callback(char* topic, byte* payload, unsigned int len) {
  /*Print to Serial, topic & message: */
  char received[MQTT_BUFFER_SIZE]; snprintf(received, len + 1, "%s", payload);
  Serial.printf("Message from [%s]: %s\n", topic, received);

  /**********************************************************************************************/
  /*Parse data by topic: */
  if (strcmp(topic, "bot/status") == 0) {
    char localbartstat[] = "heard";              // Inform MQTT that Bart is in subloop
    Serial.printf("Publish to [%s]: %s \n", BART_STATUS, localbartstat);
    client.publish(BART_STATUS, localbartstat, true);

    /*$***********************$*/
    /*Parse data by payload code: */
//    if (strcmp(received, "docked@bar") == 0) {
//      Serial.println("BAM1");
//    } else if (strcmp(received, "docked@base") == 0) {
//      Serial.println("BAM2");
//    } else if (strcmp(received, "runto@bar") == 0) {
//      Serial.println("BAM3");
//    } else if (strcmp(received, "runto@base") == 0) {
//      Serial.println("BAM4");
//    } else {
//      Serial.println("BAM5");
//    };

  } else if (strcmp(topic, "app/m1direct") == 0) {
    char localbartstat[] = "busy";              // Inform MQTT that Bart is in subloop
    Serial.printf("Publish to [%s]: %s \n", BART_STATUS, localbartstat);
    client.publish(BART_STATUS, localbartstat, true);

    int quantity = String(received).toInt();
    pumpSelect(1);
    pumpOperate(quantity);

  } else if (strcmp(topic, "app/m2direct") == 0) {
    char localbartstat[] = "busy";              // Inform MQTT that Bart is in subloop
    Serial.printf("Publish to [%s]: %s \n", BART_STATUS, localbartstat);
    client.publish(BART_STATUS, localbartstat, true);

    int quantity = String(received).toInt();
    pumpSelect(2);
    pumpOperate(quantity);

  } else if (strcmp(topic, "app/m3direct") == 0) {
    char localbartstat[] = "busy";              // Inform MQTT that Bart is in subloop
    Serial.printf("Publish to [%s]: %s \n", BART_STATUS, localbartstat);
    client.publish(BART_STATUS, localbartstat, true);

    int quantity = String(received).toInt();
    pumpSelect(3);
    pumpOperate(quantity);

  } else if (strcmp(topic, "app/m4direct") == 0) {
    char localbartstat[] = "busy";              // Inform MQTT that Bart is in subloop
    Serial.printf("Publish to [%s]: %s \n", BART_STATUS, localbartstat);
    client.publish(BART_STATUS, localbartstat, true);

    int quantity = String(received).toInt();
    pumpSelect(4);
    pumpOperate(quantity);

  } else if (strcmp(topic, "app/m5direct") == 0) {
    char localbartstat[] = "busy";              // Inform MQTT that Bart is in subloop
    Serial.printf("Publish to [%s]: %s \n", BART_STATUS, localbartstat);
    client.publish(BART_STATUS, localbartstat, true);

    int quantity = String(received).toInt();
    pumpSelect(5);
    pumpOperate(quantity);

  } else if (strcmp(topic, "app/m6direct") == 0) {
    char localbartstat[] = "busy";              // Inform MQTT that Bart is in subloop
    Serial.printf("Publish to [%s]: %s \n", BART_STATUS, localbartstat);
    client.publish(BART_STATUS, localbartstat, true);

    int quantity = String(received).toInt();
    pumpSelect(6);
    pumpOperate(quantity);

  } else if (strcmp(topic, "app/m7direct") == 0) {
    char localbartstat[] = "busy";              // Inform MQTT that Bart is in subloop
    Serial.printf("Publish to [%s]: %s \n", BART_STATUS, localbartstat);
    client.publish(BART_STATUS, localbartstat, true);

    int quantity = String(received).toInt();
    pumpSelect(7);
    pumpOperate(quantity);

  } else if (strcmp(topic, "app/m8direct") == 0) {
    char localbartstat[] = "busy";              // Inform MQTT that Bart is in subloop
    Serial.printf("Publish to [%s]: %s \n", BART_STATUS, localbartstat);
    client.publish(BART_STATUS, localbartstat, true);

    int quantity = String(received).toInt();
    pumpSelect(8);
    pumpOperate(quantity);

  } else if (strcmp(topic, "app/order") == 0) {
    char intake_bartstat[] = "intake";                 // Inform MQTT that Bart is idle
    Serial.printf("Publish to [%s]: %s \n", BART_STATUS, intake_bartstat);
    client.publish(BART_STATUS, intake_bartstat, true);

    String order = received;
    order.replace("'", ""); order.replace(" ", "");
    order = order.substring(1, order.length() - 1);
    //Serial.println(order);
    for (int counter = 0; counter < order.length(); counter++) {
      char pump[2];
      char quantity[4];
      if (order[counter] == '(') {
        counter = counter + 2;
        pump[0] = order[counter];
        counter = counter + 2;
        int i = 0;
        while (order[counter] != ')') {
          quantity[i] = order[counter];
          counter++; i++;
        }

        Serial.print("Pump: ");
        Serial.println(String(pump).toInt());
        Serial.print("quantity: ");
        Serial.println(String(quantity).toInt());

        pumpSelect(String(pump).toInt());
        pumpOperate(String(quantity).toInt());
      }
    }

    if (!client.connected()) {                        // Check for client connection to MQTT
      reconnect();                                    // Connect to MQTT, make subscriptions, etc.
    }

    char done_bartstat[] = "done";                    // Inform MQTT that Bart is done
    Serial.printf("Publish to [%s]: %s \n", BART_STATUS, done_bartstat);
    client.publish(BART_STATUS, done_bartstat, true);
    delay(10000);
    pumpSelect(99);
  }

  // Test parse from payload
  if ((char)payload[0] == '1') {
    digitalWrite(LED, LOW);
  } else {
    digitalWrite(LED, HIGH);
  }
}

/************************************************************************************************/
void reconnect() {
  while (!client.connected()) {
    Serial.printf("Attempting MQTT connection...");
    String clientId = "bartender";
    bool MQTT_connection = client.connect(clientId.c_str(), NULL, NULL, BART_HEARTBEAT, 2, 1, "offline");
    if (MQTT_connection) {
      Serial.println("\t\t[DONE]");
      client.publish(BART_HEARTBEAT, "online", true);
      client.subscribe(APP_MQTT);
      client.subscribe(BOT_MQTT);
    } else {
      Serial.println("\t\t[FAIL]");
      Serial.printf("MQTT Error State: %d \n", client.state());
      Serial.println("Retry in 5 seconds");
      delay(5000);
    }
  }
}
