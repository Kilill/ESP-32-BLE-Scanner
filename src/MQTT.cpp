#include <PubSubClient.h>
#include "fileStructs.h"
#include <WiFi.h>

extern WiFiClient espClient;
extern Config config;
extern void sendWsText(const char *level,const char *message);

PubSubClient pubSubclient(espClient);

void MQTTreconnect();
bool mqttSettingsAvailable = false;
char mqttScanTopic[150];
char mqttStateTopic[150];

void setupMQTT() {
  if (String(config.mqttServer).length() > 1 && String(config.mqttPort).length() > 1){
    mqttSettingsAvailable = true;
    pubSubclient.setServer(config.mqttServer,String(config.mqttPort).toInt());
    strcat(mqttScanTopic,config.mqttScanTopicPrefix);
    strcat(mqttScanTopic,config.room);
    strcat(mqttStateTopic,config.mqttStateTopicPrefix);
    strcat(mqttStateTopic,config.room);
    MQTTreconnect();
  }else{
    Serial.println("No MQTT server and/or port set, can't set up the connection!");
  }
}


void MQTTreconnect() {
  // Loop until we're reconnected
  while (!pubSubclient.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    bool connected = pubSubclient.connect(config.hostname,
                             !String(config.mqttUser).length()?NULL:config.mqttUser,
                             !String(config.mqttPassword)?NULL:config.mqttPassword );
    if (!connected) {
      Serial.print("failed, rc=");
      Serial.print(pubSubclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }else{
      Serial.println("MQTT connected.");
    }
  }
}

void publishToScanTopic(char mqtt_msg[]){
  if (pubSubclient.connected()){
    pubSubclient.publish(mqttScanTopic, mqtt_msg);
  }else{
     sendWsText("warn","{\"msg\":\"MQTT not connected, can't publish.\"}");
  }  
}

