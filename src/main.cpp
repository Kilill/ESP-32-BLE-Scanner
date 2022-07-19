#include "Arduino.h"
#include <stdio.h>
#include <string>
#include <map>
#include "SPIFFS.h" 
#include <ArduinoJson.h> 
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "time.h"

#include "Settings.hpp"
#include "Config.hpp"
#include "Devices.hpp"
#include "Util.hpp"

#define  DEBUG_LEVEL INFO_L
#include "dbgLevels.h"
#include "debug.h"

void setupWIFI();
void setupWebServer();
void setupMQTT();
void setupBluetoothScanner();
void runScan();


extern bool mqttSettingsAvailable;
extern bool wifiAccesPointModeEnabled;

/*
static FILE uartout = {0};
static int uart_putchar(char c, FILE *stream) {
	Serial.write(c);
	return 0;
}

static int serial_putc(const char c, FILE *stream) { return Serial.write(c); }
static FILE *serial_stream = fdevopen(serial_putc, NULL);
*/



Config config("/config.json");
Devices devices("/devices.json");

void setup() {
	Serial.begin(115200);
	setDbgLvl(VERBOSE_L);
	/*
	fdev_setup_stream((&uartout,uart_putchar,NULL,_FDEV_SETUP_WRITE);
	stdout = serial_stream;
	*/

	INFO("ESP32 BLE Scanner setup\n");
	config.load();
	if(config.debugLvl != 0 )
		setDbgLvl(config.debugLvl);
	setupWIFI();

	setupWebServer();
	setupMQTT();
	devices.load();
	setupBluetoothScanner();

	INFO("ESP32 BLE Scanner starting \n");

}

void loop() {
#if DEBUG_LEVEL == VERBOSE_L
	long int fremem = heap_caps_get_free_size(MALLOC_CAP_8BIT);
	VERBOSE("Free memory = %d\n",fremem);
#endif

	// for some reason we need to feed the dog
	TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
	TIMERG0.wdt_feed=1;
	TIMERG0.wdt_wprotect=0;

	if (!wifiAccesPointModeEnabled) {
			runScan();
	}
} 
