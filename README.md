# ESP 32 BLE Beacon Scanner
for Home Assistant. [See thread at Home Assistant Board.](https://community.home-assistant.io/t/esp-32-ble-scanner-a-room-presence-detection-solution/315205)

### This is a fork

This is a fork from https://github.com/realjax/ESP-32-BLE-Scanner wich in turn is a for from https://github.com/HeimdallMidgard/ESP-32-BLE-Scanner

I concurr with realjax that it is "An excellent idea", i also started with the version from HeimdalMidgard but whanted some additions so tried the "realjax" fork but there was still some things i whanted to change,
So I in turn forked to see if i could fix those issues. But i  also ended up refactoring a lot of the code which made it. If it is easier to understand or  extend upon i have no idea.

### Installing 
the Bin directory contains the spiffs file system image and the firmware image (spiffs.bin and firmware.bin) but one still needs the ESPRESS  tools to upload them, the same goes for if you hand edit the config or deice json files you will beed
the espressif tools to build a new file system image.

so i think the easiest way to install it is via plaformio, you only need the platformio core version see https://docs.platformio.org/en/latest/core/installation/index.html. 

Running 'pio run -r build'  will download all requist tools and libraries.




Notes
* If you have not adjusted config.json and added your wifi credentials, then when you first fire up the esp32, it will be in AP mode. Connect to the wifi AP by the name of 'ESP32 BLE Scanner' and then in your browser connect to http://192.168.4.1  You can then change the settings on the config page accordingly.
* The bluetooth scanning will only work once there is an active MQTT connection set
