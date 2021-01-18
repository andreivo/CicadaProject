/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */
#include "DCPSIM800.h"

// Time to try
#define SIM_CONN_COUNTER 10
// Delay to connect to WiFi (WIFI_CONN_DELAY X WIFI_CONN_COUNTER = time to access point mode)
#define SIM_CONN_DELAY 500

int CIC_REVALIDATE_CONN = 3; //Minutes

/**
 * File system directories and variables
 */
SPIFFSManager simSpiffsManager;

DCPLeds simDCPLeds;

DCPRTC simRTC;

//Canal serial que vamos usar para comunicarmos com o modem. Utilize sempre 1
HardwareSerial SerialGSM(1);
TinyGsm modemGSM(SerialGSM);

DCPSDCard simSdCard;

DCPSIM800::DCPSIM800() {
}

void DCPSIM800::turnOn() {
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutex()) {
            digitalWrite(PIN_MODEM_TURNON, HIGH);
            simDCPLeds.redBlink(40, 500);
            giveCommunicationMutex();
            break;
        } else {
            CIC_DEBUG(F("Waiting to modem turnOn ..."));
        }
        attempts = attempts + 1;
        delay(SIM_ATTEMPTS_DELAY);
    }
}

void DCPSIM800::turnOff() {
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutex()) {
            digitalWrite(PIN_MODEM_TURNON, LOW);
            giveCommunicationMutex();
            break;
        } else {
            CIC_DEBUG(F("Waiting to modem turnOff ..."));
        }
        attempts = attempts + 1;
        delay(SIM_ATTEMPTS_DELAY);
    }
}

/**
 * Setup WiFi module
 */
boolean DCPSIM800::setupSIM800Module() {
    CIC_DEBUG_HEADER(F("SETUP SIM800 MODULE"));
    nextSlotToRevalidateConn();
    enableRevalidate = false;

    // Get MQTT Host
    String apn = simSpiffsManager.getSettings(F("SIM Carrier APN"), DIR_SIMCARD_APN, true);
    // Get MQTT User
    String user = simSpiffsManager.getSettings(F("SIM Carrier APN User"), DIR_SIMCARD_USER, false);
    // Get MQTT Password
    String pwd = simSpiffsManager.getSettings(F("SIM  Carrier APN Pwd"), DIR_SIMCARD_PWD, true);

    if (apn != "") {
        int count = 0;

        while (count++ < SIM_CONN_COUNTER) {
            turnOn();
            CIC_DEBUG(F("Setup GSM..."));
            CIC_DEBUG(F("Please, wait for registration on the network. This operation takes quite some time."));
            boolean conn = true;

            //Inicializamos a serial onde está o modem
            SerialGSM.begin(9600, SERIAL_8N1, PIN_MODEM_RX, PIN_MODEM_TX, false);
            simDCPLeds.redBlink(60, 500);
            String csq = getSignalQuality();
            CIC_DEBUG_(F("Signal quality: "));
            CIC_DEBUG(csq);

            //Mostra informação sobre o modem
            CIC_DEBUG_(F("Modem Info: "));
            String mInfo = modemGSM.getModemInfo();
            if (!mInfo || mInfo == "") {
                CIC_DEBUG(F("FAIL"));
                conn = false;
            }
            CIC_DEBUG(mInfo);

            //Inicializa o modem
            if (conn) {
                if (!modemGSM.restart()) {
                    CIC_DEBUG(F("Restarting GSM Modem failed"));
                    delay(SIM_CONN_DELAY);
                    conn = false;
                } else {
                    CIC_DEBUG(F("Modem restart"));
                }
            }

            //Espera pela rede
            if (conn) {
                if (!modemGSM.waitForNetwork()) {
                    CIC_DEBUG(F("Failed to connect to network"));
                    delay(SIM_CONN_DELAY);
                    conn = false;
                } else {
                    CIC_DEBUG(F("Network Ok"));
                }
            }

            //Conecta à rede gprs (APN, usuário, senha)
            if (conn) {
                if (!modemGSM.gprsConnect(apn.c_str(), user.c_str(), pwd.c_str())) {
                    CIC_DEBUG(F("GPRS Connection Failed"));
                    delay(SIM_CONN_DELAY);
                    conn = false;
                } else {
                    CIC_DEBUG(F("GPRS Connection"));
                }
            }

            if (conn) {
                CIC_DEBUG(F("Setup GSM Success"));
                simDCPLeds.redTurnOff();
                simDCPLeds.greenBlink(20);
                simDCPLeds.greenTurnOff();
                enableRevalidate = true;
                return true;
            }
            simDCPLeds.redTurnOff();
            turnOff();
            CIC_DEBUG(F("The connection failed. A new connection will be made in 5 seconds."));
            delay(5000);
        }
    } else {
        CIC_DEBUG(F("No APN credentials for SIM card"));
        turnOff();
        return false;
    }
    turnOff();
    return false;
}

