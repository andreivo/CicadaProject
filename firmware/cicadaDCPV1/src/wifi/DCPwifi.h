/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#ifndef DCPwifi_h
#define DCPwifi_h
#include <WiFi.h>
#include <esp_wifi.h>
#include <NTPClient.h> //Biblioteca NTPClient modificada
#include <WiFiUdp.h> //Socket UDP
#include "../system/DCPSystem.h"

class DCPwifi {
public:
    DCPwifi();

    boolean setupWiFiModule();
    void setupNTP();
    char* getNetworkDate();
};

#endif