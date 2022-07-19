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
#define DEBUG_LEVEL INFO_L
#include "debug.h"

WiFiClient espClient;

bool wifiAccesPointModeEnabled = false;

void setupAPmode(){
	DBG("setupAPmode\n");
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
	boolean LWB2 = WiFi.softAPConfig(ipaddress, ipaddress, IPAddress(255, 255, 255, 0));	

	if ( !LWB2 ) { 
			FAIL("FAIL: softAPConfig failed with SSID: %s", config.hostname.c_str());	
	}
	WiFi.mode(WIFI_AP);
	delay(1000);	
	WiFi.persistent(false); 
	boolean LWB1 = WiFi.softAP(config.hostname.c_str());
	if ( LWB1 ) {  
			wifiAccesPointModeEnabled = true;
			INFO("Connect to the WIFI hotspot: \"%s\"\nThen connect your browser to http://%s\n",
				config.hostname.c_str(),WiFi.softAPIP().toString().c_str());				
	}else{	
			ERR("ERROR Start AccessPoint failed!\n");  
	}  
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

	INFO("\nWiFi connected\nConnect your browser to http://%s\n",WiFi.localIP().toString().c_str());
	
	ts[0]=0;
	DBG("Checking for ntp server setup\n");
	if(config.ntpServer.size()) {
		INFO("Setting up ntpsserver: %s\n",config.ntpServer.c_str());
		configTime(config.gmtOffset, config.daylightOffset, config.ntpServer.c_str());	

		while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        	DBG("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
			vTaskDelay(2000 / portTICK_PERIOD_MS);
    	}
        time(&stamp);
        now=localtime(&stamp);
        if (now != nullptr)  {
            strftime(ts,30,"%d/%m/%Y %R",now);        
			DBG("time set to: %s\n",ts);
		}
        else
			WARN( "Failed to set time \n");
	}
}

void setupWIFI(){
	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
	WiFi.persistent(false);

	INFO("Setting up WIFI access.\n");
	if (!config.ssid.size()){
		 WARN("No wifi credentials found, setting up AP mode.\n");
		 setupAPmode();		
	}else{
		 INFO("Setting up WIFI Client mode.\n");
		 startWifiClient();
	}
}


