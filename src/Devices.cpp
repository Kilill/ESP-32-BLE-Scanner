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


bool Devices::load() {

StaticJsonDocument<DEVICE_JSON_DOC_SIZE> doc;

	valid=false;
	INFO("Loading devices\n");
	if (openFile()) {
			DeserializationError desError;
			VERBOSE("Deserializing json data\n");
			if((desError=deserializeJson(doc, file)) != DeserializationError::Ok ) {

				setStatus(ERROR_MSG,ALL,"Failed to deserialize device file:(%s). No devices configured",fileName.c_str());
#ifdef DEBUG
				printf("code: %d = ",desError);
				switch(desError.code()) {
					case DeserializationError::Ok:
						printf("OK\n");
					break;
					case DeserializationError::EmptyInput:
						printf("Empty Input\n");
					break;
					case DeserializationError::IncompleteInput:
						printf("Incomplete Input\n");
					break;
					case DeserializationError::InvalidInput:
						printf("Invalid Iput\n");
					break;
					case DeserializationError::NoMemory:
						printf("No Memory\n");
					break;
					case DeserializationError::TooDeep:
						printf("To Deep\n");
					break;
					default:
						printf("Uknown Deserialize error\n");
					break;
				}
#endif
				count=0;
				return false;
			}
	} else {
		ERR("failed to open %s file\n",fileName.c_str());
		return false;
	}
	return fill(doc["devices"].as<JsonArray>());
}


bool Devices::fill(const JsonArray& devices) {
	int i=0;	
	DBG("Filling device map\n");
	for (JsonObject jdevice : devices) {
		std::string uuid=jdevice["uuid"].as<std::string>();
		std::string name=jdevice["name"].as<std::string>();
		
		VERBOSE("#%d:\t %s: %s\n",i+1,uuid.c_str(),name.c_str());
		devList[uuid] = name;
		if(++i >= MAX_DEVICES) {
			setStatus(ERROR_MSG,ALL,"%s -  to many devices (%d) only %d will be defined",__PRETTY_FUNCTION__,count,MAX_DEVICES);
			count=MAX_DEVICES;
			break;
		}
		count=i;
	}
	DBG("Added %d devices from list\n",count);
	closeFile();
	valid=true;
	return true;
}

bool Devices::save(){

	bool success = true;
	openFile("w");
	StaticJsonDocument<DEVICE_JSON_DOC_SIZE> doc;
	JsonArray jdevices = doc.createNestedArray("devices");
	devMap_t::iterator dev;
	for(dev=devList.begin(); dev!= devList.end(); dev++) {
		JsonObject jdevice = jdevices.createNestedObject();
		jdevice["uuid"] = dev->first;
		jdevice["name"] = dev->second;
	}
	if(serializeJson(doc, file)==0) {
		setStatus(ERROR_MSG,ALL,"Failed to save devices to file");
		success = false;
	}
	closeFile();
	return success;
}
std::string Devices::operator[](std::string uuid) { return devList.find(uuid)->second; };
/** \func [] operator pair reference to n:th device
 */

devMap_t::iterator Devices::operator[](int index) {

	 devMap_t::iterator dev=devList.end();;
	 if(index >=0 || index<devList.size()) {
		 dev = devList.begin();
		 std::advance(dev,index);
	 }
	 return dev;
 }

/** \func set(key,val)
 * will either set the value of key or insert if not found, but only device count <MAX_DEVICES
 */

void Devices::set(std::string uuid,std::string name) {
	// sanity check that we are not exceding capacity in case were no "key" exist
	if((devList.find(uuid)==devList.end()) && devList.size() >= MAX_DEVICES) {
		ERR("Max number of devices reached , can't insert new\n");
		return;
	}
	devList[uuid]=name; 
};

