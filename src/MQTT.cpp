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
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <AsyncMqttClient/DisconnectReasons.hpp>
#include <ArduinoJson.h> 
#include "Settings.hpp"
#include "Config.hpp"
#include "Devices.hpp"
#include "Util.hpp"


#include "dbgLevels.h"
//#define DEBUG_LEVEL DBG3_DBGL
#include "debug.h"


//--PubSubClient pubSubclient(espClient);
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
bool mqttConnected;

bool mqttSettingsAvailable = false;
bool mqttCredentialsAvailable = false;
char *mqttScanTopic;
char *mqttStateTopic;

/*! \fn MQTTconnect 
 * connect to mqtt called by mqttReconnctTimer
 * \return void
*/
void MQTTconnect() {
static int connectCount=1;
	DBG3("Mqtt connect atempt: %d\n",connectCount);
	connectCount++;
	if (WiFi.isConnected()) {
		setStatus(INFO_MSG,(WEB|PRINT),"Connecting to Mqtt, atempt: %d",connectCount);
		mqttClient.connect();
	} else {
		ERR("Wifi not connected cant connect to mqtt yet\n");
		setStatus(ERROR_MSG,PRINT,"FAIL: Wifi not connected, cant connect MQTT");
	}
}

/*! \fn onMqttConnect
 * on connect call back
 * send "online" to status topic
 * \param bool sessionPresent 
 */
void onMqttConnect(bool sessionPresent) {
	uint16_t msgid;
	time_t stamp;
	struct tm *now;
	char ts[30]; 

	INFO("Connected to MQTT\n");
	VERBOSE("Session %s\n",sessionPresent?"pressent":"not present");

	DBG1("publishing onlne status to %s\n",mqttStateTopic);

	// publish "online" status to the location, dont care about returned msg id 
	time(&stamp);
	now=localtime(&stamp);
	if (now != nullptr)  {
		strftime(ts,30,"at: %d/%m/%Y %R",now);		  
	} else 
		ts[0]='\0';

	mqttConnected=true;
	setStatus(INFO_MSG,ALL,"Online %s",ts);
}

/*! \fn onMqttDisconnect
 * if DEBUG defined, prints out disconnectioins reason
 * send message to evential webclient
 * sets up reconnect timer
 * \param AsyncMqttClientDisconnectReason reason reason for dissconnection
 */
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
	const char * rp;
	char * msgp;

	mqttConnected=false;
	switch (reason) {
		case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED :
			rp="TCP_DISCONNECTED";
			break;
		case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
			rp= "MQTT_UNACCEPTABLE_PROTOCOL_VERSION";
			break;
		case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED:
			rp="MQTT_IDENTIFIER_REJECTED";
			break;
		case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE:
			rp="MQTT_SERVER_UNAVAILABLE";
			break;
		case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS:
			rp="MQTT_MALFORMED_CREDENTIALS";
			break;
		case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED:
			rp="MQTT_NOT_AUTHORIZED";
			break;
		case AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE:
			rp="ESP8266_NOT_ENOUGH_SPACE";
			break;
		case AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT:
			rp="TLS_BAD_FINGERPRINT";
			break;
		default:
			rp="Uknown";
	}
	
	DBG3("Disconnected from MQTT:%s",rp);
	setStatus(ERROR_MSG,(WEB|PRINT),"Disconnected from MQTT:%s",rp);;

	if (WiFi.isConnected()) {
		DBG3("Starting reconect timer\n");
	// try reconnecting in 5 seconds see setup;
		xTimerStart(mqttReconnectTimer, 0);
	}
}

/*! \fn onMqttPublish
 * message published callback
 * if DEBUG print out message id
 * \param unit16_t msgid message id
 */

void onMqttPublish(uint16_t msgId) {
	DBG3("Received publish ack. Id: %d\n",msgId);
}

//TODO: add incoming message handling

/*! fn onMqttMessage
 * recieved message callback
 */
/* 
 * have to figure out how to authenticate the sender...
 * server password perhaps ?
 * should idealy be by asymetric key...
 * but that is probably overkill
 *
int onMqttMessage( ??? ) {
	json document from arg
	should be in the format:
	{"cmd":"cmdarg", "data":"cmddata"}
	where cmdarg is one of:

	{"cmd":"addDev", "data":[{"uuuid": "36char UUid","name:"max 30 char name"},...]}
		add a device to the list of recognized devices,
		nothing is done and an ERROR_MSG is sent to the status topic IF:
		 data content is empty
		 or if the current device list count is >= MAX_DEVICES 
		 or if anny of uuid or name is longer than MAX_UUID_LEN or MAX_NAME_LEN

		otherwise the device is added to the "devices" map, saved to the devices.json file	and a OK_MSG is sent to in status topic

	{"cmd":"remDev", "data":[{"uuuid": "36char UUid","name:"max 30 char name"},...]}
		if the devices list contains an entry with the uuid and name it is removed
		if either uuid or name is empty the compare is done on the non empty member
		if both are empty nothing is done and an ERROR_MSG is sent to the status topix

	{"cmd":"listDev"}
i		complete json device file is sent as an INFO_MSG to the status topic
		otherwise the device with either the uuid or name is sent

	{"cmd": "listConf"}
		the complete config file is sent back in an INFO_MSG to the status topic
		otherwise only the setting for tagname is sent back
		if tagname is invalid an ERROR_MSG is sent back to the status topic

	{"cmd": "setConf", "data":{ "tag":"config tag", "value": "new data",....}}
		set config entry "config tag" the the value of "new data";
		OK_MSG is sent on success to the status_topic and the devices is rebooted with the new config

		if uknown config_tag , or data is out of bounds or file save fails
		ERROR_MSG is sent to status topic 

	{"cmd":"start"}
		start reporting pressence

	{"cmd":"stop"}
		stop reporting presence
		presence reports will not be sent untill a "start" or the scaner is rebooted

	{"cmd": "reboot"}
}
*/

