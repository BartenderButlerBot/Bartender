/*
   AUTHOR                  : Edward Daniel Nichols
   LAST CONTRIBUTION DATE  : January 15, 2020

   Rev00 - Objective:
   - Print any data to MQTT server
   - Print data from MQTT server
*/

/*Required local libraries: IN THIS ORDER */
#include    "WifiFunctions.h"
#include    "MqttFunctions.h"

/*Magnetometer global setup: header library files. */
#include   <Wire.h>

String  article;
void setup()
{
  // Start up the serial port connection and announce title.
  Serial.begin( 115200 );
  Serial.println( "ESP8266 MQTT Bartender - Rev00" );

  WifiSetup();                                // Setup and connect to WiFi.

  // Prompt MQTT to connect for the first time.
  MQTT_connect( 250 );
}

void loop()
{
  // retrieve sensor information here

  // Prepare string to publish the values to MQTT
  article = String("bartender: I'm alive!");
  
  // Verify MQTT broker is connected.
  if ( !mqtt.connected() )
    MQTT_connect(500);

  // Convert the String object to a character array, so that MQTT can actually publish it. */
  char post[64];
  article.toCharArray(post, 128);
  devOutput.publish(post);
  Serial.println(article);

  // Delay to give MQTT time to catch up.
  delay(500);
};
