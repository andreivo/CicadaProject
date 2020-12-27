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

//Mutex
SemaphoreHandle_t SerialMutex = xSemaphoreCreateMutex();

boolean takeSerialMutex() {
    return (xSemaphoreTake(SerialMutex, 1) == pdTRUE);
}

void giveSerialMutex() {
    xSemaphoreGive(SerialMutex);
}

const String FIRMWARE_VERSION = "0.0.1.0";

String STATION_ID = "CicadaDCP";
const String STATION_ID_PREFIX = "CicadaDCP-";
String STATION_NAME = "CicadaDCP";
String STATION_LONGITUDE = "";
String STATION_LATITUDE = "";
float CIC_STATION_BUCKET_VOLUME = 3.22; // Bucket Calibrated Volume in ml


String SIM_ICCID = "";
String SIM_OPERA = "";
String COM_LOCAL_IP = "";
String COM_SIGNAL_QUALITTY = "";
String COM_TYPE = "WIFI";


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

DCPMQTT cicadaMQTT;

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
    lastEpMetadados = cicadaRTC.nowEpoch();
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

    //inicia o SDCard
    cicadaSDCard.setupSDCardModule();
    cicadaSDCard.printDirectory("/", 0);

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

    initMQTT();

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
    // ticks * (seconds * minutes) = 10 minutos
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
    initSensorsConfig();

    cicadaLeds.redTurnOff();
    cicadaLeds.greenTurnOff();
    cicadaLeds.blueTurnOff();

    CIC_DEBUG_(F("Startup completed on: "));
    printNowDate();
}

void DCPSystem::printNowDate() {
    CIC_DEBUG(cicadaRTC.now());
}

void DCPSystem::checkAPWizard(xTaskHandle coreTask) {
    if (digitalRead(PIN_AP_WIZARD) == HIGH) {
        vTaskDelete(coreTask);
        delay(100);
        setupWizard();
    }
}

void DCPSystem::blinkStatus() {
    cicadaLeds.blinkStatusOk();
}

void DCPSystem::readSensors() {
    dcpDHT.readDHT();
    delay(1000);
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
 * Initialize MQTT
 */
void DCPSystem::initMQTT() {
    CIC_DEBUG_HEADER(F("INIT MQTT Config"));

    // Get MQTT Host
    String host = spiffsManager.getSettings("MQTT Host", DIR_MQTT_SERVER, true);
    // Get MQTT Port
    String port = spiffsManager.getSettings("MQTT Port", DIR_MQTT_PORT, false);
    // Get MQTT User
    String user = spiffsManager.getSettings("MQTT User", DIR_MQTT_USER, false);
    // Get MQTT Password
    String pass = spiffsManager.getSettings("MQTT Password", DIR_MQTT_PWD, true);
    // Get MQTT Topic
    String topic = spiffsManager.getSettings("MQTT Topic", DIR_MQTT_TOPIC, false);

    String sttPass = spiffsManager.FSReadString(DIR_STATION_PASS);
    String timeToSend = spiffsManager.FSReadString(DIR_STATION_SENDTIMEINTERVAL);
    if (timeToSend == "") {
        timeToSend = "10";
        spiffsManager.FSDeleteFiles(DIR_STATION_SENDTIMEINTERVAL);
        spiffsManager.FSCreateFile(DIR_STATION_SENDTIMEINTERVAL, timeToSend);
    }

    cicadaMQTT.setupMQTTModule(timeToSend.toInt(), STATION_ID, host, port, user, pass, topic, STATION_NAME, sttPass, STATION_LATITUDE, STATION_LONGITUDE);
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
    dcpDHT.initDHTSensor(codetemp, dttemp, codehum, dthum, colltemp.toInt(), collhum.toInt());

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

/************************************************************************/
/************************************************************************/

/************************************************************************/
void DCPSystem::loopCore2() {
    CIC_DEBUG_HEADER(F("INIT LOOP 2"));
    String taskMessage = F("Task running on core ");
    taskMessage = taskMessage + xPortGetCoreID();

    CIC_DEBUG(taskMessage);
    /* Inspect our own high water mark on entering the task. */
    UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    CIC_DEBUG_("Allocated stack: ");
    CIC_DEBUG(uxHighWaterMark);

    while (true) {
        storeMetadados();
        transmiteData();
        vTaskDelay(10);
    }
}

void DCPSystem::transmiteData() {
    if (dcpWifi.isConnected()) {
        cicadaMQTT.sendAllMessagesData();
    } else {
        if (dcpSIM800.isConnected()) {
            cicadaMQTT.sendAllMessagesData(dcpSIM800.getModem());
        } else {
            initCommunication();
        }
    }
}

void DCPSystem::storeMetadados() {
    int actualHour = cicadaRTC.now("%H").toInt();
    int actualMinutes = cicadaRTC.now("%M").toInt();
    int actualSeconds = cicadaRTC.now("%S").toInt();

    if (((actualHour % 12) == 0) && (actualMinutes == 12) && (actualSeconds == 12)) {
        updateCommunicationStatus();
        cicadaSDCard.storeMetadadosStation(STATION_LATITUDE, STATION_LONGITUDE, String(CIC_STATION_BUCKET_VOLUME), COM_TYPE, SIM_ICCID, SIM_OPERA, COM_LOCAL_IP, COM_SIGNAL_QUALITTY);
        vTaskDelay(1000);
    }
}

void DCPSystem::updateCommunicationStatus() {
    CIC_DEBUG_HEADER(F("UPDATE COMMUNICATION STATUS"));

    if (dcpWifi.isConnected()) {
        COM_TYPE = "WIFI";
        CIC_DEBUG_(F("Conection Type:"));
        CIC_DEBUG(COM_TYPE);

        SIM_ICCID = "";
        CIC_DEBUG_(F("CCID:"));
        CIC_DEBUG(SIM_ICCID);

        SIM_OPERA = "";
        CIC_DEBUG(F("Operator:"));
        CIC_DEBUG(SIM_OPERA);

        IPAddress local = dcpWifi.getLocalIP();
        COM_LOCAL_IP = IpAddress2String(local);
        CIC_DEBUG_(F("Local IP:"));
        CIC_DEBUG(COM_LOCAL_IP);

        COM_SIGNAL_QUALITTY = dcpWifi.getSignalQuality();
        CIC_DEBUG_(F("Signal quality:"));
        CIC_DEBUG(COM_SIGNAL_QUALITTY);
    } else {
        COM_TYPE = "SIM";
        CIC_DEBUG_(F("Conection Type:"));
        CIC_DEBUG(COM_TYPE);

        SIM_ICCID = dcpSIM800.getSimCCID();
        CIC_DEBUG_(F("CCID:"));
        CIC_DEBUG(SIM_ICCID);

        SIM_OPERA = dcpSIM800.getOperator();
        CIC_DEBUG_(F("Operator:"));
        CIC_DEBUG(SIM_OPERA);

        IPAddress local = dcpSIM800.getLocalIP();
        COM_LOCAL_IP = IpAddress2String(local);
        CIC_DEBUG_(F("Local IP:"));
        CIC_DEBUG(COM_LOCAL_IP);

        COM_SIGNAL_QUALITTY = dcpSIM800.getSignalQuality();
        CIC_DEBUG_(F("Signal quality:"));
        CIC_DEBUG(COM_SIGNAL_QUALITTY);
    }
}

String DCPSystem::IpAddress2String(const IPAddress& ipAddress) {
    return String(ipAddress[0]) + String(".") +\
 String(ipAddress[1]) + String(".") +\
 String(ipAddress[2]) + String(".") +\
 String(ipAddress[3]);
}
