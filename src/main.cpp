#include <Arduino.h>
#include <PubSubClient.h>
#include "fileStructs.h"

String configFilename = "/config.json";  
String devicesFilename = "/devices.json"; 

extern void loadConfiguration(String filename, Config &config);
extern void loadDevices(String filename, Devices &devices);
extern void setupWIFI(Config &config);
extern void setupWebServer(Config &config);
extern void setupWebServer(Config &config);
extern void setupMQTT();
extern void MQTTreconnect();
extern void setupBluetoothScanner();
extern void runScan();


extern PubSubClient pubSubclient;
extern bool mqttSettingsAvailable;
extern bool wifiAccesPointModeEnabled;


Config config;
Devices devices; 

void setup() {
  Serial.begin(115200);
  loadConfiguration(configFilename,config);
  loadDevices(devicesFilename,devices);
  setupWIFI(config);
  setupWebServer(config);
  setupMQTT();
  setupBluetoothScanner();
}

void loop() {
  if (!wifiAccesPointModeEnabled && mqttSettingsAvailable && !pubSubclient.connected()) {
      MQTTreconnect();
  }
  if (!wifiAccesPointModeEnabled) {
      runScan();
  }
  pubSubclient.loop();
} 