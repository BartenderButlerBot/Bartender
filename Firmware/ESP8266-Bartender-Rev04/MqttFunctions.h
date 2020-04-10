#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Defining the connection to the MQTT broker:
#define MQTT_SERVER     "192.168.1.78"          // MQTT broker IP address, hardcoded
#define MQTT_SERVERPORT 1883                    // MQTT service port, unsecured
#define MQTT_USERNAME   ""                      //
#define MQTT_PASSWORD   ""
#define MQTT_ID         "bart"                  // Local client identifier
Adafruit_MQTT_Client    mqtt( &wifiClient, MQTT_SERVER, MQTT_SERVERPORT );

// MQTT output topics from the local bart_:
#define bart_heatbeat     "bart/heartbeat"      // Update MQTT client as online
Adafruit_MQTT_Publish     local_connection = Adafruit_MQTT_Publish( &mqtt, bart_heatbeat);
#define bart_status       "bart/status"         // General "ready for order" flag
Adafruit_MQTT_Publish     local_status = Adafruit_MQTT_Publish( &mqtt, bart_status );
#define bart_cupAlignment "bart/cupAlignment"   // SR04 Ultrasonic, single cup alignment sensor
Adafruit_MQTT_Publish     local_cupAlignment = Adafruit_MQTT_Publish( &mqtt, bart_cupAlignment );

// MQTT input topics from the core applicatiom, or the "app":
#define app_order         "app/order"           // Order recieved from the user
Adafruit_MQTT_Subscribe   app_orderFeed = Adafruit_MQTT_Subscribe( &mqtt, app_order, MQTT_QOS_1 );

// MQTT input topics from the Butler "Alfred", or the "bot":
#define bot_status        "bot/status"          // Butler location status; "bar", "barbound", "base", "basebound"
Adafruit_MQTT_Subscribe   bot_statusFeed = Adafruit_MQTT_Subscribe( &mqtt, bot_status, MQTT_QOS_1 );

String                  ID        = "ID - ";
String                  mqttID( MQTT_ID );

void MQTT_connect( int blockingTime )           // Function to connect and reconnect as necessary;
{
  int8_t rc;
  if ( mqtt.connected() )                       // If already connected...
  {
    mqtt.processPackets( blockingTime );
    if ( !mqtt.ping() )
      mqtt.disconnect();
  }
  else
  {
    Serial.printf("Connecting to MQTT ...");
    uint8_t retries = 15;
    while ( (rc = mqtt.connect()) != 0 )      // connect will return 0 for connected
    {
      Serial.println( mqtt.connectErrorString( rc ) );
      Serial.println( "Retrying MQTT connection in five seconds ..." );
      mqtt.disconnect();
      delay( 5000 );
      retries--;
      if ( retries == 0 )
        while ( true );     // just give up and wait for WTD reset
    }
    Serial.println("\t\t\t  [OK]");
    Serial.print( "MQTT ID: " ); Serial.println( mqttID ); Serial.println();
  }
}
