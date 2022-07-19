#define ARDUINOJSON_ENABLE_COMMENTS 1
#include "Arduino.h"
#include <stdio.h>
#include <string>
#include <map>
#include "SPIFFS.h" 
#include <ArduinoJson.h> 
#include "Settings.hpp"
#include "Config.hpp"
#include "Devices.hpp"
#include "Util.hpp"

#include "dbgLevels.h"
//#define DEBUG_LEVEL DBG_L
#include "debug.h"

Config::Config(std::string cfgFileName): Settings(cfgFileName) {
	valid=false;
}

/* need to read the file into a char buffer
 * since trying to pass and JsonObject as am argument to fillit fails with
 * weird errors that im not smart enough to grasp
 */
bool Config::load() {
	bool result;

	StaticJsonDocument<CONFIG_DOC_SIZE> jdoc;
	DeserializationError desError;
	INFO("Loading config\n");

	if (! openFile()) {
		setStatus(ERROR_MSG, ALL, "%s - Failed to open config file %s", __PRETTY_FUNCTION__, fileName.c_str() );
		return false;
	}

	if ( (desError=deserializeJson(jdoc, file)) != DeserializationError::Ok ) {
		ERR("failed to deserialize config Data");
		return false;
	}
	result=fillit(jdoc);
	return result;
}

bool Config::fillit(JsonDocument & doc) {

	

	ssid = 					doc["ssid"].as<std::string>();
	password=				doc["password"].as<std::string>();
	hostname=				doc["hostname"].as<std::string>();
	room=					doc["room"].as<std::string>();
	ipaddress=				doc["ipaddress"].as<std::string>();
	apipaddress=			doc["apipaddress"].as<std::string>();
	netmask=				doc["netmask"].as<std::string>();
	mqttServer=				doc["mqttServer"].as<std::string>();
	mqttPort=				doc["mqttPort"].as<std::string>();
	mqttUser=				doc["mqttUser"].as<std::string>();
	mqttPassword=			doc["mqttPassword"].as<std::string>();
	mqttStateTopicPrefix=	doc["mqttStateTopicPrefix"].as<std::string>();
	mqttScanTopicPrefix=	doc["mqttScanTopicPrefix"].as<std::string>();
	ntpServer=				doc["ntpServer"].as<std::string>();
	gmtOffset=				doc["gmtOffser"].as<int>();
	daylightOffset=			doc["daylightOffset"].as<int>();
	debugLvl=				doc["debugLvl"].as<int>();

	// set default for some
	// "hostname" will be modified to "hostname_room" by startWifiClient 
	if(hostname.size()==0) hostname=DEFAULT_HOST_NAME;
	fullHostname=hostname+"_"+room;
	//
	// default gateway will be set to same ip address 
	if(apipaddress.size()==0) apipaddress=DEFAULT_IP_ADDRESS;
	if(netmask.size()==0) netmask=DEFAULT_NET_MASK;
	if(password.size()==0) password=DEFAULT_WIFI_PASSWORD;
	if(mqttUser.size()==0) mqttUser=DEFAULT_MQTT_USER;
	if(mqttPort.size()==0) mqttPort=DEFAULT_MQTT_PORT;


	closeFile();
	// check for valid wifi credentials
	if(  ssid.size() 
		&& password.size()
		&& hostname.size() 
		&& ssid.size() 
	){
		DBG("Web lig valid\n") ;
		webValid =true;
	} else {
		FAIL("missing ssid,hostname or password in config\n");
	}

	if( room.size()
		&& mqttServer.size()
		&& mqttPort.size()
		&& mqttUser.size()
		&& mqttPassword.size()
		&& mqttStateTopicPrefix.size()
		&& mqttScanTopicPrefix.size()
	  ) {
		DBG("Mqtt config valid\n") ;
		valid= webValid && mqttValid;
		mqttValid = true;
	} else {
		ERR("Missing mqttserver, mqttPort, mqttUser, mqttPassword, mqttStateTopicPrefix or mqttScanTopicPregix\n");
	}
	INFO("Config  Loaded \n");
	return true;
};

bool Config::save(){

	bool valid = false;
	if(!openFile("w")) {

		setStatus(ERROR_MSG,ALL,"%s - Cant Save config file, no file name found",__PRETTY_FUNCTION__);
		return false;
	}
	StaticJsonDocument<CONFIG_DOC_SIZE> doc;

	doc["room"]					= room;
	doc["ssid"]					= ssid;
	doc["password"]				= password;
	doc["hostname"]				= hostname;
	doc["mqttServer"]			= mqttServer;
	doc["mqttPort"]				= mqttPort;
	doc["mqttUser"]				= mqttUser;
	doc["mqttPassword"]			= mqttPassword;
	doc["mqttScanTopicPrefix"]	= mqttScanTopicPrefix;
	doc["mqttStateTopicPrefix"]	= mqttStateTopicPrefix;
	doc["ntpServer"] 			= ntpServer;
	doc["gmtOffser"]			= gmtOffset;
	doc["daylightOffset"]		= daylightOffset;			

	if(serializeJson(doc, file)==0) {
		setStatus(ERROR_MSG,ALL,"Failed to write settings to SPIFFS config file");
		return false;
	}
	return true;
}
