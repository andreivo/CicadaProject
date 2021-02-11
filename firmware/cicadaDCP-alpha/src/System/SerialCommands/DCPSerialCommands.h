/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

#ifndef DCPSerialCommands_h
#define DCPSerialCommands_h

#include "../DCPSystem.h"
#include "../SelfUpdate/DCPSelfUpdate.h"

void IRAM_ATTR onTimeoutSerialWizard();

class DCPSerialCommands {
public:
    DCPSerialCommands();

    void initSerialCommands(String firmware, String firmwareDate);
    void readSerialCommands(xTaskHandle coreTask);
private:
    String FIRMWARE;
    String FIRMWARE_DATE;
    String getArguments(String data, int index, char separator = '-');
    float bytesConverter(float bytes, char prefix);
    void printCommands();
    void rebootComm();
    void timeComm(String serialCommand);
    void statusComm();
    void factoryresetComm(xTaskHandle coreTask);
    void sconfigComm();
    void wizardComm(xTaskHandle coreTask);
    void setupTimeoutWizard();
    void printSystemEnvironmentStatus();
    void wifiComm(String serialCommand);
    void printSystemWiFiStatus();
    void simComm(String serialCommand);
    void printSystemSimStatus();
    void weatherComm(String serialCommand);
    void printSystemWeathers();
    void lsComm(String serialCommand);
    void catComm(String serialCommand);
    void fsstatusComm();
    void clearSerialInput();
    String padL(int len, String inS);
    void forceUpdateComm(String serialCommand);
    void initSelfUpdate();
    void nextslottimeComm();

};

#endif
