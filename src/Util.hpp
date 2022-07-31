#ifndef __util_h__
#define __util_h__
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

#include <string>
#include <map>
#include <Config.hpp>
#include <Devices.hpp>

enum MsgType : uint16_t {
	OK_MSG,
	DEVICE_MSG,
	INFO_MSG,
	DBG_MSG,
	WARN_MSG,
	ERROR_MSG,
	FAIL_MSG,
} ;

/*! \def B(bit)
 * return value with bit "bit" set
 */
#define B(bit) (1<<(bit))

/*! enum bitmap of report targets
 */
enum MsgTarget_t : uint16_t {
	PRINT=B(0),
	MQTT =B(1),
	WEB  =B(2),
	ALL  =7
} ;

extern const char *  MsgTypeTxt[]; 	///<list of message types names

extern Devices devices;				///< list of known devices 
extern Config config;				///< curent configuration

extern char *mqttScanTopic;			///< mqtt topic where to send scan results
extern char *mqttStateTopic;		///< mqtt topic where to send status reports

/*! reports the current status
 *
 * sends a json reports to one or several targest 
 * targets can any MsgType ored together
 *
 * @param status [IN] MsgType type of message to send on of enum MsgType
 * @param target [IN] MsgTarget_t bitmap of target(s) where to send message
 * @param format [IN] printf style format string
 * #param ...	[IN] eventual arugemtns to format string
 */
void setStatus(MsgType status, uint16_t targets, const char * format,...);

/*! sends a json formated message via websocket to eventual listening clients
 *
 * @param type 	[IN] MsgType type of message to send on of enum MsgType
 * @param message  [IN] message ot send
 */
void sendWsText(MsgType type,const char *message);

/*! published a message to the mqttserver
 *
 * @param topic 	[IN]  char * topic to send to 
 * @param mqtt_msg  [IN] message t0 send
 * @return int the message id
 */
extern int publishToTopic(char *topic,char mqtt_msg[]);

/*! setup wifi
 *
 * sets up the wifi depending on information from config
 *
 * if no ssid is set wifi will be set up in AP mode with ssid constructed from "hostname" and "room" with wifi password from  "password" in connfig
 * with the ipaddres from "apipaddress" in the config.
 *
 * othewise a connecition is tried to the ssid from "ssid" in the config with paswword from "password" in the configuration
 * if no ipaddress is specified in the condfig dhcp is used to get one
 *
 * will report progress on the serial port
 */
 
void setupWIFI();

/*! trigers the reset timer 
 *
 * after 5 second it triggers a hard reset
 */
 void triggerReset();

#endif
