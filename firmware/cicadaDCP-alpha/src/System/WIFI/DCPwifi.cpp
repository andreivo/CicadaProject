/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */
#include "DCPwifi.h"

// Time to try
#define WIFI_CONN_COUNTER 120
// Delay to connect to WiFi (WIFI_CONN_DELAY X WIFI_CONN_COUNTER = time to access point mode)
#define WIFI_CONN_DELAY 1000

#define timeZone 0

//Socket UDP que a lib utiliza para recuperar dados sobre o horário
WiFiUDP udp;

//Objeto responsável por recuperar dados sobre horário
NTPClient ntpClient(
        udp, //socket udp
        "0.br.pool.ntp.org", //URL do servwer NTP
        timeZone * 3600, //Deslocamento do horário em relacão ao GMT 0
        60000); //Intervalo entre verificações online

/**
 * File system directories and variables
 */
SPIFFSManager wifiSpiffsManager;

DCPLeds wifiDCPLeds;

DCPwifi::DCPwifi() {
}

/**
 * Setup WiFi module
 */
boolean DCPwifi::setupWiFiModule() {

    CIC_DEBUG_HEADER(F("SETUP WIFI MODULE"));

    // Get MQTT Host
    String ssid = wifiSpiffsManager.getSettings("SSID", DIR_WIFI_SSID, false);
    // Get MQTT User
    String pwd = wifiSpiffsManager.getSettings("Password", DIR_WIFI_PWD, false);

    if (ssid != "") {
        CIC_DEBUG(F("Using last saved values, should be faster"));
        int count = 0;

        while (count++ < WIFI_CONN_COUNTER) {
            CIC_DEBUG_(F("Connection attempt: "));
            CIC_DEBUG(count);
            wifiDCPLeds.redBlink();

            WiFi.begin(ssid.c_str(), pwd.c_str());
            if (WiFi.status() == WL_CONNECTED) {
                CIC_DEBUG(F("CONNECTED - Wi-Fi connection successful established."));
                setupNTP();
                wifiDCPLeds.redTurnOff();
                wifiDCPLeds.greenBlink(20);
                wifiDCPLeds.greenTurnOff();
                return true;
            } else if (WiFi.status() == WL_NO_SSID_AVAIL) {
                CIC_DEBUG(F("NO SSID AVAILABLE - Configured SSID cannot be reached."));
            } else if (WiFi.status() == WL_CONNECT_FAILED) {
                CIC_DEBUG(F("CONNECT FAILED - Password is incorrect."));
            } else if (WiFi.status() == WL_IDLE_STATUS) {
                CIC_DEBUG(F("IDLE STATUS - Wi-Fi is in process of changing between statuses."));
            } else if (WiFi.status() == WL_DISCONNECTED) {
                CIC_DEBUG(F("DISCONNECTED - Wi-Fi module is not configured in station mode."));
            }
            delay(WIFI_CONN_DELAY);
        }
    } else {
        CIC_DEBUG("No saved credentials");
        return false;
    }

    return false;
}

void DCPwifi::setupNTP() {

    CIC_DEBUG_HEADER(F("SETUP NTP Client"));

    //Inicializa o client NTP
    ntpClient.begin();

    //Espera pelo primeiro update online
    wifiDCPLeds.redTurnOn();
    wifiDCPLeds.greenTurnOff();
    CIC_DEBUG(F("Waiting for first update"));
    while (!ntpClient.update()) {
        wifiDCPLeds.redBlink();
        wifiDCPLeds.greenBlink();
        CIC_DEBUG_(".");
        ntpClient.forceUpdate();
        delay(2000);
    }

    CIC_DEBUG("");
    CIC_DEBUG("NTP First Update Complete");
}

String DCPwifi::getNetworkDate() {
    if (WiFi.status() == WL_CONNECTED) {
        //Recupera os dados de data e horário usando o client NTP
        String strDate = ntpClient.getFormattedDate();
        CIC_DEBUG_(F("NTP DateTime: "));
        CIC_DEBUG(strDate);
        return strDate;
    }
    return "";
}

int32_t DCPwifi::getNetworkEpoch() {
    if (WiFi.status() == WL_CONNECTED) {
        //Recupera os dados de data e horário usando o client NTP
        return ntpClient.getEpochTime();
    }
    return 0;
}

boolean DCPwifi::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress DCPwifi::getLocalIP() {
    return WiFi.localIP();
}

String DCPwifi::getSignalQuality() {
    int quality = getRSSIasQuality(WiFi.RSSI());
    return String(quality);
}

int DCPwifi::getRSSIasQuality(int RSSI) {
    int quality = 0;

    if (RSSI <= -100) {
        quality = 0;
    } else if (RSSI >= -50) {
        quality = 100;
    } else {

        quality = 2 * (RSSI + 100);
    }
    return quality;
}