int publishToTopic(char * topic, char *mqtt_msg){
int msgId=0;
	if (mqttClient.connected()){
		msgId=mqttClient.publish(topic, 1, false, mqtt_msg);
		DBG1("mqttPublish to: %s - %s \n", topic, mqtt_msg);
		if(!msgId) {
			setStatus(WARN_MSG,(WEB|PRINT),"Failed to publish message to MQTT topic: \"%s\"",topic);
		}
	} else {
		setStatus(WARN_MSG,(WEB|PRINT),"MQTT not connected, can't publish. Trying Reconnect\"}\n");
		xTimerStart(mqttReconnectTimer, 0);
	}  
	return msgId;
}
//TODO: this asumes WIFI is upp and running, maybe check for that here,
//even though this is not called by main untill wifi is up....
void setupMQTT() {
	const int DOT_DELAY=1000;		//millisec to wait 
	const int DOT_COUNT	= 10;		// number of wait loops between printng a "."
	const int TIMEOUT_COUNT = 60;	// number of loops before printing a fail message
	int dotCounter=0; 
	int timeoutCounter = TIMEOUT_COUNT;

	INFO("Setting up Mqtt\n");

	if (config.mqttValid) {
		mqttClient.setServer(config.mqttServer.c_str(),(uint16_t) std::stoi(config.mqttPort));
		mqttSettingsAvailable = true;

		DBG1("host:%s, port:%d\n",config.mqttServer.c_str(),std::stoi(config.mqttPort));

		if (config.mqttUser.size() > 0 && config.mqttPassword.size() > 0){
			VERBOSE("User: %s Password: %s\n",config.mqttUser.c_str(), config.mqttPassword.c_str());
			mqttClient.setCredentials(config.mqttUser.c_str(), config.mqttPassword.c_str());
			mqttCredentialsAvailable = true;
		}
#define LASTCHR(str) str[strlen(str)-1]

		if(mqttScanTopic != nullptr) free(mqttScanTopic);
		if(mqttStateTopic != nullptr) free(mqttStateTopic);

		asprintf(&mqttScanTopic,"%s%s",config.mqttScanTopicPrefix.c_str(),config.location.c_str());
		asprintf(&mqttStateTopic,"%s%s",config.mqttStateTopicPrefix.c_str(),config.location.c_str());
	
		DBG1("\tmqttScanTopic: %s\n\tmqttStatTopic: %s\n",mqttScanTopic,mqttStateTopic);
		
		mqttClient.onConnect(onMqttConnect);
		mqttClient.onDisconnect(onMqttDisconnect);
		mqttClient.onPublish(onMqttPublish);
		mqttClient.setWill(mqttStateTopic,1, true, "(\"Status\":\"INFO\",\"Msg\":\"Offline\"}");

		mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(MQTTconnect));

		DBG1("Waiting on mqtt Connect\n");
		//xTimerStart(mqttReconnectTimer, 0);
		MQTTconnect();
		// no mqtt connections is a fatall condition, so hang here untill resolved or rebooted

		while(!mqttClient.connected()) {
			delay(DOT_DELAY);
			if(--dotCounter<=0) {
				dotCounter=DOT_COUNT;
				if (--timeoutCounter < 0) {
					DBG3("!");
					timeoutCounter=TIMEOUT_COUNT;
					setStatus(ERROR_MSG,(WEB|PRINT),"Failed to connect to mqtt server, Not giving up !\n");
				} else  {
					DBG3("?");
				}
				xTimerStart(mqttReconnectTimer, 0);  // trigger a reconnect "real soon";
			} else {
				DBG3(".");
			}
		}
		printf("\n");

		INFO("MQTT Connected\n");
		setStatus(INFO_MSG,(ALL),"MQTT Connected");

	} else{
		ERR("Invalid mqtt config\n");
		setStatus(ERROR_MSG,(WEB|PRINT),"MQTT config not valid, can't set up the connection!");
	}

	DBG2("Mqtt setup done\n");
}


