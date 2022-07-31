# ESP 32 BLE Beacon Scanner
for Home Assistant. [See thread at Home Assistant Board.](https://community.home-assistant.io/t/esp-32-ble-scanner-a-room-presence-detection-solution/315205)

### This is a fork

This is a fork from https://github.com/realjax/ESP-32-BLE-Scanner wich in turn is a fork from https://github.com/HeimdallMidgard/ESP-32-BLE-Scanner

I concurr with realjax that it is "An excellent idea", i also started with the version from HeimdalMidgard but whanted some additions so tried the "realjax" fork but there was still some things i whanted to change,
So I in turn forked to see if i could fix those issues. But ended up refactoring a lot of the code. If it is easier to understand or  extend upon i have no idea.


### Installing 
You will need the ESPRESSIF tools to build and uppload the firmware and filesystem images and frankly the easies way of getting all ther requisists is to use Platformio to build and upload new image files
Only need the "platformio core" version is needed see https://docs.platformio.org/en/latest/core/installation/index.html. 

connect the ESP to the PC via USB 

`pio run`  will checkfor / download all requist tools and libraries and then build the firmware

before building the filesystem , copy the *config.json.dist* and *devices.json.dist* in the **data** directory to *config.json* and *devices.json* repectively in the same directory.
edit both files to your liking. They need to conform to the json format of the files. 

Now run `pio run -r buildfs` to build the filesystem image.

`pio run -t uploadfs` will upload the filesystem image
and
`pio run -t upload` will upload the firmware image

if you'r on a linux system and have `make` installed:

`make` builds both the firmware and filesystem images
`make up` uploads the firmware imate
`make upfs` uploads the filesystem image

### Configuration

The main configurarion is the _config.json_ file.

| Tag | Description |
| --- | --- |
| `ssid` | SSID of the local wifi net to connect to. If not present it forces  the scanner will start in AP mode. |
| `password` | The password for the above wifi net. If running in AP mode it will also be used as the AP password. |
| `hostname` | Together with `location` will be used as the complete hostname and if running in AP mode, the SSID of the scanner. |
| `ipaddress` | Hardcoded IP4 address, If blank, not set or invalid DHCP will be used to get an ip address. |
| `apipaddress` | IP4 Address to use in AP mode. Defaults to 192.168.4.1 if not set or invalid. |
| `netmask` | Netmask to use. Unless DHCP is used to set up `ipaddress`. It defaults to 255.255.255.0 if blank. |
| `mqttServer` | Domain name  or IP address to mqtt server. |
| `mqttPort` | Port on mqttserver to connect to. Defaults to 1883. |
| `mqttUser` | Mqtt user with submit previlieges. No default. |
| `mqttPassword` | Password for same. No default. |
| `mqttScanTopicPrefix` | Scan topic base path for scan  messages. `location` will be concatenated at the end to form full topic defaults to _/has/presence/scan_ if not set. |
| `mqttStateTopicPrefix` | Status topic base path for status messages. `location` will be concatenated at the end to form full topic defaults to _/has/presence/status_ if not set. |
| `location` | Name of location where sensor is located. added to the base topic path as well as to hostname. |
| `ntpServer` | FQDN /IP address of nntp server used for retreiving correct time. Defaults to pool.ntp.org. |
| `gmtOffset` | GMT offset of local time zone in seconds , ie EU is at 3600 . |
| `daylightOffset` | Daylight saving offset from GMT if in effect. |

The _devices.json_ contains known devices for wich to report presence and needs to conform exactly to the format.

| Tag | Description |
| --- | --- |
|`uuid` | Advertising UUID of the device, needs to conform exactly to the BLE beacon UUID format :"aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"|
| name | Name of device to report on max 30 characters |


### Changes compared to original 
Upgraded versions of some of the libraries.
Mainly changed the internal representation of devices to use std:map so it is easier to add/substract devices. (planned: allow for adding devices from mqtt...).
Added entries to the config file;
Added debug facility to send debug messages to serial. Controlled by debug macros that exclude the debug code if level not set.
Changed the gui to request / post json information directly.
Gui now also sanity checks eventual entries for devices as well as some config entries
Gui for devices will enforce the BLE UUID format, and restricts location names to 30 characters


Notes
* If you have not adjusted config.json and added your wifi credentials, then when you first fire up the esp32, it will be in AP mode. Connect to the wifi AP by the name of 'ESP32 BLE Scanner' and then in your browser connect to http://192.168.4.1  You can then change the settings on the config page accordingly.
* The bluetooth scanning will only work once there is an active MQTT connection set
