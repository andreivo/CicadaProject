/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

#include "DCPSystem.h"

/**
 * File system directories and variables
 */
SPIFFSManager spiffsManager;

#define CIC_DEBUG_ENABLED true
#define CIC_SYSTEM_BAUDRATE 115200
/******************************************************************************/
/******************************************************************************/
//Mutex
SemaphoreHandle_t SerialMutex = xSemaphoreCreateMutex();

boolean takeSerialMutex() {
    return (xSemaphoreTake(SerialMutex, (TickType_t) 1) == pdTRUE);
}

void giveSerialMutex() {
    xSemaphoreGive(SerialMutex);
}

//Mutex
SemaphoreHandle_t CommunicationMutex = xSemaphoreCreateMutex();

boolean takeCommunicationMutex() {
    //CIC_DEBUG("Get CommunicationMutex");
    return (xSemaphoreTake(CommunicationMutex, (TickType_t) 1) == pdTRUE);
}

boolean takeCommunicationMutexWait() {
    //CIC_DEBUG("Get CommunicationMutex");
    return (xSemaphoreTake(CommunicationMutex, portMAX_DELAY) == pdTRUE);
}

void giveCommunicationMutex() {
    //CIC_DEBUG("Give CommunicationMutex");
    xSemaphoreGive(CommunicationMutex);
}

void giveCommunicationMutexWait() {
    //CIC_DEBUG("Give CommunicationMutex");
    xSemaphoreGive(CommunicationMutex);
}
/******************************************************************************/
/******************************************************************************/

const String FIRMWARE_VERSION = "0.0.1-alpha";

String STATION_ID = "CicadaDCP";
const String STATION_ID_PREFIX = "CicadaDCP-";
String STATION_NAME = "CicadaDCP";
String STATION_PWD = "";
String STATION_LONGITUDE = "";
String STATION_LATITUDE = "";
float CIC_STATION_BUCKET_VOLUME = 3.22; // Bucket Calibrated Volume in ml
int CIC_STATION_STOREMETADATA = 10;


String SIM_ICCID = "";
String SIM_OPERA = "";
String COM_LOCAL_IP = "";
String COM_SIGNAL_QUALITTY = "";
String COM_TYPE = "WIFI";


DCPSerialCommands DCPCommands;

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

DCPSDCard logSDCard;

DCPMQTT cicadaMQTT;

boolean enableLog = false;

void logEnable() {
    enableLog = true;
}

void logDisable() {
    enableLog = false;
}

void cic_log(String msg, boolean ln) {
    if (enableLog) {
        logSDCard.writeLog(msg, ln);
    }
};

/**
 * Sensor configurations
 *
 */
DCPDht dcpDHT;
DCPRainGauge dcpRainGauge;
DCPVoltage dcpVoltage;

void IRAM_ATTR onTimeoutWizard() {
    //If the Wizard is active and the timeout has been reached reboot the module.
    //This causes an exception for access to global variables
    //without a critical section and forces a reboot.
    ESP.restart();
}

DCPSystem::DCPSystem() {

}

/**
 * Read Serial Commands
 */
void DCPSystem::readSerialCommands(xTaskHandle coreTask) {
    DCPCommands.readSerialCommands(coreTask);
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

    DCPCommands.initSerialCommands(FIRMWARE_VERSION);

    // Init the Serial
    CIC_DEBUG_SETUP(CIC_SYSTEM_BAUDRATE);
    CIC_DEBUG_(F("\n\nCICADA DCP FIRMWARE (Version "));
    CIC_DEBUG_(getFwmVersion());
    CIC_DEBUG(F(")"));

    //inicia o SDCard
    logSDCard.setupSDCardModule();
    cicadaSDCard.setupSDCardModule();
    cicadaSDCard.printDirectory("/");

    // Get Station ID from file system or create the file with
    initStationID();

    // Get Station Name
    initStationName();

    // Get Station PWD
    initStationPWD();

    // Register Firmware Version
    initFirmwareVersion();

    // Get Station Latitude and Longitude
    initStationCoordinates();

    // Get Station Calibrated Bucket Volume
    initBucketVolume();

    // Get Time slot to store metadata
    initSlotStoreMetadata();

    //Show all config
    printConfiguration();
}

