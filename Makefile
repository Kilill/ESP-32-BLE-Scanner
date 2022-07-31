all:  .pio/build/esp32dev/firmware.bin

.pio/build/esp32dev/firmware.bin:  src/*.cpp src/*.hpp
	pio run

# upload binary to board
up: 
	pio run -t upload

upt: 
	pio run -t upload
	miniterm /dev/ttyUSB0 115200

fs: .pio/build/esp32dev/spiffs.bin
	pio run -t buildfs

upfs: fs
	pio run -t uploadfs

upfst: upfs
	miniterm /dev/ttyUSB0 115200
	


.pio/build/esp32dev/spiffs.bin: data/*.json data/*.html data/*.js  data/*.png 

