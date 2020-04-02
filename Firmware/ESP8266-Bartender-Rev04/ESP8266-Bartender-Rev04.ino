/*
   AUTHOR                  : Edward Daniel Nichols
   LAST CONTRIBUTION DATE  : April 1st, 2020
   Rev04 - Objective:
   - Print
*/

/*Required local libraries: IN THIS ORDER */
#include    "WifiFunctions.h"
#include    "MqttFunctions.h"
#include   <Wire.h>

/*Required PINOUT mappings: */
#define LED           0
#define PUMP_S0       12
#define PUMP_S1       13
#define PUMP_S2       2       // PULLED HIGH
#define PUMP_CMD      14
#define SR_TRIGPIN    15      // PULLED LOW
#define SR_ECHOPIN    16

/*Configurable directives:  */
#define MQTTTIMEOUT     333
#define PUMP_SGL_SCALE  500   // (Single-pump) time required for 1 mL, in milliseconds <-- TODO, validate!
#define PUMP_DBL_SCALE  250   // (Double-pump) time required for 1 mL, in milliseconds <-- TODO, validate!
#define PUMP_TST_SCALE  100   // Arbitrary duration per mL desired, in milliseconds

// Definitions for MQTT article/publish
char    post;
String  article;
int     distance;

/*Custom function prototypes:
  - cfgInfo(): provides information from ESP stack
  - pumpSelect(): Selects an active pump line through MUX output
  - pumpOperate(): Operates the active pump line on MUX output
*/
void pumpOperate(int mL, int type=0);
void pumpSelect(int pump, int type=0);

void setup()
{
  /***********************************************************************************************
    This section prepares the Serial Ports: */
  // Start up the serial port connection and announce title.
  Serial.begin( 115200 );
  Serial.println( "ESP8266 MQTT Bartender - Rev04" );
  cfgInfo();

  /***********************************************************************************************$
    This section prepares GPIO pins: */
  pinMode(LED, OUTPUT);
  pinMode(SR_TRIGPIN, OUTPUT);      // Sets the trigPin as an Output
  pinMode(SR_ECHOPIN, INPUT);       // Sets the echoPin as an Input
  pinMode(PUMP_S0, OUTPUT);
  pinMode(PUMP_S1, OUTPUT);
  pinMode(PUMP_S2, OUTPUT);
  pinMode(PUMP_CMD, OUTPUT);

  /***********************************************************************************************$
    This section prepares WiFi 802.11 & MQTT: */
  WifiSetup();                      // Setup and connect to WiFi.
  MQTT_connect( MQTTTIMEOUT );      // Prompt MQTT to connect for the first time.

  /* ----------------------------------------------------->
  MQTT-Output - Updating MQTT of connection status */
  // Prepare a string to publish the values to MQTT
  char article[] = "online";
  Serial.printf("\tPublishing to MQTT: \"%s\" ...", article);
  if(local_connection.publish(article)) { Serial.printf("\t[ACK]\n"); } else { Serial.println("\t[NACK]\n"); };

  /* ----------------------------------------------------->
  MQTT-Subscriptions */
  mqtt.subscribe(&app_orderFeed);
  mqtt.subscribe(&bot_statusFeed);
}

void loop()
{
  /***********************************************************************************************$
    This section contains loop fail-safe measures. */
  digitalWrite(PUMP_CMD, LOW); // Ensure actively selected pump is OFF

  // Verify MQTT broker is connected.
  if ( !mqtt.connected() ) {
    MQTT_connect( MQTTTIMEOUT );
  };

  String readyflag = "ready";

  // Prepare a string to publish the values to MQTT
  char post[64]; readyflag.toCharArray(post, 64);

  // Publish the post to the topic:
  local_status.publish(post); Serial.println(readyflag);

  // Ping the server to refresh connection, or disconnect if unreachable.
  if(!mqtt.ping()){
    mqtt.disconnect();
  }
};

void cfgInfo() {
  // Print optional info to serial
  Serial.println("Configuration Information...");
  Serial.print(" IP address: "); Serial.println(WiFi.localIP());
  Serial.printf(" Free sketch space: %7d bytes\n", ESP.getFreeSketchSpace() );
  Serial.printf(" Free sketch space: %7d bytes\n", ESP.getFreeSketchSpace() );
  Serial.printf("         Free heap: %7d bytes\n", ESP.getFreeHeap()        );
  Serial.printf("   Flash chip size: %7d bytes\n", ESP.getFlashChipSize()   );
  Serial.println();
};

