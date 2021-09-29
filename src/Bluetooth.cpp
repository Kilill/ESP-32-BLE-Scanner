#include <NimBLEDevice.h>
#include <NimBLEAdvertisedDevice.h>
#include "NimBLEEddystoneURL.h"
#include "NimBLEEddystoneTLM.h"
#include "NimBLEBeacon.h"
#include <ArduinoJson.h> 
#include "fileStructs.h"


// Scanner
#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

// Scanner Variables
int scanTime = 5; //In seconds //5
BLEScan *pBLEScan;
char foundDeviceInfo[120];

extern Devices devices;
extern StaticJsonDocument<1000> getDevicesAsJson(Devices &devices);

extern void sendScanResultsToWebClient(char mqtt_msg[]);
extern void publishToScanTopic(char mqtt_msg[]);
extern void sendWsText(char level[],char message[]);


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

    if (advertisedDevice->haveManufacturerData() == true){
      std::string strManufacturerData = advertisedDevice->getManufacturerData();
      uint8_t cManufacturerData[100];
      strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);

      if (strManufacturerData.length() == 25 && cManufacturerData[0] == 0x4C && cManufacturerData[1] == 0x00){
        BLEBeacon oBeacon = BLEBeacon();
        oBeacon.setData(strManufacturerData);

        StaticJsonDocument<1000> devicesJson = getDevicesAsJson(devices);

        for (byte i = 1; i < 3; i++) {              
          String namey ="device_name";
          String devicey ="device_uuid";
          devicey = devicey + i;
          namey  = namey  + i;
          String device = devicesJson[namey];
            if ( oBeacon.getProximityUUID().toString() == devicesJson[devicey]  ) {
              float distance = calculateAccuracy(oBeacon.getSignalPower(), advertisedDevice->getRSSI());
              sprintf(foundDeviceInfo, "{ \"id\": \"%s\", \"name\": \"%s\", \"distance\": %f } \n", oBeacon.getProximityUUID().toString().c_str(), device.c_str(), distance );
              sendWsText("device", foundDeviceInfo);
              publishToScanTopic(foundDeviceInfo);
            }
        }
      }
    }
    return;
  }
};


void setupBluetoothScanner() {
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(90); // less or equal setInterval value

}

void runScan(){
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
}
