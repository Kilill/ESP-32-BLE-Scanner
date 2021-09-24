#include <Arduino.h>
#include "fileStructs.h"

// refs to externally defined methods

String configFilename = "/config.json";  
String devicesFilename = "/devices.json"; 

extern void loadConfiguration(String filename, Config &config);
extern void loadDevices(String filename, Devices &devices);
extern void setupWIFI(Config &config);
extern void setupWebServer(Config &config);

Config config;
Devices devices; 

void setup() {
  Serial.begin(115200);
  loadConfiguration(configFilename,config);
  loadDevices(devicesFilename,devices);
  setupWIFI(config);
  setupWebServer(config);
}

void loop() {
 
} 