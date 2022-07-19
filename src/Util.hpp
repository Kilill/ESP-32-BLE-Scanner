#ifndef __util_h__
#define __util_h__
#include <string>
#include <map>
#include <Config.hpp>
#include <Devices.hpp>

#define MSG printf

enum MsgType : uint16_t {
	OK_MSG,
	DEVICE_MSG,
	INFO_MSG,
	DBG_MSG,
	WARN_MSG,
	ERROR_MSG,
	FAIL_MSG,
} ;
#define B(bit) (1<<(bit))

enum MsgTarget_t : uint16_t {
	PRINT=B(0),
	MQTT =B(1),
	WEB  =B(2),
	ALL  =7
} ;

extern const char *  MsgTypeTxt[];


extern Devices devices;
extern Config config;

extern char *mqttScanTopic;
extern char *mqttStateTopic;

void setStatus(MsgType status, uint16_t targets, const char * format,...);
void sendWsText(MsgType type,const char *message);
extern int publishToTopic(char *mqttScanTopic,char mqtt_msg[]);

void setupWIFI();

#endif
