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
//#define DEBUG_LEVEL VERBOSE_DBGL
#include "debug.h"


bool Devices::load() {

StaticJsonDocument<DEVICE_JSON_DOC_SIZE> doc;

	valid=false;
	INFO("Loading devices\n");
	if (openFile()) {
			DeserializationError desError;
			VERBOSE("Deserializing json data\n");
			if((desError=deserializeJson(doc, file)) != DeserializationError::Ok ) {
#if WERBOSE_L 
				const char * msg;
				switch(desError.code()) {
					case DeserializationError::Ok:
						msg="OK\n";
					break;
					case DeserializationError::EmptyInput:
						msg="Empty Input\n";
					break;
					case DeserializationError::IncompleteInput:
						msg="Incomplete Input\n";
					break;
					case DeserializationError::InvalidInput:
						msg="Invalid Iput\n";
					break;
					case DeserializationError::NoMemory:
						msg="No Memory\n";
					break;
					case DeserializationError::TooDeep:
						msg="To Deep\n";
					break;
					default:
						msg="Uknown Deserialize error\n";
					break;
				}
				setError(JsonDeserialize_E, "Failed to deserialize device file:(%s). Reason: %s [%d]. No devices configured",fileName.c_str(),msg,desError);
#else
				setError(JsonDeserialize_E, "Failed to deserialize device file:(%s). Reason: %d. No devices configured",fileName.c_str(),desError);
#endif
				count=0;
				return false;
			}
	} else {
		setError(FileOpen_E,"%s ",fileName.c_str());
		return false;
	}
	return fill(doc["devices"].as<JsonArray>());
}


bool Devices::fill(const JsonArray& devices) {
	int i=0;	
	DBG1("Devices.fill\n");
	// erase previous devices first 
	devList.clear();
	for (JsonObject jdevice : devices) {
		std::string uuid=jdevice["uuid"].as<std::string>();
		std::string name=jdevice["name"].as<std::string>();

		devList[uuid] = name;
		DBG3("#%d:\t %s: %s\n",i+1,uuid.c_str(),devList[uuid].c_str());

		if(++i >= MAX_DEVICES) {
			setError(DeviceCount_E,"(%d) only %d will be defined",count,MAX_DEVICES);
			count=MAX_DEVICES;
			break;
		}
		count=i;
	}

	DBG3("Added %d devices from list\n",count);
	valid=true;
	return true;
}

//-----------------------------------------------------------------------------

bool Devices::save(){

	bool success = true;
	DBG1("Device:save\n");

	StaticJsonDocument<DEVICE_JSON_DOC_SIZE> doc;

	JsonArray jdevices = doc.createNestedArray("devices");
	
	// get iterrator on device map and loop trhough all adding them to json doc
	devMap_t::iterator dev;

	for(dev=devList.begin(); dev!= devList.end(); dev++) {
		JsonObject jdevice = jdevices.createNestedObject();

		DBG3("Adding %s:%s\n",dev->first.c_str(),dev->second.c_str());

		jdevice["uuid"] = dev->first;
		jdevice["name"] = dev->second;
	}

	if(!openFile("w")) {
		return false;
	} 

	DBG3("serializing to file\n");

	if(serializeJson(doc, file)==0) {
		setError(JsonSerialize_E,"to %s",fileName);
		success = false;
	}
	closeFile();
	return success;
}
// get name of device "uuid"
std::string Devices::operator[](std::string uuid) { return devList.find(uuid)->second; };

// return iterator of indexed device
devMap_t::iterator Devices::operator[](int index) {

	 devMap_t::iterator dev=devList.end();;
	 if(index >=0 || index<devList.size()) {
		 dev = devList.begin();
		 std::advance(dev,index);
	 }
	 return dev;
 }

// will either set the value of key or insert if not found, but only device count <MAX_DEVICES

void Devices::set(std::string uuid,std::string name) {
	// sanity check that we are not exceding capacity in case were no "key" exist
	if((devList.find(uuid)==devList.end()) && devList.size() >= MAX_DEVICES) {
		setError(DeviceCount_E," can't insert new %s = %s",uuid,name);
		return;
	}
	devList[uuid]=name; 
};