boolean DCPSystem::initCommunication(boolean startPromptOnFail) {
    if (!dcpWifi.setupWiFiModule()) {
        if (!dcpSIM800.setupSIM800Module()) {
            if (startPromptOnFail) {
                return networkFailureBoot();
            }
            return false;
        } else {
            cicadaRTC.setupRTCModule(dcpSIM800.getNetworkDate());
        }
    } else {
        cicadaRTC.setupRTCModule(dcpWifi.getNetworkDate());
    }
    return true;
}

boolean DCPSystem::networkFailureBoot() {
    cicadaLeds.redTurnOn();
    cicadaLeds.greenTurnOn();
    cicadaLeds.blueTurnOn();

    Serial.println(F("\n\nNETWORK FAILURE ON BOOT"));
    Serial.println(F("=========================================================="));
    Serial.println(F("WARNING.....WARNING.....WARNING.....\n"
            "The system cannot be started without the internal clock being set.\n"
            "For clock setting, enable the network or provide the datetime manually."
            "\n"
            "What do you want to do? Options are:\n"
            "\tW - Enable Cicada Wizard on the access point to configuration network.\n"
            "\tM - Provide the datetime manually\n"
            "\tQ - Quit and reboot system.\n"
            "\n"
            "Enter option: "));
    while (!Serial.available()) {
        SysCall::yield();
    }
    char command = Serial.read();
    if (!strchr("WMQ", command)) {
        Serial.println(F("FAIL: Invalid option entered."));
        clearSerialInput();
        networkFailureBoot();
    }

    // Read any existing Serial data.
    clearSerialInput();
    if (command == 'W') {
        setupWizard(NULL);
    } else if (command == 'M') {
        clearSerialInput();
        Serial.println(F("Please provide a datetime in YYYY-mm-ddTHH:MM:SSZ format:"));

        while (!Serial.available()) {
            SysCall::yield();
        }

        if (Serial.available() > 0) {
            String strDatetime = Serial.readString();
            strDatetime.trim();
            Serial.flush();
            Serial.println(strDatetime);
            if (cicadaRTC.checkFormat(strDatetime)) {
                cicadaRTC.setupRTCModule(strDatetime);
                Serial.println(F("The clock setting is done! Operating without a network!"));
                clearSerialInput();
                cicadaLeds.redTurnOff();
                cicadaLeds.greenTurnOff();
                cicadaLeds.blueTurnOff();
                return true;
            } else {
                Serial.println(F("Quiting, invalid datetime. Restart in a few seconds."));
                delay(2000);
                ESP.restart();
            }
        }
    } else if (command == 'Q') {
        ESP.restart();
    }
}

void DCPSystem::clearSerialInput() {
    uint32_t m = micros();
    do {
        if (Serial.read() >= 0) {
            m = micros();
        }
    } while (micros() - m < 10000);
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
void DCPSystem::setupWizard(xTaskHandle coreTask) {
    CIC_DEBUG_HEADER(F("WIZARD CICADA DCP"));
    CIC_DEBUG(F("Timeout: 10 minutes"));

    cicadaLeds.blueTurnOn();
    setupTimeoutWizard();

    esp_task_wdt_delete(NULL);
    if (coreTask) {

        vTaskDelete(coreTask);
    }
    delay(100);

    cicadaWizard.setDebugOutput(true);
    String ssid = getSSIDAP();
    cicadaWizard.startWizardPortal(ssid.c_str(), STATION_PWD.c_str());
}

/**
 * Init all system configurations
 */
void DCPSystem::initSystem() {

    initSensorsConfig();
    nextSlotToSaveMetadata();
    initMQTT();

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

        setupWizard(coreTask);
    }
}

void DCPSystem::blinkStatus() {

    cicadaLeds.blinkStatusOk();
}

