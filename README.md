# ESP-32-BLE-Scanner
for Home Assistant. [See thread at Home Assistant Board.](https://community.home-assistant.io/t/esp-32-ble-scanner-a-room-presence-detection-solution/315205)

### This is a fork

This is a fork of this repo https://github.com/HeimdallMidgard/ESP-32-BLE-Scanner. An excellent idea, however I ran into some issues when trying to use it on my esp32. So I forked it in an attempt to fix those issues (changed the MQTT lib used for instance). I also ended up refactoring a lot of the code which made it, I think, somewhat easier to grasp and to extend upon. I am a programmer, but by no means an expert c++ programmer so there is undoubtedly still much room for improvement :). 
Unfortunately, at the moment I am not able to create a working .bin file from the code, so the only way to get this to work is to install it using PlatformIO (see [full original instructions](https://github.com/HeimdallMidgard/ESP-32-BLE-Scanner) ).



Notes
* If you have not adjusted config.json and added your wifi credentials, then when you first fire up the esp32, it will be in AP mode. Connect to the wifi AP by the name of 'ESP32 BLE Scanner' and then in your browser connect to http://192.168.4.1  You can then change the settings on the config page accordingly.
* The bluetooth scanning will only work once there is an active MQTT connection set
