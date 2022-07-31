/*
 ESP32 BLE Beacon scanner

  Copyright (c) 2022 Kim Lilliestierna. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include <NimBLEDevice.h>
#include <NimBLEAdvertisedDevice.h>
#include "NimBLEEddystoneURL.h"
#include "NimBLEEddystoneTLM.h"
#include "NimBLEBeacon.h"
#include <string_view>
#include <ArduinoJson.h> 
#include "Config.hpp"
#include "Devices.hpp"
#include "dbgLevels.h"

#define DEBUG_LEVEL DBG1_DBGL
#include "debug.h"
#include "Util.hpp"

// Scanner
#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

// Scanner Variables
int scanTime = 5; //In seconds //5
BLEScan *pBLEScan;

// Distance Calculation
float calculateAccuracy(float txCalibratedPower, float rssi){
		float ratio_db = txCalibratedPower - rssi;
		float ratio_linear = pow(10, ratio_db / 10);

		float r = sqrt(ratio_linear);
		r = r /20;
		return r;
}

// Scanner

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks{

	void onResult(BLEAdvertisedDevice *advertisedDevice){
	float distance;
	float RSSI;
	float sigPower;
	char * devInfo;

	//
		// check if Beacon
/*
		if (advertisedDevice->haveManufacturerData() == true
			&& advertisedDevice->getManufacturerData().length() == 25
			&& advertisedDevice->getManufacturerData()[0]== 0x4c
			&& advertisedDevice->getManufacturerData()[1]== 0x00) {

			BLEBeacon oBeacon=BLEBeacon();
			oBeacon.setData(advertisedDevice->getManufacturerData());
			auto search=devices.find(oBeacon.getProximityUUID().toString());
 			if(devices.exists(uuid)........
*/

		if (advertisedDevice->haveManufacturerData() == true){
			std::string strManufacturerData = advertisedDevice->getManufacturerData();

			uint8_t cManufacturerData[100];
			strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);

			if (strManufacturerData.length() == 25 && cManufacturerData[0] == 0x4C && cManufacturerData[1] == 0x00){
				VERBOSE("Found beacon: %s\n",advertisedDevice->toString().c_str());
				BLEBeacon oBeacon = BLEBeacon();
				oBeacon.setData(strManufacturerData);
				std::string uuid = oBeacon.getProximityUUID().toString();

				// is it in the list ?
				if ( devices.exists(uuid)) {
					sigPower = oBeacon.getSignalPower();
					RSSI= advertisedDevice->getRSSI();
					distance = calculateAccuracy(sigPower, RSSI);
					asprintf(&devInfo,
							"{ \"id\": \"%s\", \"name\": \"%s\", \"distance\": %2.2f , \"power\": %3.2f , \"rssi\": %3.2f }",
							oBeacon.getProximityUUID().toString().c_str(), 
							devices[uuid].c_str(), 
							distance, 
							sigPower,
							RSSI 
					);
					DBG1("Found: name %s, distance %2.2f , power: %3.2f , rssi: %3.2f\n",
						oBeacon.getProximityUUID().toString().c_str(), 
						devices[uuid].c_str(), 
						distance, 
						sigPower,
						RSSI 
					);

					sendWsText(DEVICE_MSG, devInfo);
					publishToTopic(mqttScanTopic,devInfo);
					delay(50);
					free(devInfo);
				} 
//				else { DBG3"Found beacon (not in list): %s\n", uuid.c_str() ); }
			}
		}
		else {
			VERBOSE("Found Bluetoth device: %s\n", advertisedDevice->getAddress().toString().c_str());
		}

		return;
	}
};


void setupBluetoothScanner() {
	INFO("Setting up Bluetooth\n");
	BLEDevice::init("");
	pBLEScan = BLEDevice::getScan(); 	//create new scan
	pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
	pBLEScan->setActiveScan(true);		//active scan uses more power, but get results faster
	pBLEScan->setInterval(100);
	pBLEScan->setWindow(90);			// less or equal setInterval value
}

void runScan(){
	DBG2("Runing Scan\n");
	BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
	pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
}
