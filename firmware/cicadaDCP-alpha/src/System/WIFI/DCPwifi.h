/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

#ifndef DCPwifi_h
#define DCPwifi_h
#include <WiFi.h>
#include <esp_wifi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "../DCPSystem.h"

class DCPwifi {
public:
    DCPwifi();

    boolean setupWiFiModule();
    void setupNTP();
    String getNetworkDate();
    int32_t getNetworkEpoch();
    boolean isConnected();
    IPAddress getLocalIP();
    String getSignalQuality();
    int getRSSIasQuality(int RSSI);
    void deleteWifiCredentials();
    void setSSID(String ssid);
    void setPWD(String pwd);
    String scanNetworks();
};

#endif