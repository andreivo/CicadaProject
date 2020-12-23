/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#include "DCPSystem.h"

/**
 * File system directories and variables
 */
SPIFFSManager spiffsManager;

#define CIC_DEBUG_ENABLED true
#define CIC_SYSTEM_BAUDRATE 115200


const String FIRMWARE_VERSION = "0.0.1.0";

String STATION_ID = "CicadaDCP";
const String STATION_ID_PREFIX = "CicadaDCP-";
String STATION_NAME = "CicadaDCP";
String STATION_LONGITUDE = "";
String STATION_LATITUDE = "";
float CIC_STATION_BUCKET_VOLUME = 3.22; // Bucket Calibrated Volume in ml

// Initialize CicadaWizard
CicadaWizard cicadaWizard;
hw_timer_t * timeoutWizard = NULL;

// Initialize DCPWifi
DCPwifi dcpWifi;
// Initialize DCPSIM800
DCPSIM800 dcpSIM800;

DCPLeds cicadaLeds;

DCPRTC cicadaRTC;

DCPSDCard cicadaSDCard;

/**
 * Sensor configurations
 *
 */
DCPDht dcpDHT;

void IRAM_ATTR onTimeoutWizard() {
    //If the Wizard is active and the timeout has been reached reboot the module.
    //This causes an exception for access to global variables
    //without a critical section and forces a reboot.
    ESP.restart();
}

DCPSystem::DCPSystem() {

}

/**
 * Initialize Station ID
 */
void DCPSystem::preInitSystem() {
    pinMode(PIN_AP_WIZARD, INPUT);
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_BLUE, OUTPUT);
    pinMode(PIN_MODEM_TURNON, OUTPUT);

    dcpSIM800.turnOff();

    cicadaLeds.redTurnOff();
    cicadaLeds.greenTurnOff();
    cicadaLeds.blueTurnOff();


    // Init the Serial
    CIC_DEBUG_SETUP(CIC_SYSTEM_BAUDRATE);
    CIC_DEBUG_(F("\n\nCICADA DCP FIRMWARE (Version "));
    CIC_DEBUG_(getFwmVersion());
    CIC_DEBUG(F(")"));


    // Initialize Station ID
    initStationID();

    // Initialize Station Name
    initStationName();

    // Register Firmware Version
    initFirmwareVersion();

    initSensorsConfig();

    //Show all config
    printConfiguration();
}

void DCPSystem::initCommunication() {
    if (!dcpWifi.setupWiFiModule()) {
        if (!dcpSIM800.setupSIM800Module()) {
            setupWizard();
        } else {
            cicadaRTC.setupRTCModule(dcpSIM800.getNetworkDate());
        }
    } else {
        cicadaRTC.setupRTCModule(dcpWifi.getNetworkEpoch());
    }
}

void DCPSystem::setupTimeoutWizard() {
    /****
     * Time interrupt for timeout to AP Wizard mode
     */
    // Configure Prescaler to 80, as our timer runs @ 80Mhz
    // Giving an output of 80,000,000 / 80 = 1,000,000 ticks / second
    timeoutWizard = timerBegin(0, 80, true);
    timerAttachInterrupt(timeoutWizard, &onTimeoutWizard, true);
    // Fire Interrupt every 1m ticks, so 1s
    // ticks * (seconds * minutes)
    uint64_t timeoutWiz = 1000000 * (60 * 10);
    //uint64_t timeoutWiz = 1000000 * (10);
    timerAlarmWrite(timeoutWizard, timeoutWiz, true);
    timerAlarmEnable(timeoutWizard);
}

/**
 * Setup Cicada Wizard
 */
void DCPSystem::setupWizard() {
    CIC_DEBUG_HEADER(F("SETUP CICADA DCP"));
    cicadaLeds.blueTurnOn();

    setupTimeoutWizard();

    cicadaWizard.setDebugOutput(true);
    String ssid = getSSIDAP();
    cicadaWizard.startWizardPortal(ssid.c_str());
}

/**
 * Init all system configurations
 */
void DCPSystem::initSystem() {

    // Get Station ID from file system or create the file with
    initStationID();

    // Get Station Name
    initStationName();

    // Register Firmware Version
    initFirmwareVersion();

    // Get Station Latitude and Longitude
    initStationCoordinates();

    // Get Station Calibrated Bucket Volume
    initBucketVolume();

    //inicia o SDCard
    cicadaSDCard.setupSDCardModule();
    cicadaSDCard.printDirectory("/", 0);

    cicadaLeds.redTurnOff();
    cicadaLeds.greenTurnOff();
    cicadaLeds.blueTurnOff();

    CIC_DEBUG_(F("Startup completed on: "));
    CIC_DEBUG(cicadaRTC.now());

}