void pumpSelect(int pump, int type) {
  // Given the static pinout, #defined above
  switch (type) {
    /*This bit doesn't actually do anything right now, it's purely aesthetic.
      Maybe in the future, this could be used to validate selections and report errors.*/
    case 1: // Single-pump
      Serial.printf("\tSelecting S-Pump --> M%d ...", pump);
      break;
    case 2: // Double-pump
      Serial.printf("\tSelecting D-Pump --> M%d ...", pump);
      break;
    default: // Purely aesthetic print-to-serial
      Serial.printf("\tSelecting   Pump --> M%d ...", pump);
  };
  switch (pump) {
    /*This bit is where the magic actually happens. Pumps M1-M8 are connected to MUX output,
      on selection PUMP_Sx pins defined above. Assume M8 for troubleshooting. */
    case 1: // M1-PWM
      digitalWrite(PUMP_S0, LOW);
      digitalWrite(PUMP_S1, LOW);
      digitalWrite(PUMP_S2, LOW);
      Serial.printf("\t[OK]\n");
      break;
    case 2: // M2-PWM
      digitalWrite(PUMP_S0, HIGH);
      digitalWrite(PUMP_S1, LOW);
      digitalWrite(PUMP_S2, LOW);
      Serial.printf("\t[OK]\n");
      break;
    case 3: // M3-PWM
      digitalWrite(PUMP_S0, LOW);
      digitalWrite(PUMP_S1, HIGH);
      digitalWrite(PUMP_S2, LOW);
      Serial.printf("\t[OK]\n");
      break;
    case 4: // M4-PWM
      digitalWrite(PUMP_S0, HIGH);
      digitalWrite(PUMP_S1, HIGH);
      digitalWrite(PUMP_S2, LOW);
      Serial.printf("\t[OK]\n");
      break;
    case 5: // M5-PWM -> M5-A & M5-B
      digitalWrite(PUMP_S0, LOW);
      digitalWrite(PUMP_S1, LOW);
      digitalWrite(PUMP_S2, HIGH);
      Serial.printf("\t[OK]\n");
      break;
    case 6: // M6-PWM -> M6-A & M6-B
      digitalWrite(PUMP_S0, HIGH);
      digitalWrite(PUMP_S1, LOW);
      digitalWrite(PUMP_S2, HIGH);
      Serial.printf("\t[OK]\n");
      break;
    case 7: // M7-PWM -> M7-A & M7-B
      digitalWrite(PUMP_S0, LOW);
      digitalWrite(PUMP_S1, HIGH);
      digitalWrite(PUMP_S2, HIGH);
      Serial.printf("\t[OK]\n");
      break;
    default: // M8-PWM -> NO CONNECTIONS EXPECTED HERE
      digitalWrite(PUMP_S0, HIGH);
      digitalWrite(PUMP_S1, HIGH);
      digitalWrite(PUMP_S2, HIGH);
      Serial.printf("\tATTN\n");
      Serial.printf("\t\t ^--- Override selection to M8!\n", pump);
      break;
  };
};

void pumpOperate(int v, int type) {
  // For the pump which was selected with 'pumpSelect' -> Dispense "v" amount in mL (milliLiters)
  int pumpTime; // in ms
  switch (type) {
    case 1: // Single-pump
      digitalWrite(PUMP_CMD, HIGH);
      pumpTime = v * PUMP_SGL_SCALE;  // calc for mL
      delay(pumpTime);      // pour until "mL" have been dispensed
      digitalWrite(PUMP_CMD, LOW);
      break;
    case 2: // (Double-pump): 1 mL = 250 ms <- requires validation!!!
      digitalWrite(PUMP_CMD, HIGH);
      pumpTime = v * PUMP_DBL_SCALE;  // calc for mL
      delay(pumpTime);      
      digitalWrite(PUMP_CMD, LOW);
      break;
    default: // (Arbitrary): 1 "mL" = 100 ms
      digitalWrite(PUMP_CMD, HIGH);
      pumpTime = v * PUMP_TST_SCALE;  // calc for arbitrary "mL"
      delay(pumpTime);      
      digitalWrite(PUMP_CMD, LOW);
      break;
  };
};

double sr_distance() {
  long duration; // Instantiate local pulse travel time
  digitalWrite(SR_TRIGPIN, LOW); delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(SR_TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(SR_TRIGPIN, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(SR_ECHOPIN, HIGH);

  // Calculating the distance
  return duration*0.034/2;
}