String DCPSIM800::getNetworkDate() {
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutex()) {
            if (modemGSM.isGprsConnected()) {
                int year = 0;
                int month = 0;
                int day = 0;
                int hour = 0;
                int min = 0;
                int sec = 0;
                float timezone = 0;

                simDCPLeds.redTurnOn();
                simDCPLeds.greenTurnOff();

                for (int8_t i = 10; i; i--) {
                    simDCPLeds.redBlink();
                    simDCPLeds.greenBlink();

                    if (modemGSM.getNetworkTime(&year, &month, &day, &hour, &min, &sec,
                            &timezone)) {
                        CIC_DEBUG_(F("Year: "));
                        CIC_DEBUG_(year);
                        CIC_DEBUG_(F(" Month: "));
                        CIC_DEBUG_(month);
                        CIC_DEBUG_(F(" Day: "));
                        CIC_DEBUG_(day);
                        CIC_DEBUG_(F(" Hour: "));
                        CIC_DEBUG_(hour);
                        CIC_DEBUG_(F(" Minute: "));
                        CIC_DEBUG_(min);
                        CIC_DEBUG_(F(" Second: "));
                        CIC_DEBUG_(sec);
                        CIC_DEBUG_(F(" Timezone: "));
                        CIC_DEBUG(timezone);

                        //Setting to universal Time
                        hour = hour + ((-1) * timezone);

                        String result = String(year) + "-" + String(month) + "-" + String(day) + "T" + String(hour) + ":" + String(min) + ":" + String(sec) + "Z";
                        simDCPLeds.redTurnOff();
                        simDCPLeds.greenTurnOff();
                        giveCommunicationMutex();
                        return result;
                    } else {
                        CIC_DEBUG(F("Couldn't get network time, retrying in 1s."));
                        delay(1000L);
                    }
                }
                giveCommunicationMutex();
                return "";
            }
            giveCommunicationMutex();
            return "";
        } else {
            CIC_DEBUG(F("Waiting to modem getNetworkDate ..."));
        }
        attempts = attempts + 1;
        delay(SIM_ATTEMPTS_DELAY);
    }
    return "";
}

TinyGsmSim800 DCPSIM800::getModem() {
    return modemGSM;
}

String DCPSIM800::getSimCCID() {
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutex()) {
            String result = modemGSM.getSimCCID();
            giveCommunicationMutex();
            return result;
        } else {
            CIC_DEBUG(F("Waiting to modem getSimCCID ..."));
        }
        attempts = attempts + 1;
        delay(SIM_ATTEMPTS_DELAY);
    }
    return "";
}

String DCPSIM800::getOperator() {
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutex()) {
            String result = modemGSM.getOperator();
            giveCommunicationMutex();
            return result;
        } else {
            CIC_DEBUG(F("Waiting to modem getOperator ..."));
        }
        attempts = attempts + 1;
        delay(SIM_ATTEMPTS_DELAY);
    }
    return "";
}

IPAddress DCPSIM800::getLocalIP() {
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutex()) {
            IPAddress result = modemGSM.localIP();
            giveCommunicationMutex();
            return result;
        } else {
            CIC_DEBUG(F("Waiting to modem getLocalIP ..."));
        }
        attempts = attempts + 1;
        delay(SIM_ATTEMPTS_DELAY);
    }

}

