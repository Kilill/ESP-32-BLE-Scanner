#include <stdio.h>
#include <Settings.hpp>
#include <SPIFFS.h>
#include "Util.hpp"
#include "dbgLevels.h"
//#define DEBUG_LEVEL DBG_L
#include "debug.h"
/* base class for all other settings like Config and Devices
 *
 */

bool Settings::openFile(const char * mode){
	if(!fsValid){
		if(!SPIFFS.begin()) {	// should have been done by the constructor lets try again
			setStatus(ERROR_MSG,PRINT,"ERROR occurred during SPIFFS mounting");
			return false;
		} else fsValid = true;
	}

	DBG("Trying to open %s \n",fileName.c_str());
	if(fileName.size() ==0)  return false;
	file=SPIFFS.open(fileName.c_str(),mode);
	if(!file) {
		ERR("Failed to open file %s\n");
		return false;
	}
	DBG("File open\n");
	
	return true;
}

