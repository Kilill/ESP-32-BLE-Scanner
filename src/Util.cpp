#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "Util.hpp"
#include "time.h"
#include "dbgLevels.h"
//#define DEBUG_LEVEL DBG_L
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



/*! \func setStatus
 * send status message to combination of serial, Mqtt, Web
 * \param Stat_t stat type of status 
 * \param StatTarget_t destination(s) 
 * \param char * format printf format
 * ...  eventual printf arguments
 */

void setStatus(MsgType status ,uint16_t targets,const char * format,...) {
va_list ap;
char* tmp=nullptr;

	if(!format) return; //no format ? get out...

	va_start(ap, format);
	vasprintf(&tmp,format,ap); // inital formating of message

	if(targets & WEB) sendWsText(status,tmp); // sendWsText will handle the msg type

	// add messge type in front
	if(targets&(PRINT|MQTT)) {
		char * msg;
		struct tm *now;
		time_t stamp;
		char ts[30];
		time(&stamp);
		now=localtime(&stamp);
		if (now != nullptr) 
			strftime(ts,30,",\"Date\": \"%d/%m/%Y %R\"",now);
		else 
			ts[0]='\0';

		asprintf(&msg,"{\"Status\":\"%s\",\"Msg\":%s\"%s}\n",MsgTypeTxt[status],tmp,ts);;

		if(targets & PRINT) INFO("SetStatus: %s",msg);
		if(targets & MQTT)  publishToTopic(mqttStateTopic,msg);

		free(msg);
	}
	free(tmp);
}