String DCPSIM800::getSignalQuality() {
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutex()) {
            String result = String(modemGSM.getSignalQuality());
            giveCommunicationMutex();
            return result;
        } else {
            CIC_DEBUG(F("Waiting to modem getSignalQuality ..."));
        }
        attempts = attempts + 1;
        delay(SIM_ATTEMPTS_DELAY);
    }
    return "";

}

String DCPSIM800::getIMEI() {
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutex()) {
            String result = String(modemGSM.getIMEI());
            giveCommunicationMutex();
            return result;
        } else {
            CIC_DEBUG(F("Waiting to modem getSignalQuality ..."));
        }
        attempts = attempts + 1;
        delay(SIM_ATTEMPTS_DELAY);
    }
    return "";

}

String DCPSIM800::getIMSI() {
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutex()) {
            String result = String(modemGSM.getIMSI());
            giveCommunicationMutex();
            return result;
        } else {
            CIC_DEBUG(F("Waiting to modem getSignalQuality ..."));
        }
        attempts = attempts + 1;
        delay(SIM_ATTEMPTS_DELAY);
    }
    return "";

}

boolean DCPSIM800::isConnected() {
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutexWait()) {
            boolean result = modemGSM.isGprsConnected();
            giveCommunicationMutex();
            return result;
        } else {
            CIC_DEBUG(F("Waiting to modem isConnected ..."));
        }
        attempts = attempts + 1;
        delay(SIM_ATTEMPTS_DELAY);
    }
    return false;
}

void DCPSIM800::nextSlotToRevalidateConn() {
    /*********************/
    int actualSeconds = simRTC.now("%M").toInt() + 1;

    int rSlot = actualSeconds % CIC_REVALIDATE_CONN;
    int iSlot = (int) (actualSeconds / CIC_REVALIDATE_CONN);

    if (rSlot > 0) {
        iSlot = iSlot + 1;
    }

    int nextSlot = iSlot*CIC_REVALIDATE_CONN;
    if (nextSlot >= 60) {
        nextSlot = nextSlot - 60;
    }
    nextTimeSlotToRevalidateConn = nextSlot;
}

boolean DCPSIM800::onTimeToRevalidateConn() {
    int actualSeconds = simRTC.now("%M").toInt();
    return actualSeconds == nextTimeSlotToRevalidateConn;
}

void DCPSIM800::revalidateConnection() {
    if (enableRevalidate) {
        if (onTimeToRevalidateConn()) {
            if (!isConnected()) {
                turnOff();
            }
            nextSlotToRevalidateConn();
        }
    }
}

void DCPSIM800::resetConfig() {
    simSpiffsManager.deleteSettings(F("SIM  Carrier APN Pwd"), DIR_SIMCARD_APN);
    simSpiffsManager.deleteSettings(F("SIM Carrier APN User"), DIR_SIMCARD_USER);
    simSpiffsManager.deleteSettings(F("SIM  Carrier APN Pwd"), DIR_SIMCARD_PWD);
}

void DCPSIM800::setAPN(String apn) {
    // Get SIM APN
    simSpiffsManager.saveSettings("SIM Carrier APN", DIR_SIMCARD_APN, FILE_SIMCARD_APN, apn);
}

void DCPSIM800::setUSER(String user) {
    // Get SIM User
    simSpiffsManager.saveSettings("SIM Carrier APN User", DIR_SIMCARD_USER, user);
}

void DCPSIM800::setPWD(String pwd) {
    // Get SIM Password
    simSpiffsManager.saveSettings("SIM  Carrier APN Pwd", DIR_SIMCARD_PWD, FILE_SIMCARD_PWD, pwd);
}

String DCPSIM800::sendAT(String comm) {
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutex()) {
            modemGSM.sendAT(comm);

            String res = "";
            if (modemGSM.waitResponse(10000L, res) != 1) {
                res = F("TIMEOUT");
            }
            res.trim();
            giveCommunicationMutex();
            return res;
        } else {
            CIC_DEBUG(F("Waiting to sendAT ..."));
        }
        attempts = attempts + 1;
        delay(SIM_ATTEMPTS_DELAY);
    }
    return "";
}

