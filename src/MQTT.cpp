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
//#define DEBUG_LEVEL DBG_L
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
char *p;

	INFO("Connecting to MQtt\n");

	if (WiFi.isConnected()) {
		setStatus(INFO_MSG,(WEB|PRINT),"Connecting to Mqtt, atempt: %d",connectCount);
		mqttClient.connect();
		connectCount++;
	} else {
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

	DBG("publishing onlne status to %s\n",mqttStateTopic);

	// publish "online" status to the room, dont care about returned msg id 
	time(&stamp);
	now=localtime(&stamp);
	if (now != nullptr)  {
		strftime(ts,30," at: %d/%m/%Y %R",now);		  
	} else 
		ts[0]='\0';

	mqttConnected=true;
	setStatus(INFO_MSG,ALL,"Online%s",ts);
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
	
	setStatus(ERROR_MSG,(WEB|PRINT),"Disconnected from MQTT:%s",rp);;

	if (WiFi.isConnected()) {
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
	DBG("Received publish ack. Id: %d\n",msgId);
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

	{"cmd":"addDev", "data":{"uuuid": "36char UUid","name:"max 30 char name"}}
		add a device to the list of recofnized devices,
		if data content is empty
		or if the current device list count is >= MAX_DEVICES 
		or if anny of uuid or name is longer than MAX_UUID_LEN or MAX_NAME_LEN nothing is done and an ERROR_MSG is sent to the status topic
		otherwise the device is added to the "devices" map, saved to the devices.json file	and a OK_MSG is sent to in status topic

	{"cmd":"remDev", "data":{"uuuid": "36char UUid","name:"max 30 char name"}}
		if the devices list contains an entry with the uuid and name it is removed
		if either uuid or name is empty the compare is done on the non empty member
		if both are empty nothing is done and an ERROR_MSG is sent to the status topix

	{"cmd":"listDev", "data":{"uuuid": "36char UUid","name:"max 30 char name"}}
		if data is empty the complete json file is sent as an INFO_MSG to the status topic
		otherwise the device with either the uuid or name is sent

	{"cmd": "listConf", "data":{"tag":"tagame"}}
		if data is empty the complete config file is sent back in an INFO_MSG to the status topic
		otherwise only the setting for tagname is sent back
		if tagname is invalid an ERROR_MSG is sent back to the status topic

	{"cmd": "setConf", "data":{ "tag":"config tag","value": "new data"}}
		set config entry "config tag" the the value of "new data";
		OK_MSG is sent on success to the status_topic
		otherwise ERROR_MSG is sent

	{"cmd":"start"}
		start reporting pressence

	{"cmd":"stop"}
		stop reporting presence
		presence reports will not be sent untill a "start" or the scaner is rebooted
}
*/

int publishToTopic(char * topic, char *mqtt_msg){
int msgId=0;
	if (mqttClient.connected()){
		msgId=mqttClient.publish(topic, 1, false, mqtt_msg);
		DBG("mqttPublish to: %s - %s \n", topic, mqtt_msg);
		if(!msgId) {
			setStatus(WARN_MSG,(WEB|PRINT),"Failed to publish message to MQTT topic: \"%s\"",topic);
		}
	} else {
		setStatus(WARN_MSG,(WEB|PRINT),"MQTT not connected, can't publish. Trying Reconnect\"}\n");
		xTimerStart(mqttReconnectTimer, 0);
	}  
	return msgId;
}

void setupMQTT() {

	INFO("Setting up Mqtt\n");

	if (config.mqttValid) {
		mqttClient.setServer(config.mqttServer.c_str(),(uint16_t) std::stoi(config.mqttPort));
		mqttSettingsAvailable = true;

		DBG("host:%s, port:%d\n",config.mqttServer.c_str(),std::stoi(config.mqttPort));

		if (config.mqttUser.size() > 0 && config.mqttPassword.size() > 0){
			VERBOSE("User: %s Password: %s\n",config.mqttUser.c_str(), config.mqttPassword.c_str());
			mqttClient.setCredentials(config.mqttUser.c_str(), config.mqttPassword.c_str());
			mqttCredentialsAvailable = true;
		}
#define LASTCHR(str) str[strlen(str)-1]

		asprintf(&mqttScanTopic,"%s%s",config.mqttScanTopicPrefix.c_str(),config.room.c_str());
		asprintf(&mqttStateTopic,"%s%s",config.mqttStateTopicPrefix.c_str(),config.room.c_str());
	
		DBG("\tmqttScanTopic: %s\n\tmqttStatTopic: %s\n",mqttScanTopic,mqttStateTopic);
		
		mqttClient.onConnect(onMqttConnect);
		mqttClient.onDisconnect(onMqttDisconnect);
		mqttClient.onPublish(onMqttPublish);
		mqttClient.setWill(mqttStateTopic,1, true, "Offline");

		mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(MQTTconnect));

		DBG("waiting on mqtt Connect\n");
		int count=0;
		//xTimerStart(mqttReconnectTimer, 0);
		MQTTconnect();
		while(!mqttClient.connected()) {
			Serial.print(".");
			delay(1000);
			count++;
			if(count>=10) {
				count=0;
				xTimerStart(mqttReconnectTimer, 0);
				Serial.print("!");
			}
		}
		Serial.print("\n");

	} else{
		setStatus(ERROR_MSG,(WEB|PRINT),"MQTT config not valid, can't set up the connection!");
	}

	DBG("Mqtt setup end\n");
}


