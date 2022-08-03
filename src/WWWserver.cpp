/*
 ESP32 BLE Beacon scanner

  Copyright (c) 2022 Kim Lilliestierna. All rights reserved.
  https://github.com/Kilill/ESP-32-BLE-Scanner

  based on code from:
  Copyright (c) 2021 realjax (https://github.com/realjax). All rights reserved.
  https://github.com/realjax/ESP-32-BLE-Scanner
  
  Copyright (c) 2020 (https://github.com/HeimdallMidgard) All rights reserved.
  https://github.com/HeimdallMidgard/ESP-32-BLE-Scanner


  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library in the LICENSE file. If not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  or via https://www.gnu.org/licenses/gpl-3.0.en.html

*/

#define ARDUINOJSON_ENABLE_COMMENTS 1
#include "Arduino.h"
#include <stdio.h>
#include <string>
#include <map>
#include "SPIFFS.h" 
#include <AsyncJson.h> 
#include <ArduinoJson.h> 
#include "ESPAsyncWebServer.h"

#include "Util.hpp"
#include "Devices.hpp"
#include "Config.hpp"

#include "dbgLevels.h"
//#define DEBUG_LEVEL VERBOSE_DBGL
#include "debug.h"

// TODO: what about user password access to server 
// and /or dont start server unless button pushed ....

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

AsyncWebSocketClient* wsClient;

void notFound(AsyncWebServerRequest *request);
void write_to_logs(const char* new_log_entry);

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);


void notFound(AsyncWebServerRequest *request) {	
	char * method;

#if VERBOSE_L

	switch (request->method()) {
		case  HTTP_GET:
			method=(char *)"GET";
			break;
		case  HTTP_POST:
			method=(char *)"POST";
			break;
		case  HTTP_DELETE:
			method=(char *)"DELETE";
			break;
		case  HTTP_PUT:
			method=(char *)"PUT";

		case  HTTP_PATCH:
			method=(char *)"PATCH";
			break;
		case  HTTP_HEAD:
			method=(char *)"HEAD";
			break;
		case  HTTP_OPTIONS:
			method=(char *)"OPTIONS";
			break;
		default :
			method=(char *)"UNKNOWN";
		break;
  }
  printf("NOT FOUND:  %s, http://%s%s\n", method, request->host().c_str(), request->url().c_str());

  if(request->contentLength()){
    printf("ContentType: %s. ContentLength: %u\n", request->contentType().c_str() , request->contentLength());
  }

  int headers = request->headers();
  int i;
  for(i=0; i<headers; i++) {
    AsyncWebHeader* h = request->getHeader(i);
    printf("HEADER[%d]: %s=%s\n", i,h->name().c_str(), h->value().c_str());
  }

  int params = request->params();
	  for(i = 0; i < params; i++) {
		AsyncWebParameter* p = request->getParam(i);
		if(p->isFile()){
		  printf("FILE %s= %s, size: %u", p->name().c_str(), p->value().c_str(), p->size());
		} else if(p->isPost()){
		  printf("POST %s = %s", p->name().c_str(), p->value().c_str());
		} else {
		  printf("GET %s = %s", p->name().c_str(), p->value().c_str());
		}
	  }


    char * reason;
	asprintf(&reason, "{404) Whoot !? i have no clue what that is.\n %s, http://%s%s\n", 
			method, request->host().c_str(), request->url().c_str());
	request->send(404, "text/plain", reason);
	free(reason);
#else
	request->send(404, "text/plain","{404) Whoot !? i have no clue what that is.\n");
#endif
}
// ---------------------------------------------------------------------------

