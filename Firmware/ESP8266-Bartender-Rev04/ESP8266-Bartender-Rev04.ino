/*
   AUTHOR                  : Edward Daniel Nichols
   LAST CONTRIBUTION DATE  : April 3rd, 2020
   Rev04
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

/*Useful global variables: */
char    article;
char*   mqttmsg;
uint16_t len;
int     distance;

/*Function protoypes:  */
void cfgInfo();                           // Prints technical info from stack
void pumpOperate(int mL, int type = 0);   // Selects an active pump line through MUX output
void pumpSelect(int pump, int type = 0);  // Operates the active pump line on MUX output
double sr_distance();                     // Returns SR04 ultrasonic distance measurement, cm

void app_orderCallback(char* mqttmsg, uint16_t len) {
  Serial.printf("\tSubscription message \"%s\": %s", app_order, mqttmsg);
  Serial.printf("\t[ACK]\n");
};

void bot_statusCallback(char* mqttmsg, uint16_t len) {
  Serial.printf("\tSubscription message \"%s\": %s", bot_status, mqttmsg);
  Serial.printf("\t[ACK]\n");
};

void setup()
{
  /***********************************************************************************************
    This section prepares the Serial Ports: */
  // Start up the serial port connection and announce title.
  Serial.begin( 115200 );
  Serial.println( "ESP8266 MQTT Bartender - Rev04" );
  cfgInfo();

  /***********************************************************************************************$
    This section prepares WiFi 802.11 & MQTT: */
  WifiSetup();                          // Setup and connect to WiFi.
  mqtt.will(bart_heatbeat, "offline");  // If this unit does not ping/update for a while, MQTT will register it as offline 
  MQTT_connect( MQTTTIMEOUT );          // Prompt MQTT to connect for the first time.
  /* ----------------------------------------------------->
    Alerts MQTT & system devices that this unit is online */
  // Prepare a string to publish the values to MQTT
  char article[] = "online";
  Serial.printf("Publishing to MQTT: \"%s\" ...", article);
  if (local_connection.publish(article)) {
    Serial.printf("\t[ACK]\n");
  } else {
    Serial.println("\t[NACK]\n");
  };

  /* ----------------------------------------------------->
    Makes the Subscriptions to relevant MQTT topics */
  app_orderFeed.setCallback(app_orderCallback);
  bot_statusFeed.setCallback(bot_statusCallback);

  mqtt.subscribe(&app_orderFeed);   // Subscribe to app/order MQTT topic
  mqtt.subscribe(&bot_statusFeed);  // Subscribe to bot/status MQTT topic

  /***********************************************************************************************$
    This section prepares pumps & sensors: */
  pinMode(LED, OUTPUT);
  /* ----------------------------------------------------->
    Configure SR04 ultrasonic sensor pins */
  pinMode(SR_TRIGPIN, OUTPUT);      // Sets the trigPin as an Output
  pinMode(SR_ECHOPIN, INPUT);       // Sets the echoPin as an Input
  /* ----------------------------------------------------->
    Configure SR04 ultrasonic sensor pins */
  pinMode(PUMP_S0, OUTPUT);
  pinMode(PUMP_S1, OUTPUT);
  pinMode(PUMP_S2, OUTPUT);
  pinMode(PUMP_CMD, OUTPUT);
  /* ----------------------------------------------------->
    Failsafe measures */
  digitalWrite(LED, HIGH);          // Ensure LED is OFF (LOW=ON, see PCB)
  digitalWrite(PUMP_CMD, LOW);      // Ensure actively selected pump is OFF
  pumpSelect(8);                    // Ensure pump selection defaults to rarely connected unit
}

void loop()
{
  //TEST
  digitalWrite(LED, LOW);
  delay(500);

  /***********************************************************************************************$
    This section contains loop fail-safe measures. */

  if ( !mqtt.connected() ) {        // Verify MQTT broker is connected
    MQTT_connect( MQTTTIMEOUT );    // If not connected, then connect
  };

  /* ----------------------------------------------------->
    Alert MQTT that this unit is "idle" */
  char article[] = "idle";
  Serial.printf("Publishing to MQTT: \"%s\" ...", article);
  if (local_status.publish(article)) {
    Serial.printf("\t\t[ACK]\n");
  } else {
    Serial.println("\t\t[NACK]\n");
  };

  mqtt.processPackets(10000);

  // Ping the server to refresh connection, or disconnect if unreachable.
  if (!mqtt.ping()) {
    mqtt.disconnect();
  };

  digitalWrite(LED, HIGH);
  delay(2500);
};

void cfgInfo() {
  // Print optional info to serial
  Serial.println("Configuration Information...");
  Serial.printf("\tFree sketch space: %7d bytes\n", ESP.getFreeSketchSpace() );
  Serial.printf("\tFree sketch space: %7d bytes\n", ESP.getFreeSketchSpace() );
  Serial.printf("\t        Free heap: %7d bytes\n", ESP.getFreeHeap()        );
  Serial.printf("\t  Flash chip size: %7d bytes\n", ESP.getFlashChipSize()   );
  Serial.println();
};

void pumpSelect(int pump, int type) {
  // Given the static pinout, #defined above
  switch (type) {
    /*This bit doesn't actually do anything right now, it's purely aesthetic.
      Maybe in the future, this could be used to validate selections and report errors.*/
    case 1: // Single-pump
      Serial.printf("Selecting S-Pump --> M%d ...", pump);
      break;
    case 2: // Double-pump
      Serial.printf("Selecting D-Pump --> M%d ...", pump);
      break;
    default: // Purely aesthetic print-to-serial
      Serial.printf("Selecting   Pump --> M%d ...", pump);
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
      Serial.printf("\t\t [OK]\n");
      Serial.printf("^--- Override selection to M8!\n", pump);
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
  return duration * 0.034 / 2;
}
