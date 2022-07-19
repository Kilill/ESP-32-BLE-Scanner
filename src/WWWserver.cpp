#define ARDUINOJSON_ENABLE_COMMENTS 1
#include "Arduino.h"
#include <stdio.h>
#include <string>
#include <map>
#include "SPIFFS.h" 
#include <ArduinoJson.h> 
#include "ESPAsyncWebServer.h"

#include "Settings.hpp"
#include "Devices.hpp"
#include "Config.hpp"
#include "Util.hpp"

#include "dbgLevels.h"
//#define DEBUG_LEVEL DBG_L
#include "debug.h"

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

void notFound(AsyncWebServerRequest *request) {
		request->send(404, "text/plain", "Whoot !? i have no clue what that is. (404)");
}

/** /func read post json data and fill the config instance
 */

bool configPost(uint8_t * data,AsyncWebServerRequest *request){
	uint16_t count;
	bool fillOk , saveOk;
	DBG("configPost");

	StaticJsonDocument<CONFIG_DOC_SIZE> jdoc;
	DeserializationError desError;

	StaticJsonDocument<CONFIG_DOC_SIZE> doc;
	if ( (desError=deserializeJson(doc, (const char *) data)) != DeserializationError::Ok ) {
		ERR("failed to deserialize config Data");
		request->send(200, "text/html", "<div class=error>Failed to parse config data!</div>");
		return false;
	}
	
	if((fillOk=config.fillit(doc)) && (saveOk=config.save())){
			request->send(200, "text/html", restart_html);
			delay(1000);
			ESP.restart();
			return true; // should never return here but....
	}else {
		if(!fillOk)	
			request->send(200, "text/html", "<div class=error>Failed to fill config data!</div>");
		if(!saveOk)	
			request->send(200, "text/html", "<div class=error>Failed to save config data!</div>");
	}
	return false;
}

/** /func read post json data and fill the device instance
 */
bool devicesPost(uint8_t * data,AsyncWebServerRequest *request) {
	DeserializationError desError;
	uint16_t count;
	bool fillOk, saveOk;
	StaticJsonDocument<DEVICE_JSON_DOC_SIZE> jsonDoc;

	DBG("devicePost");
	if ( (desError=deserializeJson(jsonDoc, (const char*)data)) != DeserializationError::Ok ) {
		ERR("failed to deserialize config Data");
		request->send(200, "text/html", "<div class=error>Failed to parse config data!</div>");
		return false;
	}

	JsonArray postDev = jsonDoc["devices"].as<JsonArray>();
	if( (fillOk=devices.fill(postDev)) && (saveOk=devices.save()) ) {
		INFO("got %d devices from web UI \n", devices["devices"].size());
		request->send(200, "text/html", "<div class=responce>New devices saved </div>");
		return true;
	} else {
		ERR("%s failed \n",!fillOk?"fill":"save");
		if(!fillOk)	
			request->send(200, "text/html", "<div class=error>Failed to parse device data!</div>");
		if(!saveOk)	
			request->send(200, "text/html", "<div class=error>Failed to save device data!</div>");
	}
	return false;
}


void setupWebServer() {
	INFO("Setting up webserver\n");

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/index.html");
	}); 

	server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/style.css", "text/css");
	});

	server.on("/wsclient.js", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/wsclient.js", "application/javascript");
	});

	server.on("/scripts.js", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/scripts.js", "application/javascript");

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

	server.onRequestBody( [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
		uint16_t error=0;	
		if (request->method() == HTTP_POST) {
			if (request->url() == "/devices") {
				if(error=devicesPost(data,request))
					request->send(200,"update devices failed \n");
			} else if (request->url() == "/config"){
				if(error=configPost(data,request))
					request->send(200,"update config failed \n");
			}
				
		}
	});

	server.onNotFound(notFound);

	ws.onEvent(onWsEvent);
	server.addHandler(&ws);
	server.begin();

	INFO("Webserver & Webservice started.\n");
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
