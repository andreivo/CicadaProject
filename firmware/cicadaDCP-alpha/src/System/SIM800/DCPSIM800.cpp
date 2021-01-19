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
            CIC_DEBUG_(csq);
            CIC_DEBUG(F("%"));

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
            String result = String(getCSQasQuality(modemGSM.getSignalQuality()));
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

int DCPSIM800::getCSQasQuality(int CSQ) {
    int quality = 0;
    quality = (CSQ * 100) / 31;
    if (quality > 100) {
        quality = 0;
    }

    return quality;
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
        if (takeCommunicationMutex()) {
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
            int attempts = 0;
            while (attempts <= SIM_ATTEMPTS) {
                if (takeCommunicationMutex()) {
                    boolean result = modemGSM.isGprsConnected();
                    giveCommunicationMutex();
                    if (!result) {
                        turnOff();
                    }
                    nextSlotToRevalidateConn();
                }
                attempts = attempts + 1;
                delay(SIM_ATTEMPTS_DELAY);
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