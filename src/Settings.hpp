#ifndef __settings_h__
#define __settings_h__
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

#include <stdio.h>
#include <string>
#include <SPIFFS.h>

/*! handle commonalitues for Devices and Config
 *
 * handles file operations and keeps track on the filesystem beeing valid and initialzed
 */


class Settings {
		

public:

	bool fsValid;				///< is the file system in a valid state
	std::string fileName;		///< file name on SPIFFS system
	File file;					///< file descriptor for above
	const char * errorS;				///< latest error string
	uint16_t error;				///< latest error code

	const uint16_t MAX_ARG_SIZE=30;

	/*! error codes
	 *
	 * if you add a new one remember to add to  the private string array at the end
	 * or bad things will happen
	 */
	enum errorT : uint16_t {

	/* 0*/	OK_E,
	/* 1*/	OutOfMem_E,

	/* 2*/	FileName_E,
	/* 3*/	SPIFFS_E,
	/* 4*/	FileOpen_E,
	/* 5*/	FileWrite_E,

	/* 6*/	JsonFormatInvalid_E,
	/* 7*/	JsonSerialize_E,
	/* 8*/	JsonDeserialize_E,

	/* 9*/	DeviceListInvalid_E,
	/*10*/	UUIDInvalid_E,
	/*11*/	UUIDLength_E,
	/*12*/	DeviceNameLenght_E,
	/*13*/	DeviceCount_E,
	
	/*14*/	ConfigInvalid_E,
	/*15*/	ConfigSize_E,
	/*16*/	ConfigTagInvalid_E,
	/*17*/	ConfigArgLength_E,
	/*18*/	ConfigMissingSSID_E,
	/*19*/	ConfigMissingPassword_E,
	/*20*/	ConfigMissingHostName_E,

	/*21*/	ConfigMissingAPIP_E,
	/*22*/	ConfigMissingAPNetmask_E,

	/*23*/	ConfigMissingLocation_E,
	/*24*/	ConfigMissingMqttServer_E,
	/*25*/	ConfigMissingMqttUser_E,
	/*26*/	ConfigMissingMqttPassword_E,
	/*27*/	ConfigMissingMqttPort_E,
	/*28*/	ConfigMissingStatusTopic_E,
	/*29*/	ConfigMissingScanTopic_E,
	
	/*30*/	LastErrorConstat
	};

	/*! constructor
	 *
	 * checks that the SPIFFS system has been started and is in a good state
		 * sets fsValid 
		 *
	 * @param fileName_arg [IN] std::string name of file
	 */
	Settings(std::string fileName_arg) {
		fileName=fileName_arg;

		if (SPIFFS.begin()) {
			fsValid=true;
		}
		error=OK_E;
		errorS=nullptr;
	}

	//Todo this should probly be broken out into its own "Status class"
	/*! sett and print error 
	 *
	 * @param errorCode [in] error type indexs into errorStrings
	 * @param format [in] printf type format string
	 * @param ...	[in] eventual printf parameters
	 */
	void setError(errorT errorCode,const char * format,...);

	/*! open the file in SPIFFs for reading	
	 *
	 * open the file for reading
	 * set file to the file descriptor
	 *
	 * @return true/false [OUT] bool returns true if open succeeded false otherwise
	 */
	bool openFile() {return openFile("r");}

	/*! open the file in SPIFFs 
	 *
	 * open the file using mode
	 * set file to the file descriptor
	 *
	 * @param mode [IN] const char * to mode string 
	 *
	 * @return true/false [OUT] bool returns true if open succeeded false otherwise
	 */
	bool openFile(const char* mode);

	/*! close file
	 */
	void closeFile();

	/*! virtual Load content
	 */
	virtual bool load()=0;
	/*! virtual Save map to file */
	virtual bool save()=0;

private:

	/*! error strings setError sets errorS to one of these
	 * 
	 * if Your add anny here dont forget to add to the enum as well
	 */
	const char * const errorStrings [LastErrorConstat] ={
	/* 0*/	"OK",
	/* 1*/	"Out of memory",

	/* 2*/	"Missing File Name",
	/* 3*/	"SPIFF mount failed",
	/* 4*/	"Failed to open file",
	/* 5*/	"Failed to write to file",

	/* 6*/	"JSON format Invalid ",
	/* 7*/	"JSON Serialize Failed",
	/* 8*/	"JSON Deserialize Failed",

	/* 9*/	"Device List Invalid",
	/*10*/	"UUID format Invalid ",
	/*11*/	"UUID to long",
	/*12*/	"Device Name to long",
	/*13*/	"Devices count exceeds limit",

	/*14*/	"Config Invalid",
	/*15*/	"Config to large",
	/*16*/	"Config tag uknown",
	/*17*/	"Config argument to long",
	/*18*/	"Missing SSID",
	/*19*/	"Missing password",
	/*20*/	"Missing hostname",

	/*21*/	"Invalid/Missing AP IP address",
	/*22*/	"Invalid/Missing AP Netmask address",

	/*23*/	"Missing location name",
	/*24*/	"Missing Mqtt Server",
	/*25*/	"Missing Mqtt User",
	/*26*/	"Missing Mqtt Password",
	/*27*/	"Missing Mqtt Port",
	/*28*/	"Missing Mqtt Status Topic",
	/*29*/	"Missing Mqtt Scan Topic"
	};
};

#endif
