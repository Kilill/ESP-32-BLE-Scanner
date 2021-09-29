struct Config {
  char ssid[33];
  char password[33];
  char hostname[50];
  char room[33];
  char mqttServer[100];
  char mqttPort[6];
  char mqttUser[33];
  char mqttPassword[33];
  char mqttScanTopicPrefix[100];
  char mqttStateTopicPrefix[100];
};

struct Devices{
    char device_uuid1[50];
    char device_name1[50];
    char device_uuid2[50];
    char device_name2[50];
    char device_uuid3[50];
    char device_name3[50];
};