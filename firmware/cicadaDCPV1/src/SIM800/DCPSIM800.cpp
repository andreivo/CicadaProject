/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */
#include "DCPSIM800.h"

// Time to try
#define SIM_CONN_COUNTER 3
// Delay to connect to WiFi (WIFI_CONN_DELAY X WIFI_CONN_COUNTER = time to access point mode)
#define SIM_CONN_DELAY 500

/**
 * File system directories and variables
 */
SPIFFSManager simSpiffsManager;

DCPLeds simDCPLeds;

//Canal serial que vamos usar para comunicarmos com o modem. Utilize sempre 1
HardwareSerial SerialGSM(1);
TinyGsm modemGSM(SerialGSM);
TinyGsmClient gsmClient(modemGSM);

DCPSIM800::DCPSIM800() {
}

void DCPSIM800::turnOn() {
    digitalWrite(PIN_MODEM_TURNON, HIGH);
    simDCPLeds.redBlink(40, 500);
}

void DCPSIM800::turnOff() {
    digitalWrite(PIN_MODEM_TURNON, LOW);
}

/**
 * Setup WiFi module
 */
boolean DCPSIM800::setupSIM800Module() {
    CIC_DEBUG_HEADER(F("SETUP SIM800 MODULE"));

    // Get MQTT Host
    String apn = simSpiffsManager.getSettings("SIM Carrier APN", DIR_SIMCARD_APN, true);
    // Get MQTT User
    String user = simSpiffsManager.getSettings("SIM Carrier APN User", DIR_SIMCARD_USER, false);
    // Get MQTT Password
    String pwd = simSpiffsManager.getSettings("SIM  Carrier APN Pwd", DIR_SIMCARD_PWD, true);

    if (apn != "") {

        turnOn();
        int count = 0;

        while (count++ < SIM_CONN_COUNTER) {

            CIC_DEBUG(F("Setup GSM..."));
            boolean conn = true;

            //Inicializamos a serial onde está o modem
            SerialGSM.begin(9600, SERIAL_8N1, PIN_MODEM_RX, PIN_MODEM_TX, false);
            simDCPLeds.redBlink(6, 500);

            //Mostra informação sobre o modem
            CIC_DEBUG_(F("Modem Info: "));
            CIC_DEBUG(modemGSM.getModemInfo());

            //Inicializa o modem
            if (!modemGSM.restart()) {
                CIC_DEBUG(F("Restarting GSM Modem failed"));
                delay(SIM_CONN_DELAY);
                conn = false;
            }

            //Espera pela rede
            if (conn) {
                if (!modemGSM.waitForNetwork()) {
                    CIC_DEBUG(F("Failed to connect to network"));
                    delay(SIM_CONN_DELAY);
                    conn = false;
                }
            }

            //Conecta à rede gprs (APN, usuário, senha)
            if (conn) {
                if (!modemGSM.gprsConnect(apn.c_str(), user.c_str(), pwd.c_str())) {

                    CIC_DEBUG(F("GPRS Connection Failed"));
                    delay(SIM_CONN_DELAY);
                    conn = false;
                }
            }

            if (conn) {
                CIC_DEBUG(F("Setup GSM Success"));
                simDCPLeds.redTurnOff();
                simDCPLeds.greenBlink(20);
                simDCPLeds.greenTurnOff();
                return true;
            }
        }
    } else {
        CIC_DEBUG("No APN credentials for SIM card");
        turnOff();
        return false;
    }
    turnOff();
    return false;
}

char* DCPSIM800::getNetworkDate() {
    modemGSM.getGSMDateTime(DATE_FULL)
    if (WiFi.status() == WL_CONNECTED) {
        //Recupera os dados de data e horário usando o client NTP
        char* strDate = (char*) ntpClient.getFormattedDate().c_str();
        return strDate;
    }
    return "";
}