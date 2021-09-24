struct Config {
  char ssid[33];
  char password[33];
  char hostname[50];
  char room[33];
  char mqttServer[16];
  char mqttPort[6];
  char mqttUser[33];
  char mqttPassword[33];
};

struct Devices{
    char device_uuid1[37];
    char device_name1[47];
    char device_uuid2[37];
    char device_name2[47];
    char device_uuid3[37];
    char device_name3[47];
};