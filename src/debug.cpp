#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "dbgLevels.h"
//#define DEBUG_LEVEL DBG_L
#include "debug.h"


/*! \func debug
 * send status message to combination of serial, Mqtt, Web
 * \param Stat_t stat type of status 
 * \param StatTarget_t destination(s) 
 * \param char * format printf format
 * ...  eventual printf arguments
 */
uint16_t DbgLevel;

void dbg(uint16_t level,const char * format,...) {
va_list ap;
const char* tmp;

    if((level>=DbgLevel) && format!=nullptr) {  // level not high enough or no format ? igore...
		switch (level) {
			case DBG_L:
				tmp="DBG:";
			break;
			case ERROR_L:
				tmp="ERROR:";
			break;
			case FAIL_L:
				tmp="FAIL:";
				break;
			default:
				tmp="";
		}
		printf(tmp);
		va_start(ap, format);
		vprintf(format,ap);
	}
}

void setDbgLvl(uint16_t level) {DbgLevel=level;}


