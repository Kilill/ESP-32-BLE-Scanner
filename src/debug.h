#ifndef __DEBUG_H__
#define __DEBUG_H__
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

/*! \file debug.h
 * debug defines
 *
 * a set of shortand macros for debugging purposes
 */

#include "dbgLevels.h"
#ifndef DEBUG_LEVEL 
#define DEBUG_LEVEL WARN_DBGL
#endif

#define FAIL_L    (DEBUG_LEVEL >= FAIL_DBGL)
#define ERROR_L   (DEBUG_LEVEL >= ERR_DBGL)
#define WARN_L    (DEBUG_LEVEL >= WARN_DBGL)
#define INFO_L    (DEBUG_LEVEL >= INFO_DBGL)
#define DBG1_L    (DEBUG_LEVEL >= DBG1_DBGL)
#define DBG2_L    (DEBUG_LEVEL >= DBG2_DBGL)
#define DBG3_L    (DEBUG_LEVEL >= DBG3_DBGL)
#define VERBOSE_L (DEBUG_LEVEL >= VERBOSE_DBGL)
#define ALL_L     (DEBUG_LEVEL >= ALL_DBGL)

/*! \def FAIL(format,...) 
 * \brief failure events, cant be masked out
 *
 * events makes the system non functional, should probably lead to a reboot, 
 * system might be able to recover
 **/

#if FAIL_L
#define FAIL(format,...) dbg(FAIL_DBGL,format,##__VA_ARGS__)
#else
#define FAIL
#endif

/*! \def ERROR(format,...) 
 * \brief fatal level errors , system is non functional but might recover
 **/

#if  ERROR_L
#define ERR(format,...) dbg(ERROR_DBGL,format,##__VA_ARGS__)
#else
#define ERR
#endif

/*! \def WARN(format,...) 
 * \brief non ERROR or FAIL conditions 
 *
 * System if operational but with restrictions
 **/
#if  WARN_L
#define WARN(format,...) dbg(WARN_DBGL,format,##__VA_ARGS__)
#else
#define WARN
#endif

/*! \def INFO(format,...) 
 * \brief general information messages
 **/
#if  INFO_L
#define INFO(format,...) dbg(INFO_DBGL,format,##__VA_ARGS__)
#else
#define INFO
#endif


/*! \def DGB1(format,...) 
 * \brief level 1 debug, mayor descisions
 **/

#if  DBG1_L
#define DBG1(format,...) dbg(DBG1_DBGL,format,##__VA_ARGS__)
#else
#define DBG1
#endif

#define DBG DBG1

/*! \def DGB2(format,...) 
 * \brief level 2 debug, minor descisions
 **/

#if  DBG2_L
#define DBG2(format,...) dbg(DBG2_DBGL,format,##__VA_ARGS__)
#else
#define DBG2
#endif

/*! \def DGB3(format,...) 
 * \brief level 3 debug, minor status
 **/

#if  DBG3_L
#define DBG3(format,...) dbg(DBG3_DBGL,format,##__VA_ARGS__)
#else
#define DBG3
#endif

/*! \def VERBOSE(format,...) 
 * \brief verbose all of the previous + minor variabels
 **/

#if  VERBOSE_L
#define VERBOSE(format,...) dbg(VERBOSE_DBGL,format,##__VA_ARGS__)
#else
#define VERBOSE
#endif

/*!
 * dynamic debug level 
 *
 * global variable used to dynamically controll debugging, it as no effect if set higher than DEBUG_LEVEL though since those
 * macros wont be included in the code
 */
extern uint16_t  DbgLevel; 
						
/*!
 * send out a debug message to the serial lines
 *
 * will only print if level >  DbgLevel 
 * \param level [in] debug level required to print message
 * \param format [in] printf format string
 * \param ... [in] varargs arguments to format string
 */

void dbg(uint16_t level,const char* format,...);

/*!
 * set current global debug level
 *
 * \param level [in] level to set DbgLevel to
 */
void setDbgLvl(uint16_t level);

#endif

