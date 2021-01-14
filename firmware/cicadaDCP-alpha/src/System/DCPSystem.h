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

boolean takeCommunicationMutex();
boolean takeCommunicationMutexWait();
void giveCommunicationMutex();
void giveCommunicationMutexWait();

void logEnable();
void logDisable();
void cic_log(String msg, boolean ln = true);
/******************************************************************************/
/******************************************************************************/

#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_(text) { int attempts = 0; while (attempts <= SERIAL_ATTEMPTS) { if (takeSerialMutex()) {Serial.print((text)); cic_log(String(text), false); giveSerialMutex(); break;} attempts = attempts+1;delay(SERIAL_ATTEMPTS_DELAY);}}
#else
#define CIC_DEBUG_(text) { cic_log(String(text), false);}
#endif

#if CIC_DEBUG_ENABLED
#define CIC_DEBUGWL_(text) { int attempts = 0; while (attempts <= SERIAL_ATTEMPTS) { if (takeSerialMutex()) {Serial.print((text)); giveSerialMutex(); break;} attempts = attempts+1;delay(SERIAL_ATTEMPTS_DELAY);}}
#else
#define CIC_DEBUGWL_(text) {}
#endif

#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_HEADER(text) { int attempts = 0; while (attempts <= SERIAL_ATTEMPTS) { if (takeSerialMutex()) { Serial.println(F("\n")); Serial.println((text)); Serial.println(F("===========================================")); cic_log(String(text)); cic_log(F("===========================================")); giveSerialMutex(); break; } attempts = attempts + 1; delay(SERIAL_ATTEMPTS_DELAY);}}
#else
#define CIC_DEBUG_HEADER(text) {cic_log(String(text)); cic_log(F("==========================================="));}
#endif

#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_HEADERWL(text) { int attempts = 0; while (attempts <= SERIAL_ATTEMPTS) { if (takeSerialMutex()) { Serial.println(F("\n")); Serial.println((text)); Serial.println(F("===========================================")); giveSerialMutex(); break; } attempts = attempts + 1; delay(SERIAL_ATTEMPTS_DELAY);}}
#else
#define CIC_DEBUG_HEADERWL(text) {}
#endif

#if CIC_DEBUG_ENABLED
#define CIC_DEBUG(text) { int attempts = 0; while (attempts <= SERIAL_ATTEMPTS) { if (takeSerialMutex()) { Serial.print((text)); Serial.print(F(" (Core: ")); Serial.print(xPortGetCoreID()); Serial.println(F(")")); cic_log(String(text) + " (Core: " + String(xPortGetCoreID()) + ")"); giveSerialMutex(); break; } attempts = attempts + 1; delay(SERIAL_ATTEMPTS_DELAY); }}
#else
#define CIC_DEBUG(text) {cic_log(String(text) + " (Core: " + String(xPortGetCoreID()) + ")");}
#endif

#if CIC_DEBUG_ENABLED
#define CIC_DEBUGWL(text) { int attempts = 0; while (attempts <= SERIAL_ATTEMPTS) { if (takeSerialMutex()) { Serial.print((text)); Serial.print(F(" (Core: ")); Serial.print(xPortGetCoreID()); Serial.println(F(")")); giveSerialMutex(); break; } attempts = attempts + 1; delay(SERIAL_ATTEMPTS_DELAY); }}
#else
#define CIC_DEBUGWL(text) {}
#endif

// Serial debug
#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_SETUP(baudrate) { Serial.begin( (baudrate) );  delay(200);}
#else
#define CIC_DEBUG_SETUP(baudrate) {}
#endif

class DCPSystem {
public:
    DCPSystem();
    void readSerialCommands(xTaskHandle coreTask);
    void preInitSystem();
    boolean initCommunication(boolean startPromptOnFail = true);
    void setupWizard(xTaskHandle coreTask);
    void initSystem();
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

private:
    void setupTimeoutWizard();
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
    void nextSlotToSaveMetadata();
    boolean onTimeToSaveMetadata();
    boolean networkFailureBoot();
    void clearSerialInput();

};

#endif