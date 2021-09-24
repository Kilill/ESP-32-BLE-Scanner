#include "Arduino.h"
#include "SPIFFS.h" 
#include <ArduinoJson.h> 
#include "fileStructs.h"

File loadFile(String filename);
void deserialise(const JsonDocument& doc,File file);
const char* jsonValueOrDefault(const char* iets, const char* defaultString);


void loadConfiguration(String filename, Config &config) {
 
  File configFile = loadFile(filename);
  
  if (configFile) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    if (error){
      Serial.println(F("Failed to read file, using default configuration"));
    }
    
    strlcpy(config.ssid, jsonValueOrDefault(doc["ssid"],""), sizeof(config.ssid));
    strlcpy(config.password, jsonValueOrDefault(doc["password"],""), sizeof(config.password));
    strlcpy(config.hostname, jsonValueOrDefault(doc["hostname"],"ESP 32 BLE Scanner"), sizeof(config.hostname));
    strlcpy(config.room, jsonValueOrDefault(doc["room"],""), sizeof(config.room));
    strlcpy(config.mqttServer, jsonValueOrDefault(doc["mqttServer"],""), sizeof(config.mqttServer));
    strlcpy(config.mqttPort, jsonValueOrDefault(doc["mqttPort"],""), sizeof(config.mqttPort));
    strlcpy(config.mqttUser, jsonValueOrDefault(doc["mqttUser"],""), sizeof(config.mqttUser));
    strlcpy(config.mqttPassword, jsonValueOrDefault(doc["mqttPassword"],""), sizeof(config.mqttPassword));
    // Close the file (Curiously, File's destructor doesn't close the file)
    configFile.close();
  }else{
     Serial.println("Failed to open "+filename);
  }
}

const char* jsonValueOrDefault(const char* iets, const char* defaultString){
  const char* ret = iets;
  if (String(ret).length() < 1){
    ret = defaultString;
  }
  return ret;
}



void loadDevices(String filename, Devices &devices) {
  File devicesFile = loadFile(filename);
  
  if (devicesFile) {
    StaticJsonDocument<512> devicesDoc;
    DeserializationError error = deserializeJson(devicesDoc, devicesFile);
    if (error){
      Serial.println(F("Failed to read file, using default configuration"));
    }
    strlcpy(devices.device_uuid1,devicesDoc["device_uuid1"] | "", sizeof(devices.device_uuid1));
    strlcpy(devices.device_name1,devicesDoc["device_name1"] | "", sizeof(devices.device_name1));
    strlcpy(devices.device_uuid2,devicesDoc["device_uuid2"] | "", sizeof(devices.device_uuid2));
    strlcpy(devices.device_name2,devicesDoc["device_name2"] | "", sizeof(devices.device_name2));
    strlcpy(devices.device_uuid3,devicesDoc["device_uuid3"] | "", sizeof(devices.device_uuid3));
    strlcpy(devices.device_name3,devicesDoc["device_name3"] | "", sizeof(devices.device_name3));
    // Close the file (Curiously, File's destructor doesn't close the file)
    devicesFile.close();
  }else{
     Serial.println("Failed to open "+filename);
  }
}

File loadFile(String filename){
  if (!SPIFFS.begin()){
    Serial.println(F("!An error occurred during SPIFFS mounting"));
  }
  return SPIFFS.open(filename);
}

bool saveConfig(Config  &config){
  bool success = true;
  File outfile = SPIFFS.open("/config.json","w");
  StaticJsonDocument<1000> doc;
  doc["room"]         = config.room;
  doc["ssid"]         = config.ssid;
  doc["password"]     = config.password;
  doc["hostname"]     = config.hostname;
  doc["mqttServer"]   = config.mqttServer;
  doc["mqttPort"]     = config.mqttPort;
  doc["mqttUser"]     = config.mqttUser;
  doc["mqttPassword"] = config.mqttPassword;

  if(serializeJson(doc, outfile)==0) {
        Serial.println("Failed to write settings to SPIFFS config file");
        success = false;
  }
  return success;

}

