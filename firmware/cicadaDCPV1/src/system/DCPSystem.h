/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#ifndef DCPSystem_h
#define DCPSystem_h

#include <FS.h> // FS must be the first
#include <esp_task_wdt.h>
#include "../SDCard/DCPSDCard.h"
#include "../CicadaWizard/CicadaWizard.h"
#include "../SPIFFS/SPIFFSManager.h"
#include "../DHT/DCPDht.h"
#include "../WIFI/DCPwifi.h"
#include "../PINS_IO.h"
#include "../LEDs/DCPLeds.h"
#include "../SIM800/DCPSIM800.h"
#include "../RTC/DCPRTC.h"
#include "../MQTT/DCPMQTT.h"
#include "../RAINGauge/DCPRainGauge.h"


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
/******************************************************************************/
/******************************************************************************/

#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_(text) { int attempts = 0; while (attempts <= SERIAL_ATTEMPTS) { if (takeSerialMutex()) {Serial.print((text)); giveSerialMutex(); break;} attempts = attempts+1;delay(SERIAL_ATTEMPTS_DELAY);}}
#else
#define CIC_DEBUG_(text) {}
#endif

#if CIC_DEBUG_ENABLED

#define CIC_DEBUG_HEADER(text) { int attempts = 0; while (attempts <= SERIAL_ATTEMPTS) { if (takeSerialMutex()) { Serial.println(F("\n")); Serial.println((text)); Serial.println(F("===========================================")); giveSerialMutex(); break; } attempts = attempts + 1; delay(SERIAL_ATTEMPTS_DELAY);}}
#else
#define CIC_DEBUG_HEADER(text) {}
#endif

#if CIC_DEBUG_ENABLED

#define CIC_DEBUG(text) { int attempts = 0; while (attempts <= SERIAL_ATTEMPTS) { if (takeSerialMutex()) { Serial.print((text)); Serial.print(F(" (Core: ")); Serial.print(xPortGetCoreID()); Serial.println(F(")")); giveSerialMutex(); break; } attempts = attempts + 1; delay(SERIAL_ATTEMPTS_DELAY); }}
#else
#define CIC_DEBUG(text) {}
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

    void preInitSystem();
    void initCommunication();
    void setupWizard();
    void initSystem();
    void checkAPWizard(xTaskHandle coreTask);
    void blinkStatus();
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

};

#endif