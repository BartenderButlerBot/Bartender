/*
   AUTHOR                  : Edward Daniel Nichols
   LAST CONTRIBUTION DATE  : April 4th, 2020
   Rev05
*/

#include    "WifiFunctions.h"
#include    "MqttFunctions.h"
#include   <Wire.h>

#define LED           0
#define MQTT_TIMEOUT     333

/*Useful global variables: */
char    article;

/*MQTT output topic to other subsystems: */
#define MQTT_OUT          "bart/output"
Adafruit_MQTT_Publish     mqtt_output = Adafruit_MQTT_Publish(&mqtt, MQTT_OUT);

/*MQTT input topic from other subsystems: */
#define MQTT_IN           "bart/input"
Adafruit_MQTT_Subscribe   mqtt_input = Adafruit_MQTT_Subscribe(&mqtt, MQTT_IN, MQTT_QOS_1);

void mqtt_input_callback(double incoming) {
  Serial.printf("MQTT sub to \"%s\" ...", MQTT_IN); Serial.printf("\t\t[ACK]\n");
  Serial.printf("Payload: %d", incoming);
};

void setup()
{
  /***********************************************************************************************
    This section prepares the Serial Ports: */
  // Start up the serial port connection and announce title.
  Serial.begin( 115200 );
  Serial.println("\n\nESP8266 MQTT Bartender - TestMQTT" );

  /***********************************************************************************************$
    This section prepares WiFi 802.11 & MQTT: */
  WifiSetup();                            // Setup and connect to WiFi.
  MQTT_connect( MQTT_TIMEOUT );           // Prompt MQTT to connect for the first time.
  /* ----------------------------------------------------->
    Alerts MQTT & system devices that this unit is online */
  // Prepare a string to publish the values to MQTT
  char article[] = "online";
  Serial.printf("Publishing to %s: \"%s\" ...", MQTT_OUT, article);
  if (mqtt_output.publish(article)) {
    Serial.printf("\t\t[ACK]\n");
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
  Serial.printf("Publishing to %s: \"%s\" ...", MQTT_OUT, article);
  if (mqtt_output.publish(article)) {
    Serial.printf("\t\t[ACK]\n");
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
  delay(2500);
};
