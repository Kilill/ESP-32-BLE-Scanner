#include <WiFi.h>
#include "fileStructs.h"

WiFiClient espClient;

void startWifiClient(Config &config);
void setupAPmode(Config &config);

void setupWIFI(Config &config){
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.persistent(false);

  if (!String(config.ssid).length()){
     Serial.println("No wifi credentials found, setting up AP mode.");
     setupAPmode(config);   
  }else{
     Serial.println("Setting up WIFI access.");
     startWifiClient(config);
  }
}


void setupAPmode(Config &config){
  WiFi.disconnect(true);             
  WiFi.mode(WIFI_OFF);              
  delay(1000);                       
  WiFi.persistent(false);  
  boolean LWB2 = WiFi.softAPConfig(IPAddress(192, 168,4,1), IPAddress(192,168,4,2), IPAddress(255, 255, 255, 0));  
  if ( !LWB2 ) { 
      Serial.print(F("\n###ERR: softAPConfig failed with SSID: "));  
      Serial.print (String(config.hostname));  
  }
  delay(1000);     
  WiFi.mode(WIFI_AP);
  delay(1000);  
  WiFi.persistent(false); 
  boolean LWB1 = WiFi.softAP(config.hostname);
  if ( LWB1 ) {  
      Serial.println("Connect to the WIFI hotspot: \""+String(config.hostname)+"\"");
      Serial.println("Then connect your browser to http://"+WiFi.softAPIP().toString());        
  }else{  
      Serial.print(F("\n### Start AccessPoint failed!"));  
  }  
}


void startWifiClient(Config &config){
  delay(10);
  // First shut down AP
  WiFi.enableAP(false);
  WiFi.softAPdisconnect(true);
  // Set it to auto connect and reconnect
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to "+String(config.ssid));

  if (WiFi.status() != WL_CONNECTED) { 
    WiFi.begin(config.ssid, config.password); 
    WiFi.hostname(config.hostname);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("Connect your browser to http://");
  Serial.println(WiFi.localIP());
}
