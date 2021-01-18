/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */
#include "DCPSelfUpdate.h"

DCPRTC sUpdateRTC;
DCPwifi sUpdateWifi;

DCPSDCard sUpdateSdCard;
//DCPLeds mqttLeds;

//// Initialize DCPSIM800
DCPSIM800 sUpdateSIM800;

DCPSelfUpdate::DCPSelfUpdate() {

}

/**
 * Setup WiFi module
 */
boolean DCPSelfUpdate::setupSelfUpdate(int timeToCheck, String _host, String _hostpath, int _port, String _firmwareDate, String _stationName) {
    CIC_DEBUG_HEADER(F("SETUP SELF UPDATE MODULE"));

    if (_host == "") {
        nexTimeToCheck = 0;
        CIC_DEBUG(F("Self update disable"));
        return true;
    }

    TIME_TO_CHECK = timeToCheck;
    CIC_DEBUG_(F("Time to check update: "));
    CIC_DEBUG_(TIME_TO_CHECK);
    CIC_DEBUG(F(":00 hour"));

    host = _host;
    hostpath = _hostpath;
    port = _port;
    firmwareDate = sUpdateRTC.stringToTime(_firmwareDate);
    stationName = _stationName;
    nextTimeToCheck();
    return true;
}

boolean DCPSelfUpdate::onTimeToCheck() {
    time_t ttNow = sUpdateRTC.nowEpoch();
    return ((ttNow >= nexTimeToCheck) && (ttNow <= nexTimeToCheck + 600));
}

void DCPSelfUpdate::nextTimeToCheck() {
    int year = sUpdateRTC.now("%Y").toInt();
    int month = sUpdateRTC.now("%m").toInt();
    int day = sUpdateRTC.now("%d").toInt();
    String time = String(year) + "-" + String(month) + "-" + String(day) + "T" + TIME_TO_CHECK + ":00:00Z";
    time_t ttToCheck = sUpdateRTC.stringToTime(time);
    time_t ttNow = sUpdateRTC.nowEpoch();

    if (ttToCheck <= ttNow) {
        ttToCheck = ttToCheck + (60 * 60 * 24);
        //ttToCheck = ttToCheck + (120);
    }

    nexTimeToCheck = ttToCheck;
    CIC_DEBUG_(F("Next time to check update: "));
    CIC_DEBUG(sUpdateRTC.printTime(nexTimeToCheck));
}

boolean DCPSelfUpdate::updateFirmware(boolean force) {
    if (onTimeToCheck() || force) {
        CIC_DEBUG_HEADER(F("STARTING CHECK UPDATE"));
        if (sUpdateSdCard.deleteUpdate()) {
            //checks the update for all stations and the update specifies for this station
            if (!prepareUpdate("")) {
                nextTimeToCheck();
                setInUpdate(false);
                return false;
            }
            if (!prepareUpdate(stationName)) {
                nextTimeToCheck();
                setInUpdate(false);
                return false;
            }

            nextTimeToCheck();
            setInUpdate(false);
            return true;
        } else {
            nextTimeToCheck();
            setInUpdate(false);
            return false;
        }
    } else {
        setInUpdate(false);
        return true;
    }
}