void DCPSystem::readSensors() {

    dcpDHT.readDHT();
    dcpRainGauge.readRG();
    dcpVoltage.readVccIn();
    dcpVoltage.readVccSol();
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
 * Initialize Station PWD
 */
void DCPSystem::initStationPWD() {

    CIC_DEBUG_HEADER(F("INIT STATION PWD"));

    STATION_PWD = spiffsManager.FSReadString(DIR_STATION_PASS);

    if (!STATION_PWD.length()) {

        CIC_DEBUG_(F("STATION PWD not found.\nSetting STATION PWD: '"));
        CIC_DEBUG_(STATION_ID);
        CIC_DEBUG(F("'"));

        STATION_PWD = STATION_ID;

        spiffsManager.FSCreateFile(DIR_STATION_PASS, STATION_PWD);
    }

    CIC_DEBUG_(F("STATION PWD: "));
    CIC_DEBUG(STATION_PWD);
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
 * Initialize TIME SLOT TO STORE METADATA
 */
void DCPSystem::initSlotStoreMetadata() {
    CIC_DEBUG_HEADER(F("INIT TIME SLOT TO STORE METADATA"));
    int smi = spiffsManager.FSReadInt(DIR_STATION_STOREMETADATA);

    if (smi) {
        CIC_STATION_STOREMETADATA = smi;
    } else {

        CIC_STATION_STOREMETADATA = 10;
        spiffsManager.FSCreateFile(DIR_STATION_STOREMETADATA, String(CIC_STATION_STOREMETADATA));
        CIC_DEBUG(F("TIME SLOT TO STORE METADATA not found.\nUsing 10 minutes as default value..."));
    }

    CIC_DEBUG_(F("TIME SLOT TO STORE METADATA: "));
    CIC_DEBUG_(CIC_STATION_STOREMETADATA);
    CIC_DEBUG(" hours");
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
    String codevin = spiffsManager.getSettings("Code vcc in", DIR_SENSOR_CODEVIN, false);
    String codevso = spiffsManager.getSettings("Code vcc sol", DIR_SENSOR_CODEVSO, false);

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
        codehum = "30";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_CODEPLUV);
        spiffsManager.FSCreateFile(DIR_SENSOR_CODEPLUV, codehum);
    }

    if (codevin == "") {
        codevin = "40";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_CODEVIN);
        spiffsManager.FSCreateFile(DIR_SENSOR_CODEVIN, codevin);
    }

    if (codevso == "") {
        codevso = "50";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_CODEVSO);
        spiffsManager.FSCreateFile(DIR_SENSOR_CODEVSO, codevso);
    }

    String dttemp = spiffsManager.getSettings("Data Type temp", DIR_SENSOR_DATATYPETEMP, false);
    String dthum = spiffsManager.getSettings("Data Type hum", DIR_SENSOR_DATATYPEHUM, false);
    String dtplu = spiffsManager.getSettings("Data Type plu", DIR_SENSOR_DATATYPEPLUV, false);
    String dtvin = spiffsManager.getSettings("Data Type vcc in", DIR_SENSOR_DATATYPEVIN, false);
    String dtvso = spiffsManager.getSettings("Data Type vcc sol", DIR_SENSOR_DATATYPEVSO, false);

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
        dtplu = "pluvio";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_DATATYPEPLUV);
        spiffsManager.FSCreateFile(DIR_SENSOR_DATATYPEPLUV, dtplu);
    }

    if (dtvin == "") {
        dtvin = "vccin";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_DATATYPEVIN);
        spiffsManager.FSCreateFile(DIR_SENSOR_DATATYPEVIN, dtvin);
    }

    if (dtvso == "") {
        dtvso = "vccso";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_DATATYPEVSO);
        spiffsManager.FSCreateFile(DIR_SENSOR_DATATYPEVSO, dtvso);
    }

    String collDHT = spiffsManager.getSettings("Coll. time interval DHT", DIR_SENSOR_COLLTINTDHT, false);
    String collplu = spiffsManager.getSettings("Coll. time interval plu", DIR_SENSOR_COLLTINTPLUV, false);
    String collvin = spiffsManager.getSettings("Coll. time interval vcc in", DIR_SENSOR_COLLTINTVIN, false);
    String collvso = spiffsManager.getSettings("Coll. time interval vcc sol", DIR_SENSOR_COLLTINTVSO, false);

    if (collDHT == "") {
        collDHT = "10";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_COLLTINTDHT);
        spiffsManager.FSCreateFile(DIR_SENSOR_COLLTINTDHT, collDHT);
    }

    if (collplu == "") {
        collplu = "10";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_COLLTINTPLUV);
        spiffsManager.FSCreateFile(DIR_SENSOR_COLLTINTPLUV, collplu);
    }

    if (collvin == "") {
        collvin = "10";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_COLLTINTVIN);
        spiffsManager.FSCreateFile(DIR_SENSOR_COLLTINTVIN, collvin);
    }

    if (collvso == "") {

        collvso = "10";
        spiffsManager.FSDeleteFiles(DIR_SENSOR_COLLTINTVSO);
        spiffsManager.FSCreateFile(DIR_SENSOR_COLLTINTVSO, collvso);
    }

    // Initialize DHT Sensor
    dcpDHT.initDHTSensor(codetemp, dttemp, codehum, dthum, collDHT.toInt());

    //Setup Rain Gauge Sensor
    dcpRainGauge.setupRGSensor();
    // Initialize Rain Gauge Sensor
    dcpRainGauge.initRGSensor(codeplu, dtplu, collplu.toInt());

    // Initialize VCC Sensor
    dcpVoltage.initVccSensor(codevin, dtvin, collvin.toInt(), codevso, dtvso, collvso.toInt());

    CIC_DEBUG(F("Finish sensor config"));
    CIC_DEBUG_(F("\n\n"));
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
/*                          SECOND CORE TASK                            */

