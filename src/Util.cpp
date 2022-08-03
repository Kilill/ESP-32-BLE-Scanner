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
#include <AsyncMqttClient.h>
#include <AsyncMqttClient/DisconnectReasons.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "Util.hpp"
#include "time.h"

#include "dbgLevels.h"
#define DEBUG_LEVEL DBG_DBGL
#include "debug.h"


const char * MsgTypeTxt[] ={
	"OK",
	"DEVICE",
	"INFO",
	"DBG",
	"WARN",
	"ERROR",
	"FAIL"
};

/*! setStatus
 *
 * send status message to combination of serial, Mqtt, Web
 * \param Stat_t stat	[in] type of status 
 * \param StatTarget_t	[in] destination(s)  one of WEB,MQTT,PRINT or anny ored combination ie {WEB|PRINT)
 * \param char *		[in] format printf format
 * \param ...  				[in] eventual printf arguments
 */

void setStatus(MsgType type ,uint16_t targets,const char * format,...) {
va_list ap;
char* tmp;

	if(!format) return; //no format ? get out...

	printf("SetStatus: before va_start\n");

	va_start(ap, format);
	printf("SetStatus: before va_sprintf\n");
	vasprintf(&tmp,format,ap); // inital formating of message (and dont forget freeing tmp below...)

	printf("setStatus: sending msg %s to web\n",tmp);
	if((targets & WEB) && WebUp) sendWsText(type,tmp); // sendWsText will handle the msg type

	// add messge type in front
	
	if((targets&MQTT) && mqttConnected) {
		char* msg;
		struct tm *now;
		time_t stamp;
		char times[30];
		time(&stamp);
		now=localtime(&stamp);
		if (now != nullptr) 
			strftime(times,30,",\"Date\": \"%d/%m/%Y %R\"",now);
		else 
			times[0]='\0';

		DBG2("Publishing to mqtt Status\n");
		asprintf(&msg,"{\"Status\":\"%s\",\"Msg\":%s\"%s}\n",MsgTypeTxt[type],tmp,times);;
		VERBOSE("message: %s\n",msg);
			publishToTopic(mqttStateTopic,msg);
		free(msg);
	}
	free(tmp);
	VERBOSE("setStatus End\n");
}

