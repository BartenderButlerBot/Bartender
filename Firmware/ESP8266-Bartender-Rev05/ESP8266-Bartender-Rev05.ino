/*
   AUTHOR                  : Edward Daniel Nichols
   LAST CONTRIBUTION DATE  : April 4th, 2020
   Rev05
*/

/*Required local libraries: IN THIS ORDER */
#include    "WifiFunctions.h"
#include    "MqttFunctions.h"
#include   <Wire.h>

/*Required PINOUT mappings: */
#define LED           0

/*Configurable directives:  */
#define MQTT_TIMEOUT     333

/*Useful global variables: */
char    article;
char*   mqttmsg;
uint16_t len;

void mqtt_input_callback(char* mqttmsg, uint16_t len) {
  Serial.printf("\tSubscription message \"%s\": %s", MQTT_IN, mqttmsg);
  Serial.printf("\t[ACK]\n");
};

void setup()
{
  /***********************************************************************************************
    This section prepares the Serial Ports: */
  // Start up the serial port connection and announce title.
  Serial.begin( 115200 );
  Serial.println( "ESP8266 MQTT Bartender - Rev05" );

  /***********************************************************************************************$
    This section prepares WiFi 802.11 & MQTT: */
  WifiSetup();                            // Setup and connect to WiFi.
  MQTT_connect( MQTT_TIMEOUT );           // Prompt MQTT to connect for the first time.
  /* ----------------------------------------------------->
    Alerts MQTT & system devices that this unit is online */
  // Prepare a string to publish the values to MQTT
  char article[] = "online";
  Serial.printf("Publishing to MQTT: \"%s\" ...", article);
  if (mqtt_output.publish(article)) {
    Serial.printf("\t[ACK]\n");
    delay(25);
  } else {
    Serial.println("\t[NACK]\n");
    delay(25);
  };

  /* ----------------------------------------------------->
    Makes the Subscriptions to relevant MQTT topics */
  mqtt_input.setCallback(mqtt_input_callback);
  mqtt.subscribe(&mqtt_input);   // Subscribe to app/order MQTT topic

  /***********************************************************************************************$
    This section prepares pumps & sensors: */
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);          // Ensure LED is OFF (LOW=ON, see PCB)
}

void loop()
{
  /***********************************************************************************************$
    This section contains loop fail-safe measures. */
  if ( !mqtt.connected() ) {        // Verify MQTT broker is connected
    MQTT_connect( MQTT_TIMEOUT );    // If not connected, then connect
  };

  /* ----------------------------------------------------->
    Alert MQTT that this unit is "idle" */
  char article[] = "idle";
  Serial.printf("Publishing to MQTT: \"%s\" ...", article);
  if (mqtt_output.publish(article)) {
    Serial.printf("\t[ACK]\n");
  } else {
    Serial.println("\t[NACK]\n");
  };

  digitalWrite(LED, LOW);
  mqtt.processPackets(10000);

  // Ping the server to refresh connection, or disconnect if unreachable.
  if (!mqtt.ping()) {
    mqtt.disconnect();
  };

  digitalWrite(LED, HIGH);
  delay(5000);
};
