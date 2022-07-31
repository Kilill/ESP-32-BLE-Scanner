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

#define  DEBUG_LEVEL VERBOSE_DBGL
#include "dbgLevels.h"
#include "debug.h"

void setupWIFI();
void setupWebServer();
void setupMQTT();
void setupBluetoothScanner();
void runScan();


extern bool mqttSettingsAvailable;
extern bool wifiAccesPointModeEnabled;



Config config("/config.json");
Devices devices("/devices.json");
TimerHandle_t restartTimer;


void restartNow() {
	INFO("triggering restart\n");
//Todo: gracefully disconnect from Mqtt/Ntp/Wifi....);
	ESP.restart();
}
void triggerReset(){
	xTimerStart(restartTimer, 0);
}

void setup() {
	Serial.begin(115200);
	setDbgLvl(VERBOSE_L);

	INFO("ESP32 BLE Scanner setup\n");

	DBG("Loading config\n");
	config.load();

	// is config overriding the debug level ?
	if(config.debugLvl != 0 ) setDbgLvl(config.debugLvl);

	DBG("Starting up wifi\n");
	setupWIFI();

	DBG("Starting webserver\n");
	setupWebServer();

	DBG("Starting MQTT\n");
	setupMQTT();
	DBG("Loading Devices\n");
	devices.load();
	DBG("Starting Bluetooth\n");
	setupBluetoothScanner();
	DBG("enabling kill timer");

	restartTimer = xTimerCreate("restartTimer", pdMS_TO_TICKS(5000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(restartNow));

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
