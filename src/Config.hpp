#ifndef __config_h__
#define __config_h__
#include "Arduino.h"
#include <stdio.h>
#include <string>
#include "SPIFFS.h" 
#include <ArduinoJson.h> 
#include "Settings.hpp"

const std::string DEFAULT_CONFIG_FILE_NAME="/config.json";
const std::string DEFAULT_IP_ADDRESS="192.168.4.1";
const std::string DEFAULT_NET_MASK="192.168.4.1";
const std::string DEFAULT_HOST_NAME="Beacon_Scanner";
const std::string DEFAULT_WIFI_PASSWORD="Beacon_Scanner";
const std::string DEFAULT_MQTT_USER="BLEScanner32";
const std::string DEFAULT_MQTT_PORT="1883";

//size of json document recomended by https://arduinojson.org/v6/assistant/#/step1
#define JSON_DOC_SIZE 768

class Config : public Settings
{
public:
	std::string ssid;					// ssid to connect to
	std::string password;				// wifi password used for both ap and station
	std::string hostname;				// base hostname config will add room 
	std::string room;					// room id/name we are reporting from
	std::string fullHostname;			// filled in load() from hostname+"_"+room
	std::string ipaddress;				// ip adress of wifi in client mode, blank for DHCP
	std::string apipaddress;			// ip adress of wifi in AP mode
	std::string netmask;				//  netmask to use in AP mode
	std::string mqttServer;				// mqttserver to send reports to
	std::string mqttPort;				// port
	std::string mqttUser;				// User used to connect the mqttserver
	std::string mqttPassword;			// Password
	std::string mqttScanTopicPrefix;	// what base topic to send scan reports to, room will be appended
	std::string mqttStateTopicPrefix;	// where to report state changes room will be apended
	std::string fileName;				// filename of config gile
	// time
	std::string ntpServer; 				// time serve
	int gmtOffset;						// Local offser from gmt in seconds
	int daylightOffset;					// Daylight saving time in seconds
	int16_t debugLvl;					// Default debug level at start up see debug.h for explanatiion
	bool mqttValid;						// is mqtt config valid
	bool webValid;						// is web and Wifi valid
	bool valid;							// is config valid
	
	Config(std::string cfgFileName=DEFAULT_CONFIG_FILE_NAME);

	bool load();
	bool save();
};

#endif
