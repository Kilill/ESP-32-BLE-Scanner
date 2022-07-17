#ifndef __devices_h__
#define __devices_h__
#include <string>
#include <map>
#include "Settings.hpp"


// std c++ map container , build on "std::pair" ie associative array
typedef std::map<std::string,std::string> devMap_t ;

// Default Filenam if none set in Config
const std::string DEFAULT_DEVICE_FILENAME="/devices.json";

// max numer of devic/es 
// This affects the sized of the Jons documents used to parse the device file
// currently set to 10
/* 
 The complete computation for size can be found at https://arduinojson.org/v6/assistan
 based on the following json format:
 
	{ "devices":
		[
    		"uuid": "12345678-1234-1234-1234-1234567890ab", "name": "The name of the device" 
			....
		]
	}
	observ that extracting the UUID as a from the beacon data pretty prints it in the above format
	so this gives the length of an UUID as 32 hex characters 4 hyphens and the terminating 0 char == 37 bytes
	then we have to add the length of the name +1

	The internal structure is 16+48 bytes and an aditional 48 bytes for subsequen entries

	The space for the tags is only added once and the sum of the lengths of each tag +1 for 0 char

 		tags space=len("devices")+1+len("uuid")+1+len("name")+1 = 4+1+4+1+7+1 = 5+5+8=18 

	so the first entry takes up (16+48)+(18+UUID_LEN+1+NAME_LEN+1)i <=> 64+86 =150 bytes 

	subsequent entries adds 48 + 68 = 116 bytes

	so formula is (16+(48*MAX_DEVICES) + (18+((UUID_LEN+MAX_NAME_LEN+2)*MAX_DEVICES))

	total minimun = 1194
 	Jason Assitant recomends adding some slack and computes that as:

 	val total minmun+max( 10, stringspace*0.1
 	factor=pow( 2, max( 3, ceil( log2( val ))-2)))
 	final=ceil(val/factor)*factor
	=1535
	which turns out to be around  (total minimun *1.3) & 0xff00
	ie make space for 3 more entries
	wich is probably never going to be needed...
	so for now dont add any slack
*/


#define MAX_DEVICES 10
#define UUID_LEN 36
#define MAX_NAME_LEN 30
#define TAG_LEN 18
#define STRUCT_LEN (16+(48*MAX_DEVICES))
#define STR_LEN TAG_LEN+(UUID_LEN+MAX_NAME_LEN+2)*MAX_DEVICES

#define DEVICE_JSON_DOC_SIZE  STRUCT_LEN+STR_LEN

// map of known devices that we shall report presence on

class Devices : public Settings
{
private:
	devMap_t devList;

public:
	int16_t	count;
	bool valid;
	

	// let Settings handle the file intricacies
	Devices(std::string fileNameArg=DEFAULT_DEVICE_FILENAME): Settings(fileNameArg){
		valid=false;
	};

	std::string operator[](std::string uuid);

	devMap_t::iterator operator[](int index); 

	inline devMap_t::iterator begin(){ return devList.begin(); };
	inline devMap_t::iterator end(){ return devList.end(); };

	void set(std::string uuid,std::string name);
	inline void erase(std::string uuid) { devList.erase(uuid); };
	bool exists(std::string uuid) { return devList.find(uuid)!=devList.end();};

	bool load();
	bool save();

};

#endif
