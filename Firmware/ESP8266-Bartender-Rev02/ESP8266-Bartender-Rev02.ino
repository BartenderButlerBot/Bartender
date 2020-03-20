/*
   AUTHOR                  : Edward Daniel Nichols
   LAST CONTRIBUTION DATE  : March 19, 2020
   Rev03 - Objective:
   - Print 
*/

/*Required local libraries: IN THIS ORDER */
#include    "WifiFunctions.h"
#include    "MqttFunctions.h"
#include   <Wire.h>

/*Require definitions:  */
#define MQTTTIMEOUT   500
#define LED           0
#define MOTORCMD      14
#define S0            12
#define S1            13
#define S2            2 
#define TRIGGERPIN    15
#define ECHOPIN       16

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
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);
  digitalWrite(S2, HIGH);
  digitalWrite(MOTORCMD, LOW);  

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
  while(!Serial.read()){
    delay(50);
    //do nothing until something is read
  }
  /***********************************************************************************************$ 
  This section processes MQTT inputs: */

  /* -----------------------------------------------------> 
  MQTT-Input 01 - 'Drink Order':
  TODO- Subscribe to Drink Order topic, and listen */

  /***********************************************************************************************$ 
  This section executes conditional logic based on MQTT inputs: */
  digitalWrite(LED, HIGH);  // turn the LED on (HIGH is the voltage level)
  Serial.println("Checking on MQTT");
  delay(2000);               // wait for a second
  digitalWrite(LED, LOW);   // turn the LED on (HIGH is the voltage level)

  /***********************************************************************************************$ 
  This section processes MQTT outputs: */
  // Verify MQTT broker is connected.
  if ( !mqtt.connected() ) { MQTT_connect( MQTTTIMEOUT ); };
  
  /* -----------------------------------------------------> 
  Ultrasonic sensor shits

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
  */

  String readyflag = "dispensing";
  // Prepare a string to publish the values to MQTT
  char post[64]; readyflag.toCharArray(post, 64);
  // Publish the post to the topic:
  local_readyflag.publish(post); Serial.println(readyflag);

  // Select motor 0, and pump for 30 seconds:
  Serial.println("Pumping motor: 0");
  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);
  digitalWrite(S2, LOW);
  digitalWrite(MOTORCMD, HIGH);
  delay(30000);
  digitalWrite(MOTORCMD, LOW);

  // Select motor 1, and pump for 30 seconds:
  Serial.println("Pumping motor: 1");
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
  digitalWrite(S2, LOW);
  digitalWrite(MOTORCMD, HIGH);
  delay(30000);
  digitalWrite(MOTORCMD, LOW);

  // Select motor 2, and pump for 30 seconds:
  Serial.println("Pumping motor: 2");
  digitalWrite(S0, LOW);
  digitalWrite(S1, HIGH);
  digitalWrite(S2, LOW);
  digitalWrite(MOTORCMD, HIGH);
  delay(30000);
  digitalWrite(MOTORCMD, LOW);

  readyflag = "ready";
  // Prepare a string to publish the values to MQTT
  post[64]; readyflag.toCharArray(post, 64);
  // Publish the post to the topic:
  local_readyflag.publish(post); Serial.println(readyflag);
};
