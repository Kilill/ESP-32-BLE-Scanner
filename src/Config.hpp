#ifndef __config_h__
#define __config_h__
#include <string>
#include "Settings.hpp"

/*! \file Config.hpp 
 * \brief Config class
 *
 * Holds all configration values
 */

const std::string DEFAULT_EMPTY="";							///< default Empty
const std::string DEFAULT_UKNOWN="Uknown";					///< default Empty
const std::string DEFAULT_CONFIG_FILE_NAME="/config.json"; 	///< default file name for config file in SPIFFS
const std::string DEFAULT_IP_ADDRESS="192.168.4.1";			///< defauult ip4 adress when running in AP mode
const std::string DEFAULT_NET_MASK="255.255.255.0";			///< default ip4 netmaska when running in AP mode
const std::string DEFAULT_HOST_NAME="Beacon_Scanner";		///< default host/ssid name
const std::string DEFAULT_LOCATION="Limbo";					///< default location
const std::string DEFAULT_WIFI_PASSWORD="BLEScanner32";		///< default wifi passord
const std::string DEFAULT_MQTT_USER="BLEScanner32";			///< default mqtt user
const std::string DEFAULT_MQTT_PORT="1883";					///< default mqtt port number on mqtt server
const std::string DEFAULT_STATUS_TOPIC="/has/beacon/status/";	///< default mqtt status topic
const std::string DEFAULT_SCAN_TOPIC="/has/beacon/scan/";	///< default mqtt scan topic

/*!
 * size of json document recomended by https://arduinojson.org/v6/assistant/#/step1
 *
 * based on the following absoloutly worst case format :

{
	"ssid"					: "abcdefghijklmnopqrstuwvyz12345",
	"password"				: "abcdefghijklmnopqrstuwvyz12345",
	"hostname"				: "abcdefghijklmnopqrstuwvyz12345",
	"ipaddress"				: "abcdefghijklmnopqrstuwvyz12345",
	"apipaddress"			: "123.123.123.123",
	"netmask"				: "123.123.123.123",
	"mqttServer"			: "abcdefghijklmnopqrstuwvyz12345",
	"mqttPort"				: "12345",
	"mqttUser"				: "abcdefghijklmnopqrstuwvyz12345",
	"mqttPassword"			: "abcdefghijklmnopqrstuwvyz12345",
	"mqttScanTopicPrefix" 	: "abcdefghijklmnopqrstuwvyz12345",
	"mqttStateTopicPrefix"	: "abcdefghijklmnopqrstuwvyz12345",
	"location"				: "abcdefghijklmnopqrstuwvyz12345",
	"ntpServer"				: "abcdefghijklmnopqrstuwvyz12345",
	"gmtOffset"				: "12345",
	"daylightOffset"		: "12345"
	"debugLvl"				: 0
 }

 * minimum size is 848 and adding a bit of slack to the neares binary multiple
 * gives 1024
 */
#define CONFIG_DOC_SIZE 1024

/*!
 * \class Config
 *
 * Hold all config values in the system
 */
class Config : public Settings
{
public:

	std::string ssid;					///< ssid to connect to
	std::string password;				///< wifi password used for both ap and station
	std::string hostname;				///< base hostname config will add location 
	std::string location;					///< location id/name we are reporting from
	std::string fullHostname;			///< filled in load() from hostname+"_"+location
	std::string ipaddress;				///< ip adress of wifi in client mode, blank for DHCP
	std::string apipaddress;			///< ip adress of wifi in AP mode
	std::string netmask;				///<  netmask to use in AP mode
	std::string mqttServer;				///< mqttserver to send reports to
	std::string mqttPort;				///< port
	std::string mqttUser;				///< User used to connect the mqttserver
	std::string mqttPassword;			///< Password
	std::string mqttScanTopicPrefix;	///< what base topic to send scan reports to, location will be appended
	std::string mqttStateTopicPrefix;	///< where to report state changes location will be apended
	std::string fileName;				///< filename of config gile
	// time
	std::string ntpServer; 				///< time serve
	int gmtOffset;						///< Local offser from gmt in seconds
	int daylightOffset;					///< Daylight saving time in seconds

	uint16_t debugLvl;					///< Default debug level at start up see debug.h for explanatiion

	bool mqttValid;						///< is mqtt config valid
	bool webValid;						///< is web and Wifi config valid
	bool valid;							///< is config valid
	
	/*!
	 * constructor
	 *
	 * @param cfgFileName [in]	file name to read config from
	 */
	Config(std::string cfgFileName=DEFAULT_CONFIG_FILE_NAME);

	/*!
	 * fill in the variables from a jsondov
	 *
	 * @param doc reference to json document to read
	 * @return true on success false on failure
	 */
	bool fillit(JsonVariant jConf);

	/*!
	 * load data from json document indicated by the fileName class variable
	 *
	 * will send a logg message to the web guil on success or failure
	 *
	 * @return true on success false on failure
	 */
	bool load();

	/*!
	 * save data from json document indicated by the fileName class variable
	 *
	 * will send a logg message to the web guil on success or failure
	 *
	 * @return true on success false on failure
	 */
		bool save();
private:
		uint16_t checkArg(std::string& arg,const std::string& argDefault, errorT errorCode); 
};

#endif
