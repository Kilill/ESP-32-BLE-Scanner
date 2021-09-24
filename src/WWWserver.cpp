#include "ESPAsyncWebServer.h"
#include "fileStructs.h"
#include "SPIFFS.h" 

AsyncWebServer server(80);
extern Config config;
extern Devices devices;
extern bool saveConfig(Config &config);

String fillTemplate(const String&);
void notFound(AsyncWebServerRequest *request);


void setupWebServer(Config &config) {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html");
  }); 

   // Load CSS
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/config.html", String(), false, fillTemplate);
    
  });

  server.on("/config", HTTP_POST, [&](AsyncWebServerRequest *request){
         if (request->hasParam("ssid", true)) {
            strlcpy(config.ssid, request->getParam("ssid", true)->value().c_str(), sizeof(config.ssid));
         }
         if (request->hasParam("password", true)) {
            strlcpy(config.password, request->getParam("password", true)->value().c_str(), sizeof(config.password));
         }
         if (request->hasParam("room", true)) {
            strlcpy(config.room, request->getParam("room", true)->value().c_str(), sizeof(config.room));
         }
         if (request->hasParam("hostname", true)) {
            strlcpy(config.hostname, request->getParam("hostname", true)->value().c_str(), sizeof(config.hostname));
         }
         if (request->hasParam("mqttServer", true)) {
            strlcpy(config.mqttServer, request->getParam("mqttServer", true)->value().c_str(), sizeof(config.mqttServer));
         }
         if (request->hasParam("mqttPort", true)) {
            strlcpy(config.mqttPort, request->getParam("mqttPort", true)->value().c_str(), sizeof(config.mqttPort));
         }
         if (request->hasParam("mqttUser", true)) {
            strlcpy(config.mqttUser, request->getParam("mqttUser", true)->value().c_str(), sizeof(config.mqttUser));
         }
         if (request->hasParam("mqttPassword", true)) {
            strlcpy(config.mqttPassword, request->getParam("mqttPassword", true)->value().c_str(), sizeof(config.mqttPassword));
         }                  
         if (saveConfig(config)){
           request->send(200, "text/html", "<br><div style='text-align:center;'><h3>Saved, now restarting esp..</h3></div>");
         }else{
           request->send(200, "text/html", "<br><br><br><div style='text-align:center;'><h3><b>Failed to save data!<b> Rebooting esp</div>");
         }
         delay(1000);
         ESP.restart();
    });

  server.onNotFound(notFound);
  server.begin();

  Serial.println("\nWebserver started.");
} 


void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "naa, wrong request. (404)");
}

String fillTemplate(const String& var){
  String retValue = String();
  if(var == "SSID"){
    retValue = config.ssid;
  }else if(var == "PASSWORD"){
    retValue = config.password;
  }else if(var == "HOSTNAME"){
    retValue = config.hostname;
  }else if(var == "ROOM"){
    retValue = config.room;
  }else if(var == "MQTTSERVER"){
    retValue = config.mqttServer;
  }else if(var == "MQTTPORT"){
    retValue = config.mqttPort;
  }else if(var == "MQTTUSER"){
    retValue = config.mqttUser;
  }else if(var == "MQTTPASSWORD"){
    retValue = config.mqttPassword;
  }else if(var == "DEVICENAME1"){
    retValue = devices.device_name1;
  }else if(var == "DEVICENAME2"){
    retValue = devices.device_name2;
  }else if(var == "DEVICENAME3"){
    retValue = devices.device_name3;
  }else if(var == "UUID1"){
    retValue = devices.device_uuid1;
  }else if(var == "UUID2"){
    retValue = devices.device_uuid2;
  }else if(var == "UUID3"){
    retValue = devices.device_uuid3;
  }
  return retValue;
}