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
#include "Settings.hpp"
#include "Util.hpp"

#include "dbgLevels.h"
//#define DEBUG_LEVEL VERBOSE_DBGL
#define DEBUG_LEVEL DBG3_DBGL
#include "debug.h"
/* base class for all other settings like Config and Devices
 *
 */
Settings::Settings(std::string fileName_arg) {
		fileName=fileName_arg;

		if (SPIFFS.begin()) {
			fsValid=true;
		}
		error=OK_E;
		errorS=nullptr;
	}


bool Settings::openFile(const char * mode){
	DBG2("Settings: openFile\n");

	if(!fsValid){
		if(!SPIFFS.begin()) {	// should have been done by the constructor lets try again
			setError(SPIFFS_E, " Setting:When opeing %s for %s",fileName,mode);
			return false;
		} else fsValid = true;
	}

	if(fileName.size() ==0){
		setError(FileName_E,"Setting: trying to open %s",fileName);
		return false;
	}

	DBG3("Settings: Open(%s for %s )\n",fileName.c_str(),mode);
	file=SPIFFS.open(fileName.c_str(),mode);
	if(!file || file.size() == 0) {
		setError(FileOpen_E,"%s for %s",fileName.c_str(),mode);
		return false;
	}
	VERBOSE("file opened\n");
	return true;
}
	/*! close file
	 */
void Settings::closeFile(){ 
	VERBOSE("Closing File\n");
	file.close();
}

void Settings::setError(errorT errorCode,const char * format,...){
	va_list ap;
	char * tmp;
	
	// sett instance error variables
	if(errorCode >= LastErrorConstant) {
		printf("setError: code out of bounds %d\n",errorCode);
		return;
	}
	error=errorCode;
	errorS=errorStrings[errorCode];

	va_start(ap,format);
	vasprintf(&tmp,format,ap);
	printf("%s: %s\n",errorS,tmp);

	setStatus(ERROR_MSG,ALL,"%s : %s\n",errorS,tmp);
	delay(100);
	free(tmp);

};