boolean DCPSelfUpdate::prepareUpdate(String subPath) {
    int attempts = 0;
    boolean result = false;
    while (attempts <= SU_ATTEMPTS) {
        String newHostpath = hostpath;
        if (subPath != "") {
            newHostpath += "/" + subPath;
        }

        //Prepare download
        Client* client;

        if (sUpdateWifi.isConnected()) {
            WiFiClient clientWifi;
            client = &clientWifi;
        } else if (sUpdateSIM800.isConnected()) {
            TinyGsmSim800 modemGSM = sUpdateSIM800.getModem();
            TinyGsmClient clientGSM(modemGSM);
            delay(10);
            client = &clientGSM;
        } else {
            result = false;
        }

        if (downloadFile(client, host, newHostpath, port, "firmware.version", "update/firmware.version")) {
            String sUpdateFirmwareDate = sUpdateSdCard.readFile("update/firmware.version");
            CIC_DEBUG_("New Firmware date: ");
            CIC_DEBUG(sUpdateFirmwareDate);
            time_t timeUpdatefirmware = sUpdateRTC.stringToTime(sUpdateFirmwareDate);
            //Checks whether the host firmware date is greater than the current firmware date.
            if (timeUpdatefirmware > firmwareDate) {
                if (downloadFile(client, host, newHostpath, port, "firmware.bin", "update/firmware.bin")) {
                    setInUpdate(true);
                    delay(2000);
                    startUpdate("update/firmware.bin");
                } else {
                    result = false;
                }
            } else {
                result = true;
            }
        } else {
            result = false;
        }
        if (!result) {
            attempts = attempts + 1;
            delay(SU_ATTEMPTS_DELAY);
        } else {
            return true;
        }
    }
}

boolean DCPSelfUpdate::startUpdate(String filename) {
    CIC_DEBUG_HEADER(F("STARTING FIRMWARE UPDATE"));

    int attempts = 0;
    while (attempts <= SU_ATTEMPTS) {
        if (sUpdateSdCard.takeSDMutex("startUpdate")) {
            uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            if (!Update.begin(maxSketchSpace, U_FLASH)) { //start with max available size
                CIC_DEBUG("Error on update!");
                Update.printError(Serial);
                sUpdateSdCard.giveSDMutex("startUpdate");
                return false;
            }

            //String result = "";
            File32 myFile;
            if (myFile.open(filename.c_str(), O_RDONLY)) {
                // read from the file until there's nothing else in it:
                CIC_DEBUG(F("Update in progress. Please wait."));
                while (myFile.available()) {
                    //result = result + (char) myFile.read();
                    uint8_t ibuffer[128];
                    myFile.read((uint8_t *) ibuffer, 128);
                    Update.write(ibuffer, sizeof (ibuffer));
                    //Update Task Watchdog timer
                    esp_task_wdt_reset();
                }
                Serial.println(F(""));
                if (Update.end(true)) {
                    CIC_DEBUG(F("The firmware update was completed successfully. The system will restart in a few seconds."));
                } else {
                    CIC_DEBUG(F("The firmware update FAIL. The system will restart in a few seconds."));
                }
                // close the file:
                myFile.close();
                sUpdateSdCard.giveSDMutex("startUpdate");
                delay(2000);
                ESP.restart();
            } else {
                // if the file didn't open, print an error:
                Serial.print(F("Error opening: "));
                Serial.println(filename);
                sUpdateSdCard.giveSDMutex("startUpdate");
                return false;
            }
            return true;
        } else {
            CIC_DEBUGWL("Waiting to start update firmware...");
        }
        attempts = attempts + 1;
        vTaskDelay(pdMS_TO_TICKS(SU_ATTEMPTS_DELAY));
    }
}

String DCPSelfUpdate::getHeaderValue(String header, String headerName) {
    return header.substring(strlen(headerName.c_str()));
}

