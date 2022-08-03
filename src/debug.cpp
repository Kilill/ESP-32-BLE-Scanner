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

#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "dbgLevels.h"
//#define DEBUG_LEVEL DBG_DBGL
#include "debug.h"
uint16_t DbgLevel;

void dbg(uint16_t level,const char * format,...) {
va_list ap;
const char* tmp;
    if(( level <= DbgLevel ) && format!=nullptr) {  // level not high enough or no format ? ignore...
		switch (level) {
			case DBG1_DBGL:
			case DBG2_DBGL:
			case DBG3_DBGL:
			case VERBOSE_DBGL:
				tmp="DBG:";
				break;
			case WARN_DBGL:
				tmp="WARNING::";
				break;
			case ERROR_DBGL:
				tmp="ERROR:";
				break;
			case FAIL_DBGL:
				tmp="FAIL:";
				break;
			default:
				tmp="";
		}
		printf(tmp);			// print preample if anny
		va_start(ap, format);
		vprintf(format,ap);
	}
}

void setDbgLvl(uint16_t level) {DbgLevel=level;}