void setupWebServer() {
	INFO("Setting up webserver\n");

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		DBG2("serving: index.html\n");
		request->send(SPIFFS, "/index.html");
	}); 

	server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
		DBG2("serving: style.css\n");
		request->send(SPIFFS, "/style.css", "text/css");
	});
	server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
		DBG2("serving: config.html\n");
		request->send(SPIFFS, "/config.html");
	});

	server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest *request){
		DBG2("serving: config.json\n");
		request->send(SPIFFS, "/config.json","application/json");
	});

	server.on("/devices", HTTP_GET, [](AsyncWebServerRequest *request){
		DBG2("serving: devices.html\n");
		request->send(SPIFFS, "/devices.html");
	});

	server.on("/devices.json", HTTP_GET, [](AsyncWebServerRequest *request){
		DBG2("serving: devices.json\n");
		request->send(SPIFFS, "/devices.json","application/json" );
	});

	server.on("/jquery-3.6.0.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
		DBG2("serving: jquery\n");
		request->send(SPIFFS, "/jquery-3.6.0.min.js","text/javascript" );
	});
	server.on("/bin.png", HTTP_GET, [](AsyncWebServerRequest *request){
		DBG2("serving: bin.png\n");
		request->send(SPIFFS, "/bin.png","image/png" );
	});
	server.on("favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
		DBG2("serving: jquery\n");
		request->send(SPIFFS, "/bin.png","image/png" );
	});

	// ---- config post handler

    AsyncCallbackJsonWebHandler* cfgPostHandler=(new AsyncCallbackJsonWebHandler("/config", [](AsyncWebServerRequest *request, JsonVariant &jConf) {
				DBG1("config post request\n");

				if(!jConf.is<JsonObject>()) {
					ERR("Config posthandler: Not an json object\n");
					request->send(400,"text/plain","config received is not json ");
					return;
				}
#if VERBOSE_L
				char  tmp[1024];
				auto && jObj=jConf.as<JsonObject>();
				serializeJson(jObj,tmp,1023);
	  			VERBOSE("conf Post handler Json:\n%s\n",tmp);
#endif
				
				// fill up class and save
				
				if(! config.fillit(jConf)) {	
					ERR("Failed to fill config\n");
					request->send(200, "text/html", "<div class=error>Failed to fill config data!</div>");
					return ;
				}
				if(!config.save())	{
					ERR("Failed to save config\n");
					request->send(200, "text/html", "<div class=error>Failed to save config data!</div>");
					return ;
				}
				request->send(200, "text/html", "<div class=success>Config saved successfully</div>");
				INFO("Config filled and saved. Redirecting to Restart\n");
				return;
	}));	

	cfgPostHandler->setMethod(HTTP_POST);
	server.addHandler(cfgPostHandler);

	// ---- devices post handler

    AsyncCallbackJsonWebHandler* devicesPostHandler=(new AsyncCallbackJsonWebHandler("/devices", [](AsyncWebServerRequest *request, JsonVariant &jData) {
				DBG1("config post request\n");
				if(!jData.is<JsonObject>()) {
					ERR("Devices posthandler: Not an json object\n");
					request->send(400,"text/plain","Devices received is not json ");
					return;
				}
#if VERBOSE_L
				char  tmp[1024];
				auto && jObj=jData.as<JsonObject>();
				serializeJson(jObj,tmp,1023);
	  			VERBOSE("Devices Post handler Json:\n%s\n",tmp);
#endif
				
				JsonArray jDev = jData["devices"].as<JsonArray>();


				if(!devices.fill(jDev) || !devices.save()){
					char * msgP;
					ERR("DevicesPost update failed %s\n",devices.errorS);
					asprintf(&msgP,"<div class=error>Failed to update device list!:<br>%s</div>",devices.errorS);
					request->send(512, "text/html", msgP);
					free(msgP);
					return ;
				}
				INFO("DevicesPost got %d devices from web UI \n", devices.count);
				request->send(200, "text/html", "<div class=okresponce>Device list updatead and saved </div>");
				return ;
	}));

	devicesPostHandler->setMethod(HTTP_POST);
	server.addHandler(devicesPostHandler);


	server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/restart.html");
		INFO("Restart Requested");
		triggerReset();  // trigger a restart in 5 seconds ned to wait for client actually having loaded the restart page
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
		sendWsText(INFO_MSG,"Connected to server");
	} else if(type == WS_EVT_DISCONNECT){
		wsClient = nullptr;
	}
}

void sendWsText(MsgType type,const char *message) {
 char * msgp;
 char * levelp;

	if (wsClient != nullptr && wsClient->canSend()) {
		if(type==DEVICE_MSG) {
			asprintf(&msgp,"{ \"type\": \"%s\", \"data\": %s}", MsgTypeTxt[type], message );
		} else {
			asprintf(&msgp,"{ \"type\": \"%s\", \"data\": \"%s\"}", MsgTypeTxt[type], message );
		}
		wsClient->text(msgp);
		VERBOSE("WsText sent  message %s\n",msgp);
		free(msgp);
	}
}
