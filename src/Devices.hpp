#ifndef __devices_h__
#define __devices_h__
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

/*! \def 
 *  typdef for the the std c++ map container , build on "std::pair" ie associative array that holds
 *  all device uuid and name pairs
 */
typedef std::map<std::string,std::string> devMap_t ;


const std::string DEFAULT_DEVICE_FILENAME="/devices.json";	//< Default Filenam if none set in Config

/*! \def MAX_DEVICES
 * max numer of devices 
 */
#define MAX_DEVICES 10

/**
  Size calculations for json doc size
  ===================================
  This affects the sized of the Json documents used to parse the device file
 
 The complete computation for size can be found at https://arduinojson.org/v6/assistant
 based on the following json format:
 
	{ "devices":
		[
    		"uuid": "12345678-1234-1234-1234-1234567890ab", "name": "The name of the device" 
			....
		]
	}

 observ that extracting the UUID as from the bluetooth beacon data pretty prints it in the above format
 so this gives the length of an UUID as 32 hex characters 4 hyphens + terminating 0 char == 37 bytes
 then we have to add the length of the name +1

 The internal structure is 16+48 == 64 bytes each  subsequent entry adds 48 bytes 

 The space for the tags is only added once and the sum of the lengths of each tag +1 for 0 char

 		tags space=len("devices")+1+len("uuid")+1+len("name")+1 = 4+1+4+1+7+1 = 5+5+8 = 18 

 so the first entry takes up (16+48)+(18+UUID_LEN+1+NAME_LEN+1)i = 64+86 = 150 bytes 
 subsequent entries adds 48 + 68 = 116 bytes
 so formula is (16+(48*MAX_DEVICES) + (18+((UUID_LEN + MAX_NAME_LEN +2) * MAX_DEVICES) )

	total_minimun = 1194

 Jason Assitant recomends adding some slack and computes that as:

 	val = total_minmun+max( 10, stringspace*0.1
 	factor = pow( 2, max( 3, ceil( log2( val ))-2)))
 	final = ceil(val/factor)*factor
	=1535

 which turns out to be around  (total minimun *1.3) & 0xff00
 ie make space for 3 more entries
 That seems to be overkill since we know exactly how many devices we allow
 and so fmanous last words (will it come back and bite me ?) we just ad 1 more 
*/
#define SLACK 1
#define UUID_LEN 36
#define MAX_NAME_LEN 30
#define TAG_LEN 18
#define STRUCT_LEN (16+(48*(MAX_DEVICES+SLACK)))
#define STR_LEN (TAG_LEN+(UUID_LEN+MAX_NAME_LEN+2)*(MAX_DEVICES+SLACK))

#define DEVICE_JSON_DOC_SIZE (STRUCT_LEN+STR_LEN)

//TODO: should this be made into a singelton ?
/*!
 * @brief class Devices, handle device information
 *
 * keep the uuid and name of the devices in an associative list
*/

class Devices : public Settings
{
private:
	devMap_t devList;			///< list of known devices 

public:
	int16_t	count;				///< current count of devices
	bool valid;					///< is list valid

	 
	/*! Constructor 
	 *
	 * @param fileName [IN} SPIFFS name of json SPIFFS file that hold device info
	 */
	Devices(std::string fileName=DEFAULT_DEVICE_FILENAME): Settings(fileName){
		valid=false;
	};

	/*! [std::string] operator
	 *
	 * to retreive device info baed on uuio
	 * like: name=devices['some-uuid-here']
	 *
	 * @param uuid of entry as std::string
	 * @return name std::string
	 */
	std::string operator[](std::string uuid);

	/*! [int] operator
	 *
	 * returms an std::iterator that point to the device at the index
	 */
	devMap_t::iterator operator[](int index); 

	
	inline devMap_t::iterator begin(){ return devList.begin(); };	///< \fn returns an iterator to the begining of the dev list
	inline devMap_t::iterator end(){ return devList.end(); };		///< \fn returns an iterator to the end of the dev list

	/*! set or add device to list
	 *
	 * sets the name of a device with given uuid
	 * if no device exists it is added
	 *
	 * @param uuid [IN] std::string uuid of device to set name for
	 * @param name [IN] std::string name of device
	 */
	void set(std::string uuid,std::string name);

	/*! erase device from list
	 *
	 * erases the device with given uuid from list
	 */
	inline void erase(std::string uuid) { devList.erase(uuid); };

	/* check if device is in list
	 *
	 * goes through list and sees if the device is known
	 *
	 * @param uuid [IN] std::string uuid of device to check for
	 * @return true/false [OUT] bool 
	 */
	bool exists(std::string uuid) { return devList.find(uuid)!=devList.end();};

	/*! load devices from SPIFFS json file
	 *
	 * loads devices from the file indicated by the fileName constructor argument
	 * see comment above about size and format
	 *
	 * @return true/fals [OUT] bool returns true if load was successfull otherwise returns false
	 */
	bool load();

	/*! fill  up the map from a json array 
	 *
	 * @param devices [IN] jsonArray containing uuid,name pairs to load
	 * @return true/fals [OUT] returns true if save succeeds, false otherwise
	 */
	bool fill(const JsonArray& devices);

	/*! save map to json file
	 *
	 * save device map the the file idicated byt the fileName constructor argument
	 *
	 * @return true/fals [OUT] returns true if save succeeds, false otherwise
	 */
	bool save();

};

#endif
