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

class DCPwifi {
public:
    DCPwifi();

    void setupWiFiModule(const char* stationID);
};

#endif