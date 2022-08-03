all: firmware fs

# firmware image
firmware: .pio/build/esp32dev/firmware.bin

.pio/build/esp32dev/firmware.bin:  src/*.cpp src/*.hpp
	pio run

# upload binary to board
up:
	pio run -t upload

#upload binary and start miniterm after
upt:
	pio run -t upload
	miniterm /dev/ttyUSB0 115200

# file system image
fs: .pio/build/esp32dev/spiffs.bin

.pio/build/esp32dev/spiffs.bin: data data/*.json data/*.html data/*.js  data/*.png 
	pio run -t buildfs

#upload filesystem image
upfs: fs
	pio run -t uploadfs

#upload filesystema and start miniterm
upfst: upfs
	miniterm /dev/ttyUSB0 115200

term:
	miniterm /dev/ttyUSB0 115200
	

