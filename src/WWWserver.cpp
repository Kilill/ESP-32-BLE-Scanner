#include "ESPAsyncWebServer.h"
#include "fileStructs.h"
#include "SPIFFS.h" 
#include <ArduinoJson.h> 


AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

AsyncWebSocketClient* wsClient;

extern Config config;
extern Devices devices;
extern bool saveConfig(Config &config);
extern bool saveDevices(Devices &devices);

String fillConfigTemplate(const String&);
void notFound(AsyncWebServerRequest *request);
void write_to_logs(const char* new_log_entry);
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);



const char restart_html[] PROGMEM = "<html>\
<body>\
    <p style=\"text-align:center;font-size:25px\">Saved, now restarting esp, please wait...&nbsp;<span id=\"seconds\">7</span> seconds.</p>\
    <script>\
        var seconds = 7;\
        var foo;\
        foo  = setInterval(function() {\
            document.getElementById(\"seconds\").innerHTML = seconds;\
            seconds--;\
            if (seconds == -1) {\
                clearInterval(foo);\
                document.location.href = \"http://\"+document.location.hostname;\
            }\
        }, 1000);\
    </script>\
</body>\
</html>";


void setupWebServer(Config &config) {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  }); 

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/wsclient.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/wsclient.js", "text/css");
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/config.html", String(), false, fillConfigTemplate);
  });

  server.on("/devices", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/devices.html", String(), false, fillConfigTemplate);
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
        if (request->hasParam("mqttScanTopicPrefix", true)) {
            strlcpy(config.mqttScanTopicPrefix, request->getParam("mqttScanTopicPrefix", true)->value().c_str(), sizeof(config.mqttScanTopicPrefix));
         }
         if (request->hasParam("mqttStateTopicPrefix", true)) {
            strlcpy(config.mqttStateTopicPrefix, request->getParam("mqttStateTopicPrefix", true)->value().c_str(), sizeof(config.mqttStateTopicPrefix));
         }
         if (saveConfig(config)){
           request->send(200, "text/html", restart_html);
         }else{
           request->send(200, "text/html", "<br><br><br><div style='text-align:center;'><h3><b>Failed to save data!<b> Rebooting esp</div>");
         }
         delay(1000);
         ESP.restart();
    });

  server.on("/devices", HTTP_POST, [&](AsyncWebServerRequest *request){
         if (request->hasParam("device_uuid1", true)) {
            strlcpy(devices.device_uuid1 , request->getParam("device_uuid1", true)->value().c_str(), sizeof(devices.device_uuid1));
         }
         if (request->hasParam("device_uuid2", true)) {
            strlcpy(devices.device_uuid2 , request->getParam("device_uuid2", true)->value().c_str(), sizeof(devices.device_uuid2));
         }
         if (request->hasParam("device_uuid3", true)) {
            strlcpy(devices.device_uuid3 , request->getParam("device_uuid3", true)->value().c_str(), sizeof(devices.device_uuid3));
         }
         if (request->hasParam("device_name1", true)) {
            strlcpy(devices.device_name1 , request->getParam("device_name1", true)->value().c_str(), sizeof(devices.device_name1));
         }
         if (request->hasParam("device_name2", true)) {
            strlcpy(devices.device_name2 , request->getParam("device_name2", true)->value().c_str(), sizeof(devices.device_name2));
         }
         if (request->hasParam("device_name3", true)) {
            strlcpy(devices.device_name3 , request->getParam("device_name3", true)->value().c_str(), sizeof(devices.device_name3));
         }              
         if (saveDevices(devices)){
           request->send(200, "text/html", restart_html);
         }else{
           request->send(200, "text/html", "<br><br><br><div style='text-align:center;'><h3><b>Failed to save data!<b> Rebooting esp</div>");
         }
         delay(1000);
         ESP.restart();
    });  

  server.onNotFound(notFound);

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.begin();

  Serial.println("\nWebserver & Webservice started.");
} 

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "naa, wrong request. (404)");
}

String fillConfigTemplate(const String& var){
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
  }else if(var == "MQTTSTATETOPICPREFIX"){
    retValue = config.mqttStateTopicPrefix;
  }else if(var == "MQTTSCANTOPICPREFIX"){
    retValue = config.mqttScanTopicPrefix;
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

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    wsClient = client;
  } else if(type == WS_EVT_DISCONNECT){
    wsClient = nullptr;
  }
}

void sendWsText(char level[],char message[]) {
  
  if (wsClient != nullptr && wsClient->canSend()) {
    char messageOut[250];
    sprintf(messageOut, "{ \"type\": \"%s\", \"data\": %s}", level, message );
    wsClient->text(messageOut);
  }
}


                            