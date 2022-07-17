all: src/*.cpp src/*.hpp
	pio run

up: 
	pio run -t upload

upt: 
	pio run -t upload
	miniterm /dev/ttyUSB0 115200

upfs: filesystem
	pio run -t uploadfs

filesystem: .pio/build/esp32dev/spiffs.bin
	pio run -t buildfs

.pio/build/esp32dev/spiffs.bin: data/*

