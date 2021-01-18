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

DCPSDCard wifiSdCard;

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

void DCPwifi::deleteWifiCredentials() {

    wifiSpiffsManager.FSDeleteFiles(DIR_WIFI_SSID);
    wifiSpiffsManager.FSDeleteFiles(DIR_WIFI_PWD);

    WiFi.disconnect(true); // still not erasing the ssid/pw. Will happily reconnect on next start
    WiFi.begin("0", "0"); // adding this effectively seems to erase the previous stored SSID/PW
    delay(1000);
}

void DCPwifi::setSSID(String ssid) {
    wifiSpiffsManager.saveSettings("SSID", DIR_WIFI_SSID, ssid);
}

void DCPwifi::setPWD(String pwd) {
    wifiSpiffsManager.saveSettings("Password", DIR_WIFI_PWD, pwd);
}

String DCPwifi::scanNetworks() {
    String result = "\n";

    if (!isConnected()) {
        WiFi.disconnect();
    }

    ///////////////
    int n = WiFi.scanNetworks();
    if (n == 0) {
        return F("No networks found. Refresh to scan again.");
    } else {
        //sort networks
        int indices[n];
        for (int i = 0; i < n; i++) {
            indices[i] = i;
        }

        // RSSI SORT
        // old sort
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
                    std::swap(indices[i], indices[j]);
                }
            }
        }

        // remove duplicates ( must be RSSI sorted )
        String cssid;
        for (int i = 0; i < n; i++) {
            if (indices[i] == -1) continue;
            cssid = WiFi.SSID(indices[i]);
            for (int j = i + 1; j < n; j++) {
                if (cssid == WiFi.SSID(indices[j])) {
                    indices[j] = -1; // set dup aps to index -1
                }
            }
        }

        //display networks in page
        for (int i = 0; i < n; i++) {
            if (indices[i] == -1) continue; // skip dups
            int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));
            String rssiQ;
            rssiQ += quality;
            result = result + WiFi.SSID(indices[i]) + " (" + rssiQ + "%)";

            if (WiFi.encryptionType(indices[i]) != WIFI_AUTH_OPEN) {
                result = result + " (auth)";
            }
            result = result + "\n";
        }
    }
    return result;
}

// Utility to extract header value from headers

String DCPwifi::getHeaderValue(String header, String headerName) {
    return header.substring(strlen(headerName.c_str()));
}

boolean DCPwifi::downloadFile(String host, String hostPath, int port, String filename, String saveAs, boolean checkContentType) {
    WiFiClient client;
    long contentLength = 0;
    bool isValidContentType = false;

    String bin = hostPath + "/" + filename;

    if (client.connect(host.c_str(), port)) {
        // Connection Succeed.
        // Fecthing the bin
        CIC_DEBUG("Fetching: " + String(bin));

        // Get the contents of the bin file
        client.print(String("GET ") + bin + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "Cache-Control: no-cache\r\n" +
                "Connection: close\r\n\r\n");

        // Check what is being sent
        CIC_DEBUG_(String("GET ") + bin + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "Cache-Control: no-cache\r\n" +
                "Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (client.available() == 0) {
            if (millis() - timeout > 50000) {
                CIC_DEBUG("Client Timeout !");
                client.stop();
                return false;
            }
        }

        while (client.available()) {
            // read line till /n
            String line = client.readStringUntil('\n');
            // remove space, to check if the line is end of headers
            line.trim();

            // if the the line is empty,
            // this is end of headers
            // break the while and feed the
            // remaining `client` to the
            // Update.writeStream();
            if (!line.length()) {
                //headers ended
                break; // and get the OTA started
            }

            // Check if the HTTP Response is 200
            // else break and Exit Update
            if (line.startsWith("HTTP/1.1")) {
                if (line.indexOf("200") < 0) {
                    CIC_DEBUG("Got a non 200 status code from server.");
                    CIC_DEBUG(line);
                    return false;
                }
            }

            // extract headers here
            // Start with content length
            if (line.startsWith("Content-Length: ")) {
                contentLength = atol((getHeaderValue(line, "Content-Length: ")).c_str());
                CIC_DEBUG("Got " + String(contentLength) + " bytes from server");
            }

            // Next, the content type
            if (checkContentType) {
                if (line.startsWith("Content-Type: ")) {
                    String contentType = getHeaderValue(line, "Content-Type: ");
                    CIC_DEBUG("Got " + contentType + " payload.");
                    if (contentType == "application/octet-stream") {
                        isValidContentType = true;
                    }
                }
            } else {
                isValidContentType = true;
            }
        }
    } else {
        // Probably a choppy network?
        CIC_DEBUG("Connection to " + String(host) + " failed. Please check your setup");
        return false;
    }

    // Check what is the contentLength and if content type is `application/octet-stream`
    CIC_DEBUG("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));

    // check contentLength and content type
    if (contentLength && isValidContentType) {
        // create buffer for read
        uint8_t buff[128] = {0};
        File32 myFile;

        // read all data from server
        CIC_DEBUG(F("Download in progress."));
        if (myFile.open(saveAs.c_str(), O_RDWR | O_CREAT)) {
            while (contentLength > 0 || contentLength == -1) {
                // get available data size
                size_t size = client.available();

                if (size) {
                    // read up to 128 byte
                    int c = client.readBytes(buff, ((size > sizeof (buff)) ? sizeof (buff) : size));
                    myFile.write(buff, c);

                    if (contentLength > 0) {
                        contentLength -= c;
                    }
                }
                delay(1);
                //Update Task Watchdog timer
                esp_task_wdt_reset();
            }
        }
        CIC_DEBUG(F(""));
        myFile.close();
        CIC_DEBUG("Download is done!");
        return true;
    }
}