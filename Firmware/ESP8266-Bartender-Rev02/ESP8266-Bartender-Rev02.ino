/*
   AUTHOR                  : Edward Daniel Nichols
   LAST CONTRIBUTION DATE  : March 11, 2020
   Rev02 - Objective:
   - Print 
*/

/*Required local libraries: IN THIS ORDER */
#include    "WifiFunctions.h"
#include    "MqttFunctions.h"
#include   <Wire.h>

/*Require definitions:  */
#define MQTTTIMEOUT   500
#define LED           0
#define MOTORCMD      2
#define TRIGGERPIN    12
#define ECHOPIN       13

char    post;
String  article;
long duration;
int distance;

void setup()
{
  /*********************************************************************************************** 
  This section prepares the Serial Ports: */
  // Start up the serial port connection and announce title.
  Serial.begin( 115200 );
  Serial.println( "ESP8266 MQTT Bartender - Rev01" );

  /***********************************************************************************************$ 
  This section prepares GPIO pins: */
  pinMode(TRIGGERPIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHOPIN, INPUT); // Sets the echoPin as an Input
  pinMode(LED, OUTPUT);
  pinMode(MOTORCMD, OUTPUT);

  /***********************************************************************************************$ 
  This section prepares WiFi 802.11 & MQTT: */
  // Setup and connect to WiFi.
  WifiSetup();
  // Prompt MQTT to connect for the first time.
  MQTT_connect( MQTTTIMEOUT );

  /* -----------------------------------------------------> 
  MQTT-Out 00 - Preparing connection flag: */
  // Prepare a string to publish the values to MQTT
  article = String("connected");
  // Verify MQTT broker is connected.
  if ( !mqtt.connected() ) { MQTT_connect( MQTTTIMEOUT ); };
  // Convert the String object to a character array, so that MQTT can actually publish it.
  char post[64]; article.toCharArray(post, 128);
  // Publish the post to the topic:
  local_status.publish(post); Serial.println(article);

  /* -----------------------------------------------------> 
  Preparing MQTT client last will:
  TODO- Configure MQTT to change status/bart topic to "offline" after timout */
}

void loop()
{
  /***********************************************************************************************$ 
  This section processes MQTT inputs: */

  /* -----------------------------------------------------> 
  MQTT-Input 01 - 'Drink Order':
  TODO- Subscribe to Drink Order topic, and listen */

  /***********************************************************************************************$ 
  This section executes conditional logic based on MQTT inputs: */
  digitalWrite(LED, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(500);               // wait for a second
  digitalWrite(LED, LOW);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(MOTORCMD, HIGH);   // turn the currently selected motor on

  /***********************************************************************************************$ 
  This section processes MQTT outputs: */
  // Verify MQTT broker is connected.
  if ( !mqtt.connected() ) { MQTT_connect( MQTTTIMEOUT ); };
  
  /* -----------------------------------------------------> 
  MQTT-Out 01 - Preparing 'Ready for Drink' flag: */

  // Clears the trigPin
  digitalWrite(TRIGGERPIN, LOW);
  delayMicroseconds(2);
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIGGERPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGERPIN, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHOPIN, HIGH);
  
  // Calculating the distance
  distance= duration*0.034/2;
  
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);

  String readyflag = "ready";

  // Prepare a string to publish the values to MQTT
  char post[64]; readyflag.toCharArray(post, 64);
  
  // Publish the post to the topic:
  local_readyflag.publish(post); Serial.println(readyflag);
};
