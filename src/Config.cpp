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
#include "Settings.hpp"
#include "Config.hpp"
#include "Devices.hpp"
#include "Util.hpp"

#include "dbgLevels.h"
#define DEBUG_LEVEL INFO_DBGL
#include "debug.h"

Config::Config(std::string cfgFileName): Settings(cfgFileName) {
	valid=false;
}

bool Config::load() {
	bool result;

	StaticJsonDocument<CONFIG_DOC_SIZE> jdoc;
	DeserializationError desError;
	INFO("Loading config\n");

	if (! openFile()) {
		return false;
	}

	if ( (desError=deserializeJson(jdoc, file)) != DeserializationError::Ok ) {
		setError(JsonDeserialize_E," to %s",fileName);
		return false;
	}
	result=fillit(jdoc.as<JsonVariant>());
	return result;
}

bool Config::fillit(JsonVariant jvar) {

	auto && conf=jvar.as<JsonObject>();
	DBG1("Config:fillit\n");

	ssid = 					conf["ssid"].as<std::string>();
	password=				conf["password"].as<std::string>();
	hostname=				conf["hostname"].as<std::string>();
	location=				conf["location"].as<std::string>();
	ipaddress=				conf["ipaddress"].as<std::string>();
	apipaddress=			conf["apipaddress"].as<std::string>();
	netmask=				conf["netmask"].as<std::string>();
	mqttServer=				conf["mqttServer"].as<std::string>();
	mqttPort=				conf["mqttPort"].as<std::string>();
	mqttUser=				conf["mqttUser"].as<std::string>();
	mqttPassword=			conf["mqttPassword"].as<std::string>();
	mqttStateTopicPrefix=	conf["mqttStateTopicPrefix"].as<std::string>();
	mqttScanTopicPrefix=	conf["mqttScanTopicPrefix"].as<std::string>();
	ntpServer=				conf["ntpServer"].as<std::string>();
	gmtOffset=				conf["gmtOffser"].as<int>();
	daylightOffset=			conf["daylightOffset"].as<int>();
	debugLvl=				conf["debugLvl"].as<int>();

	VERBOSE("ssid:                 %s\n", ssid.c_str() );
	VERBOSE("password:             %s\n", password.c_str());
	VERBOSE("hostname:             %s\n", hostname.c_str());
	VERBOSE("location:             %s\n", location.c_str());
	VERBOSE("ipaddress:            %s\n", ipaddress.c_str());
	VERBOSE("apipaddress:          %s\n", apipaddress.c_str());
	VERBOSE("netmask:              %s\n", netmask.c_str());
	VERBOSE("mqttServer:           %s\n", mqttServer.c_str());
	VERBOSE("mqttPort:             %s\n", mqttPort.c_str());
	VERBOSE("mqttUser:             %s\n", mqttUser.c_str());
	VERBOSE("mqttPassword:         %s\n", mqttPassword.c_str());
	VERBOSE("mqttStateTopicPrefix: %s\n", mqttStateTopicPrefix.c_str());
	VERBOSE("mqttScanTopicPrefix:  %s\n", mqttScanTopicPrefix.c_str());
	VERBOSE("ntpServer:            %s\n", ntpServer.c_str());
	VERBOSE("gmtOffset:            %d\n", gmtOffset);
	VERBOSE("daylightOffset:       %d\n", daylightOffset);

	// set default for some entrires that needs to have sane defaults
	// defaults are defined in Config.hpp
	// "hostname" will be modified to "hostname_location" by startWifiClient 
	error=OK_E;

	checkArg(hostname,		DEFAULT_HOST_NAME,		ConfigMissingHostName_E);
	checkArg(location,		DEFAULT_LOCATION,		ConfigMissingLocation_E);

	fullHostname = hostname+"_"+location;
	
	// default gateway will be set to same ip address 
	checkArg(apipaddress,	DEFAULT_IP_ADDRESS,		ConfigMissingAPIP_E);
	checkArg(netmask, 		DEFAULT_NET_MASK,		ConfigMissingAPNetmask_E);

	//TODO: validate ipaddresses and netmask for sanity.
	// std:regex ipMatch("\d{3}\.\d{3}\.\d{3}\.\d{3}");
	// if( std::regex_match(ipaddres,ipMatch) ...

	// check for valid wifi credentials
	checkArg(password,		DEFAULT_WIFI_PASSWORD,	ConfigMissingPassword_E);
	checkArg(ssid,			fullHostname,			ConfigMissingSSID_E);

	if(error == OK_E) {
		DBG2("Wifi config valid\n") ;
		webValid =true;
	}
	// Mqtt checks
	checkArg(mqttServer,			DEFAULT_UKNOWN,			ConfigMissingMqttServer_E);
	checkArg(mqttUser,				DEFAULT_MQTT_USER,		ConfigMissingMqttUser_E);
	checkArg(mqttPort,				DEFAULT_MQTT_PORT,		ConfigMissingMqttPort_E);
	checkArg(mqttPassword,			DEFAULT_EMPTY,			ConfigMissingMqttPassword_E);
	checkArg(mqttStateTopicPrefix,	DEFAULT_STATUS_TOPIC,	ConfigMissingStatusTopic_E);
	checkArg(mqttScanTopicPrefix,	DEFAULT_SCAN_TOPIC,		ConfigMissingScanTopic_E);

	if(error == OK_E) {
		DBG2("Mqtt config valid\n") ;
		mqttValid = true;
		valid = webValid && mqttValid;
		INFO("Config valid\n");
	} 
	valid= webValid && mqttValid;
	return valid;
};

bool Config::save(){

	bool valid = false;
	char buffer[CONFIG_DOC_SIZE];
	DBG2("Config:save\n");

	if(!openFile("w")) {
		return false;
	}

	StaticJsonDocument<CONFIG_DOC_SIZE> doc;

	doc["location"]				= location.c_str();
	doc["ssid"]					= ssid.c_str();
	doc["password"]				= password.c_str();
	doc["hostname"]				= hostname.c_str();
	doc["mqttServer"]			= mqttServer.c_str();
	doc["mqttPort"]				= mqttPort.c_str();
	doc["mqttUser"]				= mqttUser.c_str();
	doc["mqttPassword"]			= mqttPassword.c_str();
	doc["mqttScanTopicPrefix"]	= mqttScanTopicPrefix.c_str();
	doc["mqttStateTopicPrefix"]	= mqttStateTopicPrefix.c_str();
	doc["ntpServer"] 			= ntpServer.c_str();
	doc["gmtOffser"]			= gmtOffset;
	doc["daylightOffset"]		= daylightOffset;

	DBG3("Serialzing\n");

	if(serializeJson(doc, file)!=0) {
		VERBOSE("Config:save Config Json %s\n",buffer);
		valid=true;
	} else {
		setError(JsonSerialize_E," config to %s\n",fileName);
	}
	closeFile();
	return valid;
}
// check if tags is not null, and set to default if so
uint16_t Config::checkArg(std::string& arg, const std::string& argDefault, errorT errorCode) {

	if(arg.size() == 0) {
		arg=argDefault;
		setError(errorCode," Setting to default %s",argDefault.c_str());

		return error;
	} else if(arg.size() > MAX_ARG_SIZE) {
		setError(ConfigArgLength_E," truncating to %d",argDefault.c_str());
		arg.erase(MAX_ARG_SIZE,std::string::npos);
		return error;
	}
	return OK_E;
}