void DCPSystem::printNowDate() {
    CIC_DEBUG(cicadaRTC.now("%Y-%m-%d %H%M%S"));
}

void DCPSystem::checkAPWizard() {
    if (digitalRead(PIN_AP_WIZARD) == HIGH) {
        setupWizard();
    }
}

void DCPSystem::blinkStatus() {
    cicadaLeds.blinkStatusOk();
}

void DCPSystem::readSensors() {
    dcpDHT.readDHT();
}

/**
 * Initialize Station ID
 */
void DCPSystem::initStationID() {

    CIC_DEBUG_HEADER(F("INIT STATION ID"));

    STATION_ID = spiffsManager.FSReadString(DIR_STATION_ID);

    if (!STATION_ID.length()) {

        CIC_DEBUG(F("STATION ID not found.\nSetting STATION ID ..."));

        uint32_t high = (ESP.getEfuseMac() >> 32) % 0xFFFFFFFF;
        STATION_ID = STATION_ID_PREFIX + high;

        spiffsManager.FSCreateFile(DIR_STATION_ID, STATION_ID);
    }

    CIC_DEBUG_(F("STATION ID: "));
    CIC_DEBUG(STATION_ID);
}

/**
 * Initialize Firmware Version
 */
void DCPSystem::initFirmwareVersion() {

    CIC_DEBUG_HEADER(F("INIT FIRMWARE VERSION"));

    spiffsManager.FSDeleteFiles(DIR_FIRMWARE_VERSION);
    spiffsManager.FSCreateFile(DIR_FIRMWARE_VERSION, FIRMWARE_VERSION);

    CIC_DEBUG_(F("FIRMWARE VERSION: "));
    CIC_DEBUG(FIRMWARE_VERSION);
}

/**
 * Initialize Station Name
 */
void DCPSystem::initStationName() {

    CIC_DEBUG_HEADER(F("INIT STATION NAME"));

    String sttName = spiffsManager.FSReadString(DIR_STATION_NAME);

    // If STATION NAME not found, set default
    if (!sttName.length()) {

        CIC_DEBUG(F("STATION NAME not found.\nSetting default STATION NAME ..."));
        spiffsManager.FSCreateFile(DIR_STATION_NAME, STATION_NAME);
    } else {

        STATION_NAME = sttName;
    }

    CIC_DEBUG_(F("STATION NAME: "));
    CIC_DEBUG(STATION_NAME);
}

/**
 * Initialize Station Coordinates
 */
void DCPSystem::initStationCoordinates() {

    CIC_DEBUG_HEADER(F("INIT STATION COORDINATES"));

    STATION_LATITUDE = spiffsManager.FSReadString(DIR_STATION_LATITUDE);
    STATION_LONGITUDE = spiffsManager.FSReadString(DIR_STATION_LONGITUDE);

    CIC_DEBUG_(F("STATION LATITUDE: "));
    CIC_DEBUG(STATION_LATITUDE);
    CIC_DEBUG_(F("STATION LONGITUDE: "));
    CIC_DEBUG(STATION_LONGITUDE);
}

/**
 * Initialize Station Bucket Volume
 */
void DCPSystem::initBucketVolume() {

    CIC_DEBUG_HEADER(F("INIT BUCKET VOLUME"));

    float vol = spiffsManager.FSReadFloat(DIR_STATION_BUCKET_VOL);

    if (vol) {
        CIC_STATION_BUCKET_VOLUME = vol;
    } else {

        CIC_DEBUG(F("STATION BUCKET VOLUME not found.\nUsing default value..."));
    }

    CIC_DEBUG_(F("STATION BUCKET VOLUME: "));
    CIC_DEBUG(CIC_STATION_BUCKET_VOLUME);
}

/**
 * Initialize Sensors
 */