String DCPSIM800::getHeaderValue(String header, String headerName) {
    return header.substring(strlen(headerName.c_str()));
}

boolean DCPSIM800::downloadFile(String host, String hostPath, int port, String filename, String saveAs, boolean checkContentType) {
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutex()) {
            TinyGsmClient client(modemGSM);
            long contentLength = 0;
            bool isValidContentType = false;

            String bin = hostPath + "/" + filename;

            if (client.connect(host.c_str(), port)) {
                // Connection Succeed.
                // Fecthing the bin
                CIC_DEBUG_(F("Fetching: "));
                CIC_DEBUG(String(bin));

                // Get the contents of the bin file
                client.print(String("GET ") + bin + " HTTP/1.1\r\n" +
                        "Host: " + host + "\r\n" +
                        "Cache-Control: no-cache\r\n" +
                        "Connection: close\r\n\r\n");

                unsigned long timeout = millis();
                while (client.available() == 0) {
                    if (millis() - timeout > 10000) {
                        CIC_DEBUG(F("Client Timeout !"));
                        client.stop();
                        giveCommunicationMutex();
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

                    // Next, the content type
                    if (checkContentType) {
                        if (line.startsWith("Content-Type: ")) {
                            String contentType = getHeaderValue(line, "Content-Type: ");
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
                giveCommunicationMutex();
                return false;
            }

            // check contentLength and content type
            if (contentLength && isValidContentType) {
                // create buffer for read
                uint8_t buff[128] = {0};

                // read all data from server
                CIC_DEBUG(F("Download in progress."));
                long readLength = 0;
                uint32_t clientReadStartTime = millis();
                uint32_t clientReadTimeout = 10000;
                uint32_t timeElapsed = millis();
                boolean writeFile = true;

                printPercent(readLength, contentLength, 0);

                while (readLength < contentLength) {
                    if (!client.connected()) {
                        CIC_DEBUG(F("Connection is broken!"));
                        break;
                    }
                    if (millis() - clientReadStartTime > clientReadTimeout) {
                        CIC_DEBUG(F("Connection timeout!"));
                        break;
                    }

                    // get available data size
                    size_t size = client.available();
                    if (size) {
                        // read up to 128 byte
                        int c = client.readBytes(buff, ((size > sizeof (buff)) ? sizeof (buff) : size));
                        writeFile = simSdCard.writeBinFile(saveAs, buff, c);

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
                    }

                    //Update Task Watchdog timer
                    esp_task_wdt_reset();
                }

                client.stop();

                CIC_DEBUG_(F("Remote content-length: "));
                CIC_DEBUG_(contentLength);
                CIC_DEBUG(F(" bytes"));
                CIC_DEBUG_(F("Local content-length:  "));
                CIC_DEBUG_(readLength);
                CIC_DEBUG(F(" bytes"));
                if (readLength == contentLength) {
                    CIC_DEBUG(F("Download is DONE!\n"));
                    giveCommunicationMutex();
                    return true;
                } else {
                    CIC_DEBUG(F("Download is FAIL!\n"));
                    giveCommunicationMutex();
                    return false;
                }
            }
        } else {
            CIC_DEBUG(F("Waiting to downloadFile ..."));
        }
        attempts = attempts + 1;
        delay(SIM_ATTEMPTS_DELAY);
    }
    return false;
}
//boolean DCPSIM800::downloadFile(String host, String hostPath, int port, String filename, String saveAs, boolean checkContentType) {
//    TinyGsmClient client(modemGSM);
//    long contentLength = 0;
//    bool isValidContentType = false;
//
//    String bin = hostPath + "/" + filename;
//
//    if (client.connect(host.c_str(), port)) {
//        // Connection Succeed.
//        // Fecthing the bin
//        CIC_DEBUG_(F("Fetching: "));
//        CIC_DEBUG(String(filename));
//
//        // Get the contents of the bin file
//        client.print(String("GET ") + bin + " HTTP/1.1\r\n" +
//                "Host: " + host + "\r\n" +
//                "Cache-Control: no-cache\r\n" +
//                "Connection: close\r\n\r\n");
//
//        unsigned long timeout = millis();
//        while (client.available() == 0) {
//            if (millis() - timeout > 5000) {
//                CIC_DEBUG(F("Client Timeout !"));
//                client.stop();
//                return false;
//            }
//        }
//
//        while (client.available()) {
//            // read line till /n
//            String line = client.readStringUntil('\n');
//            // remove space, to check if the line is end of headers
//            line.trim();
//
//            // if the the line is empty,
//            // this is end of headers
//            // break the while and feed the
//            // remaining `client` to the
//            // Update.writeStream();
//            if (!line.length()) {
//                //headers ended
//                break; // and get the OTA started
//            }
//
//            // Check if the HTTP Response is 200
//            // else break and Exit Update
//            if (line.startsWith("HTTP/1.1")) {
//                if (line.indexOf("200") < 0) {
//                    CIC_DEBUG(F("Got a non 200 status code from server."));
//                    CIC_DEBUG(line);
//                    return false;
//                }
//            }
//
//            // extract headers here
//            // Start with content length
//            if (line.startsWith("Content-Length: ")) {
//                contentLength = atol((getHeaderValue(line, "Content-Length: ")).c_str());
//                CIC_DEBUG("Got " + String(contentLength) + " bytes from server");
//            }
//
//            // Next, the content type
//            if (checkContentType) {
//                if (line.startsWith("Content-Type: ")) {
//                    String contentType = getHeaderValue(line, "Content-Type: ");
//                    if (contentType == "application/octet-stream") {
//                        isValidContentType = true;
//                    }
//                }
//            } else {
//                isValidContentType = true;
//            }
//        }
//    } else {
//        // Probably a choppy network?
//        CIC_DEBUG("Connection to " + String(host) + " failed. Please check your setup");
//        return false;
//    }
//
//    // check contentLength and content type
//    if (contentLength && isValidContentType) {
//        // create buffer for read
//        uint8_t buff[128] = {0};
//        File32 myFile;
//
//        // read all data from server
//        CIC_DEBUG(F("Download in progress."));
//        long readLength = 0;
//        uint32_t clientReadStartTime = millis();
//        uint32_t clientReadTimeout = 10000;
//        uint32_t timeElapsed = millis();
//
//        if (myFile.open(saveAs.c_str(), O_RDWR | O_CREAT)) {
//            printPercent(readLength, contentLength, millis() - timeElapsed);
//
//            while (readLength < contentLength) {
//                if (!client.connected()) {
//                    CIC_DEBUG(F("Connections is broken!"));
//                    break;
//                }
//                if (millis() - clientReadStartTime > clientReadTimeout) {
//                    CIC_DEBUG(F("Connection timeout"));
//                    break;
//                }
//
//                // get available data size
//                size_t size = client.available();
//                if (size) {
//                    // read up to 128 byte
//                    int c = client.readBytes(buff, ((size > sizeof (buff)) ? sizeof (buff) : size));
//                    myFile.write(buff, c);
//
//                    readLength += c;
//                    if (readLength % 1280 == 0) {
//                        printPercent(readLength, contentLength, millis() - timeElapsed);
//                    }
//                    clientReadStartTime = millis();
//                }
//
//                //Update Task Watchdog timer
//                esp_task_wdt_reset();
//            }
//        }
//        client.stop();
//        myFile.close();
//
//        CIC_DEBUG_(F("Remote content-length: "));
//        CIC_DEBUG_(contentLength);
//        CIC_DEBUG(F(" bytes"));
//        CIC_DEBUG_(F("Local content-length:  "));
//        CIC_DEBUG_(readLength);
//        CIC_DEBUG(F(" bytes"));
//        if (readLength == contentLength) {
//            CIC_DEBUG(F("Download is DONE!\n"));
//            return true;
//        } else {
//            CIC_DEBUG(F("Download is FAIL!\n"));
//            return false;
//        }
//    }
//}

void DCPSIM800::printPercent(long readLength, long contentLength, uint32_t timeElapsed) {
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