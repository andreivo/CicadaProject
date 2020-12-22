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

                String result = String(year) + "-" + String(month) + "-" + String(day) + "T" + String(hour) + ":" + String(min) + ":" + String(sec) + "Z";

                return (char*) result.c_str();
            } else {
                CIC_DEBUG(F("Couldn't get network time, retrying in 1s."));
                delay(1000L);
            }
        }

        return "";
    }
    return "";
}