boolean DCPSelfUpdate::downloadFile(Client* client, String host, String hostPath, int port, String filename, String saveAs) {
    int attempts = 0;
    while (attempts <= SU_ATTEMPTS) {
        if (takeCommunicationMutex()) {
            long contentLength = 0;

            String bin = hostPath + "/" + filename;

            if (client->connect(host.c_str(), port)) {
                // Connection Succeed.
                // Fecthing the bin
                CIC_DEBUG_(F("Fetching: "));
                CIC_DEBUG(String(bin));

                // Get the contents of the bin file
                client->print(String("GET ") + bin + " HTTP/1.1\r\n" +
                        "Host: " + host + "\r\n" +
                        "Cache-Control: no-cache\r\n" +
                        "Connection: close\r\n\r\n");

                unsigned long timeout = millis();
                while (client->available() == 0) {
                    if (millis() - timeout > DOWNLOAD_TIMEOUT) {
                        CIC_DEBUG(F("Client Timeout !"));
                        client->stop();
                        giveCommunicationMutex();
                        return false;
                    }
                }

                while (client->available()) {
                    // read line till /n
                    String line = client->readStringUntil('\n');
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
                            CIC_DEBUG(F("Got a non 200 status code from server."));
                            CIC_DEBUG(line);
                            giveCommunicationMutex();
                            return false;
                        }
                    }

                    // extract headers here
                    // Start with content length
                    if (line.startsWith("Content-Length: ")) {
                        contentLength = atol((getHeaderValue(line, "Content-Length: ")).c_str());
                        CIC_DEBUG("Got " + String(contentLength) + " bytes from server");
                    }
                }
            } else {
                // Probably a choppy network?
                CIC_DEBUG("Connection to " + String(host) + " failed. Please check your setup");
                giveCommunicationMutex();
                return false;
            }

            // check contentLength and content type
            if (contentLength) {
                // create buffer for read
                uint8_t buff[128] = {0};

                // read all data from server
                CIC_DEBUG(F("Download in progress."));
                long readLength = 0;
                uint32_t clientReadStartTime = millis();
                uint32_t timeElapsed = millis();
                boolean writeFile = true;

                printPercent(readLength, contentLength, 0);

                while (readLength < contentLength) {

                    if (millis() - clientReadStartTime > DOWNLOAD_TIMEOUT) {
                        CIC_DEBUG(F("Connection timeout!"));
                        break;
                    }

                    // get available data size
                    size_t size = client->available();
                    if (size) {
                        // read up to 128 byte
                        int c = client->readBytes(buff, ((size > sizeof (buff)) ? sizeof (buff) : size));
                        writeFile = sUpdateSdCard.writeBinFile(saveAs, buff, c);

                        if (!writeFile) {
                            CIC_DEBUG_(F("Cannot save download file: "));
                            CIC_DEBUG(saveAs);
                            break;
                        }

                        readLength += c;
                        if (readLength % 1280 == 0) {
                            printPercent(readLength, contentLength, millis() - timeElapsed);
                        }
                        clientReadStartTime = millis();
                        //Update Task Watchdog timer
                        esp_task_wdt_reset();
                    }
                    delay(1);
                }

                client->stop();

                CIC_DEBUG_(F("Remote content-length: "));
                CIC_DEBUG_(contentLength);
                CIC_DEBUG(F(" bytes"));
                CIC_DEBUG_(F("Local content-length:  "));
                CIC_DEBUG_(readLength);
                CIC_DEBUG(F(" bytes"));
                if (readLength == contentLength) {
                    CIC_DEBUG(F("Download is DONE!"));
                    Serial.println(F(""));
                    giveCommunicationMutex();
                    return true;
                } else {
                    CIC_DEBUG(F("Download is FAIL!"));
                    Serial.println(F(""));
                    giveCommunicationMutex();
                    return false;
                }
            }
        } else {
            CIC_DEBUG(F("Waiting to downloadFile ..."));
        }
        attempts = attempts + 1;
        delay(SU_ATTEMPTS_DELAY);
    }
    return false;
}

void DCPSelfUpdate::printPercent(long readLength, long contentLength, uint32_t timeElapsed) {
    // If we know the total length
    if (contentLength != (long) - 1) {
        float perc = (100.0 * readLength) / contentLength;
        CIC_DEBUG_(perc);
        CIC_DEBUG_(F("%"));
        if (timeElapsed != 0) {
            int secRemaining = (((100 - perc) * timeElapsed) / perc) / 1000;
            int minRemaining = secRemaining / 60;
            CIC_DEBUG_(F(" - "));
            CIC_DEBUG_(minRemaining);
            CIC_DEBUG_(F(" minutes ("));
            CIC_DEBUG_(secRemaining);
            CIC_DEBUG(F(" seconds) remaining..."));
        } else {
            CIC_DEBUG(F(""));
        }
    } else {
        CIC_DEBUG(readLength);
    }
}
