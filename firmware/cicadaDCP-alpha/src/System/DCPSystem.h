/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

#ifndef DCPSystem_h
#define DCPSystem_h

#include <FS.h> // FS must be the first
#include <esp_task_wdt.h>
#include "../CicadaWizard/CicadaWizard.h"
#include "../PINS_IO.h"
#include "SerialCommands/DCPSerialCommands.h"
#include "SDCard/DCPSDCard.h"
#include "SPIFFS/SPIFFSManager.h"
#include "WIFI/DCPwifi.h"
#include "LEDs/DCPLeds.h"
#include "SIM800/DCPSIM800.h"
#include "RTC/DCPRTC.h"
#include "MQTT/DCPMQTT.h"
#include "SelfUpdate/DCPSelfUpdate.h"
#include "../Sensors/DHT/DCPDht.h"
#include "../Sensors/RAINGauge/DCPRainGauge.h"
#include "../Sensors/VOLTAGE/DCPVoltage.h"

#define CIC_DEBUG_ENABLED true

/******************************************************************************/
/******************************************************************************/
#define SERIAL_ATTEMPTS 3
#define SERIAL_ATTEMPTS_DELAY 100

boolean takeSerialMutex();
void giveSerialMutex();

#define SIM_ATTEMPTS 5
#define SIM_ATTEMPTS_DELAY 100

boolean takeCommunicationMutex(String x);
void giveCommunicationMutex(String x);

boolean getInUpdate();
void setInUpdate(boolean inup);

boolean getInDownload();
void setInDownload(boolean inDown);

void updateAllSlots();
/******************************************************************************/
/******************************************************************************/
void CIC_DEBUG_SETUP(int baudrate);
void CIC_DEBUG_HEADER(String text);
void CIC_DEBUG_(String text);
void CIC_DEBUG(String text);
void CIC_DEBUGWRITE(char cc);

class DCPSystem {
public:
    DCPSystem();
    void readSerialCommands(xTaskHandle coreTask);
    void preInitSystem();
    boolean initCommunication(boolean startPromptOnFail = true);
    void setupWizard(xTaskHandle coreTask);
    void initSystem(xTaskHandle coreTask);
    void checkAPWizard(xTaskHandle coreTask);
    void updateStatus();
    void readSensors();
    void printNowDate();
    void initMQTT();
    void transmiteData();
    void taskTransmitLoop();
    String IpAddress2String(const IPAddress& ipAddress);
    void updateCommunicationStatus();
    void updateCommunicationSignal();
    void initSelfUpdate();

private:
    void setupTimeout();
    void initStationID();
    void initStationName();
    void initStationPWD();
    void initFirmwareVersion();
    void initStationCoordinates();
    void initBucketVolume();
    void initSlotStoreMetadata();
    void initSensorsConfig();
    String getFwmVersion();
    String getSSIDAP();
    void printConfiguration();
    void storeMetadados();
    int nextTimeSlotToSaveMetadata;
    boolean inTransmitionData;
    void nextSlotToSaveMetadata();
    void updateNextSlotMetadados();
    boolean onTimeToSaveMetadata();
    boolean networkFailureBoot();
    void clearSerialInput();

};

#endif