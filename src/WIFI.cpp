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
#include <ArduinoJson.h> 
#include <WiFi.h>
#include <esp_sntp.h>
#include "Settings.hpp"
#include "Config.hpp"
#include "Devices.hpp"
#include "Util.hpp"


#include "dbgLevels.h"
#define DEBUG_LEVEL VERBOSE_DBGL
#include "debug.h"

WiFiClient espClient;

bool wifiAccesPointModeEnabled = false;

void setupAPmode(){
	WARN("setting up wifi in APmode\n");
	WiFi.disconnect(true);						 
	WiFi.mode(WIFI_OFF);							
	delay(1000);											 
	WiFi.persistent(false);  
	IPAddress ipaddress;
	IPAddress netmask;
	ipaddress.fromString(config.apipaddress.c_str());
	netmask.fromString(config.netmask.c_str());;

	// gw ip address does not make sense when in ap mode since we are not connected to anny
	// network	so setting it to itself;
	//                      ip        gateway     netmask
	if ( WiFi.softAPConfig(ipaddress, ipaddress, IPAddress(255, 255, 255, 0))) { 
		WiFi.mode(WIFI_AP);
		delay(1000);	
		WiFi.persistent(false); 

		if ( !WiFi.softAP(config.hostname.c_str(),config.password.c_str() )) {  
			FAIL("FAIL: Cant setup AP mode at SSID: %s\n Rebooting in 10 sec", config.hostname.c_str() );	
			for (int i=10;i>0;i-- ){
				printf("%d\n");
				delay(0000);
			}
			printf("GoodBye, and thanks for all the fish\n");
			ESP.restart();
			}
	}

	wifiAccesPointModeEnabled = true;
	INFO("AP up, Connect to the WIFI hotspot: \"%s\"\nThen connect your browser to http://%s\n , password: %s",
		config.hostname.c_str(),WiFi.softAPIP().toString().c_str(), config.password.c_str());				
}


void startWifiClient(){
	// First shut down AP
	time_t stamp;
	struct tm *now;
	char ts[30]; 
	const int16_t retry_count = 15;
	int16_t retry=0;
	WiFi.enableAP(false);
	WiFi.softAPdisconnect(true);
	// Set it to auto connect and reconnect
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);
	// We start by connecting to a WiFi network
	INFO("WIFI Connecting to %s\n",config.ssid.c_str());

	if (WiFi.status() != WL_CONNECTED) { 
		WiFi.begin(config.ssid.c_str(), config.password.c_str()); 
		WiFi.hostname(config.fullHostname.c_str());
	}

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		INFO(".");
	}

	
	ts[0]=0;
	if(config.ntpServer.size()) {
		INFO("Connecting to ntpserver: %s\n",config.ntpServer.c_str());
		configTime(config.gmtOffset, config.daylightOffset, config.ntpServer.c_str());	

		while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        	DBG2("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
			vTaskDelay(2000 / portTICK_PERIOD_MS);
    	}
        time(&stamp);
        now=localtime(&stamp);
        if (now != nullptr)  {
            strftime(ts,30,"%d/%m/%Y %R",now);        
			INFO("time set to: %s\n",ts);
		}
        else
			WARN( "Failed to set time \n");
	} else {
		DBG1("No ntp server sett in config\n");
	}

	INFO("\nWiFi connected\nConnect your browser to http://%s\n",WiFi.localIP().toString().c_str());
}

void setupWIFI(){
	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
	WiFi.persistent(false);

	INFO("Setting up WIFI\n");
	DBG2("SSID=%s (%d)\n",config.ssid.c_str(),config.ssid.size());
	if (!config.ssid.size()){
		 WARN("No wifi credentials found in config, setting up AP mode.\n");
		 setupAPmode();		
	}else{
		 INFO("Starting WIFI Client\n");
		 startWifiClient();
	}
}


