#include "ESPAsyncWebServer.h"
#include "SPIFFS.h" 
#include <ArduinoJson.h> 
#include "dbgLevels.h"
#define DEBUG_LEVEL DBG_L
#include "debug.h"

#include "Config.hpp"
#include "Devices.hpp"
#include "Util.hpp"

// TODO: what about user password access to server 
// or even better dont start server unless button pushed ....

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

AsyncWebSocketClient* wsClient;

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


void setupWebServer() {
	INFO("Setting up webserver\n");
	server.serveStatic("/config",SPIFFS,"/config.html");
	server.serveStatic("/devices",SPIFFS,"/devices.html");
	server.serveStatic("/",SPIFFS,"/index.html");



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
		request->send(SPIFFS, "/config.html");
	});

	server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/config.json","application/json");
	});

	server.on("/devices", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/devices.html");
	});

	server.on("/devices.json", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/devices.json","application/json" );
	});

	server.on("/config", HTTP_POST, [&](AsyncWebServerRequest *request){
		// assigments here are ugly as fk but AsyncWebsever operates on arduino String...
		if (request->hasParam("ssid", true))				config.ssid = request->getParam("ssid", true)->value().c_str();
		if (request->hasParam("password", true)) 			config.password= request->getParam("password", true)->value().c_str();
		if (request->hasParam("hostname", true))			config.hostname= request->getParam("hostname", true)->value().c_str();
		if (request->hasParam("ipaddress", true))			config.mqttServer, request->getParam("ipaddress", true)->value();
		if (request->hasParam("apipaddress", true))			config.mqttServer, request->getParam("apipaddress", true)->value();
		if (request->hasParam("netmask", true))				config.mqttServer, request->getParam("netmask", true)->value();
		if (request->hasParam("mqttServer", true))			config.mqttServer, request->getParam("mqttServer", true)->value().c_str();
		if (request->hasParam("mqttPort", true))			config.mqttPort= request->getParam("mqttPort", true)->value().c_str();
		if (request->hasParam("mqttUser", true))			config.mqttUser= request->getParam("mqttUser", true)->value().c_str();
		if (request->hasParam("mqttPassword", true))		config.mqttPassword= request->getParam("mqttPassword", true)->value().c_str();
		if (request->hasParam("mqttScanTopicPrefix", true))	config.mqttScanTopicPrefix= request->getParam("mqttScanTopicPrefix", true)->value().c_str();
		if (request->hasParam("mqttStateTopicPrefix", true))config.mqttStateTopicPrefix= request->getParam("mqttStateTopicPrefix", true)->value().c_str();
		if (request->hasParam("room", true))				config.room= request->getParam("room", true)->value().c_str();
		if (request->hasParam("ntpServer", true))			config.room= request->getParam("ntpServerroom", true)->value().c_str();
		if (request->hasParam("gmtOffset", true))			config.room= request->getParam("gmtOffset", true)->value().c_str();
		if (request->hasParam("daylightOffset", true))		config.room= request->getParam("daylightOffset", true)->value().c_str();
		if (config.save()){
			request->send(200, "text/html", restart_html);
		}else{
			request->send(200, "text/html", "<br><br><br><div style='text-align:center;'><h3><b>Failed to save data!<b> Rebooting esp</div>");
		}
				 
		delay(1000);
		ESP.restart();
	});
/*
	server.onRequestBody( [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            if ((request->url() == "/devices") && (request->method() == HTTP_POST)) {
			DeserializationError desError;
			StaticJsonDocument jsonDoc<JSON_DOC_SIZE>;
			if (DeserializationError::Ok == deserializeJson(jsonDoc, (const char*)data))
			{
				JsonArray devices = jsonDoc["devices"].as<JsonArray>();

				DBG("devices count : %d", devices["devices"].size());
			}

			request->send(200, "application/json", "{ \"status\": 0 }");
		}
    );
*/
	server.onNotFound(notFound);

	ws.onEvent(onWsEvent);
	server.addHandler(&ws);
	server.begin();

	INFO("Webserver & Webservice started.\n");
} 

void notFound(AsyncWebServerRequest *request) {
		request->send(404, "text/plain", "Whoot !? i have no clue what that is. (404)");
}

String fillConfigTemplate(const String& var){
	String retValue; 
	if(var == "SSID"){							retValue = config.ssid.c_str();
	}else if(var == "PASSWORD"){				retValue = config.password.c_str();
	}else if(var == "HOSTNAME"){				retValue = config.hostname.c_str();
	}else if(var == "ROOM"){					retValue = config.room.c_str();
	}else if(var == "MQTTSERVER"){				retValue = config.mqttServer.c_str();
	}else if(var == "MQTTPORT"){				retValue = config.mqttPort.c_str();
	}else if(var == "MQTTUSER"){				retValue = config.mqttUser.c_str();
	}else if(var == "MQTTPASSWORD"){			retValue = config.mqttPassword.c_str();
	}else if(var == "MQTTSTATETOPICPREFIX"){	retValue = config.mqttStateTopicPrefix.c_str();
	}else if(var == "MQTTSCANTOPICPREFIX"){		retValue = config.mqttScanTopicPrefix.c_str();

//TODO: Rewrite to new device structure
//this an ugly kludge untill a rewrite of the web form to query for the number of devices ....

	}else if(var.startsWith("DEVICE")) {
			int ix =var.substring(6).toInt()-1;
			if ( ix > devices.count) {
				retValue="";
			}else{
				retValue = devices[ix]->second.c_str();
			}
	}else if(var.startsWith("UUID")){
			int ix =var.substring(4).toInt()-1;
			if (ix>devices.count) {
				retValue="";
			}else{
				retValue = devices[ix]->first.c_str();
			}
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

void sendWsText(MsgType type,const char *message) {
 char * msgp;
 char * levelp;

	if (wsClient != nullptr && wsClient->canSend()) {
		asprintf(&msgp,"{ \"type\": \"%s\", \"data\": %s}", MsgTypeTxt[type], message );
		wsClient->text(msgp);
		VERBOSE("WsText sent  message %s\n",msgp);
		free(msgp);
	}
}