void DCPSystem::initSensorsConfig() {

    CIC_DEBUG_HEADER(F("INIT Sensor Config"));

    String codetemp = spiffsManager.getSettings("Code temp", DIR_SENSOR_CODETEMP, false);
    String codehum = spiffsManager.getSettings("Code hum", DIR_SENSOR_CODEHUM, false);
    String codeplu = spiffsManager.getSettings("Code plu", DIR_SENSOR_CODEPLUV, false);
    String codebtv = spiffsManager.getSettings("Code bat. vol.", DIR_SENSOR_CODEBATV, false);
    String codebtc = spiffsManager.getSettings("Code bat. cur.", DIR_SENSOR_CODEBATC, false);

    if (codetemp == "") {
        codetemp = "10";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_CODETEMP);
        spiffsManager.FSCreateFile(DIR_SENSOR_CODETEMP, codetemp);
    }

    if (codehum == "") {
        codehum = "20";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_CODEHUM);
        spiffsManager.FSCreateFile(DIR_SENSOR_CODEHUM, codehum);
    }

    if (codeplu == "") {
        spiffsManager.FSDeleteFiles(DIR_SENSOR_CODEPLUV);
        spiffsManager.FSCreateFile(DIR_SENSOR_CODEPLUV, 30);
    }

    if (codebtv == "") {
        spiffsManager.FSDeleteFiles(DIR_SENSOR_CODEBATV);
        spiffsManager.FSCreateFile(DIR_SENSOR_CODEBATV, 40);
    }

    if (codebtc == "") {
        spiffsManager.FSDeleteFiles(DIR_SENSOR_CODEBATC);
        spiffsManager.FSCreateFile(DIR_SENSOR_CODEBATC, 50);
    }

    String dttemp = spiffsManager.getSettings("Data Type temp", DIR_SENSOR_DATATYPETEMP, false);
    String dthum = spiffsManager.getSettings("Data Type hum", DIR_SENSOR_DATATYPEHUM, false);
    String dtplu = spiffsManager.getSettings("Data Type plu", DIR_SENSOR_DATATYPEPLUV, false);
    String dtbtv = spiffsManager.getSettings("Data Type bat. vol.", DIR_SENSOR_DATATYPEBATV, false);
    String dtbtc = spiffsManager.getSettings("Data Type bat. cur.", DIR_SENSOR_DATATYPEBATC, false);

    if (dttemp == "") {
        dttemp = "temp";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_DATATYPETEMP);
        spiffsManager.FSCreateFile(DIR_SENSOR_DATATYPETEMP, dttemp);
    }

    if (dthum == "") {
        dthum = "hum";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_DATATYPEHUM);
        spiffsManager.FSCreateFile(DIR_SENSOR_DATATYPEHUM, dthum);
    }

    if (dtplu == "") {
        spiffsManager.FSDeleteFiles(DIR_SENSOR_DATATYPEPLUV);
        spiffsManager.FSCreateFile(DIR_SENSOR_DATATYPEPLUV, "pluvio");
    }

    if (dtbtv == "") {
        spiffsManager.FSDeleteFiles(DIR_SENSOR_DATATYPEBATV);
        spiffsManager.FSCreateFile(DIR_SENSOR_DATATYPEBATV, "battery");
    }

    if (dtbtc == "") {
        spiffsManager.FSDeleteFiles(DIR_SENSOR_DATATYPEBATC);
        spiffsManager.FSCreateFile(DIR_SENSOR_DATATYPEBATC, "current");
    }

    String colltemp = spiffsManager.getSettings("Coll. time interval temp", DIR_SENSOR_COLLTINTTEMP, false);
    String collhum = spiffsManager.getSettings("Coll. time interval hum", DIR_SENSOR_COLLTINTHUM, false);
    String collplu = spiffsManager.getSettings("Coll. time interval plu", DIR_SENSOR_COLLTINTPLUV, false);
    String collbtv = spiffsManager.getSettings("Coll. time interval bat. vol.", DIR_SENSOR_COLLTINTBATV, false);
    String collbtc = spiffsManager.getSettings("Coll. time interval bat. cur.", DIR_SENSOR_COLLTINTBATC, false);

    if (colltemp == "") {
        colltemp = "10";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_COLLTINTTEMP);
        spiffsManager.FSCreateFile(DIR_SENSOR_COLLTINTTEMP, colltemp);
    }

    if (collhum == "") {
        collhum = "10";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_COLLTINTHUM);
        spiffsManager.FSCreateFile(DIR_SENSOR_COLLTINTHUM, collhum);
    }

    if (collplu == "") {
        spiffsManager.FSDeleteFiles(DIR_SENSOR_COLLTINTPLUV);
        spiffsManager.FSCreateFile(DIR_SENSOR_COLLTINTPLUV, 10);
    }

    if (collbtv == "") {
        spiffsManager.FSDeleteFiles(DIR_SENSOR_COLLTINTBATV);
        spiffsManager.FSCreateFile(DIR_SENSOR_COLLTINTBATV, 10);
    }

    if (collbtc == "") {
        spiffsManager.FSDeleteFiles(DIR_SENSOR_COLLTINTBATC);
        spiffsManager.FSCreateFile(DIR_SENSOR_COLLTINTBATC, 10);
    }

    // Initialize DHT Sensor
    dcpDHT.initDHTSensor(codetemp, dttemp, codehum, dthum, colltemp.toInt(), colltemp.toInt());

    CIC_DEBUG(F("Finish sensor config"));

}

void DCPSystem::printConfiguration() {

    spiffsManager.FSPrintFileList();
}

String DCPSystem::getFwmVersion() {

    return FIRMWARE_VERSION;
}

String DCPSystem::getSSIDAP() {
    String __ssidAP = STATION_ID;
    return __ssidAP;
}
