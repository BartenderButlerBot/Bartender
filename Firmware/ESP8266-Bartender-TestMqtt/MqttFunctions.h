#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Defining the connection to the MQTT broker:
#define MQTT_SERVER     "192.168.1.78"          // MQTT broker IP address, hardcoded
#define MQTT_SERVERPORT 1883                    // MQTT service port, unsecured
#define MQTT_USERNAME   ""                      
#define MQTT_PASSWORD   ""
#define MQTT_ID         "bart"                  // Local client identifier
Adafruit_MQTT_Client    mqtt( &wifiClient, MQTT_SERVER, MQTT_SERVERPORT );

String ID = "ID - ";
String mqttID(MQTT_ID);

void MQTT_connect(int blockingTime){            // Function to connect and reconnect as necessary;
  int8_t rc;
  if (mqtt.connected()) {                       // If already connected...
    mqtt.processPackets( blockingTime );
    if ( !mqtt.ping() ) {
      mqtt.disconnect();
    };
  } else {
    Serial.printf("Connecting to MQTT ...");
    uint8_t retries = 15;
    while ( (rc = mqtt.connect()) != 0 ) {      // connect will return 0 for connected
      Serial.println( mqtt.connectErrorString( rc ) );
      Serial.println( "Retrying MQTT connection in 5 seconds ..." );
      mqtt.disconnect();
      delay(5000);
      retries--;
      if ( retries == 0 )
        while ( true );     // just give up and wait for WTD reset
    };
    Serial.println("\t\t\t\t [OK]" );
    Serial.print( "MQTT ID: " ); Serial.println( mqttID ); Serial.println();
  };
}