/************************************************************************/
void DCPSystem::taskTransmitLoop() {
    CIC_DEBUG_HEADER(F("INIT TASK DATA TRANSMIT"));
    String taskMessage = F("Task running on core ");
    taskMessage = taskMessage + xPortGetCoreID();

    CIC_DEBUG(taskMessage);
    /* Inspect our own high water mark on entering the task. */
    UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    CIC_DEBUG_("Allocated stack: ");
    CIC_DEBUG(uxHighWaterMark);

    while (true) {
        vTaskDelay(100);
        storeMetadados();
        vTaskDelay(100);
        transmiteData();
    }
}

void DCPSystem::transmiteData() {
    if (!cicadaMQTT.sendAllMessagesDataWifi()) {
        if (!cicadaMQTT.sendAllMessagesDataSim(dcpSIM800.getModem())) {
            if (!initCommunication(false)) {
                CIC_DEBUG(F("Communication failure. Operating without network!"));
            }
        }
    }
}

void DCPSystem::nextSlotToSaveMetadata() {
    /*********************/
    int actualMinutes = cicadaRTC.now("%M").toInt() + 1;

    int rSlot = actualMinutes % CIC_STATION_STOREMETADATA;
    int iSlot = (int) (actualMinutes / CIC_STATION_STOREMETADATA);

    if (rSlot > 0) {
        iSlot = iSlot + 1;
    }

    int nextSlot = iSlot*CIC_STATION_STOREMETADATA;
    if (nextSlot >= 60) {
        nextSlot = nextSlot - 60;
    }
    nextTimeSlotToSaveMetadata = nextSlot;
    CIC_DEBUG_(F("Next slot to Save Metadata: "));
    CIC_DEBUG_(nextTimeSlotToSaveMetadata);
    CIC_DEBUG(F(" min."));
}

boolean DCPSystem::onTimeToSaveMetadata() {
    int actualMinutes = cicadaRTC.now("%M").toInt();
    return actualMinutes == nextTimeSlotToSaveMetadata;
}

void DCPSystem::storeMetadados() {
    if (onTimeToSaveMetadata()) {
        CIC_DEBUG_HEADER(F("STORE METADATA"));
        cicadaLeds.redTurnOn();
        updateCommunicationStatus();
        cicadaSDCard.storeMetadadosStation(STATION_LATITUDE, STATION_LONGITUDE, String(CIC_STATION_BUCKET_VOLUME), COM_TYPE, SIM_ICCID, SIM_OPERA, COM_LOCAL_IP, COM_SIGNAL_QUALITTY);
        nextSlotToSaveMetadata();
        cicadaLeds.redTurnOff();

        cicadaSDCard.cleanOlderFiles();
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