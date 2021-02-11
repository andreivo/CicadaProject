/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */
#include "DCPSerialCommands.h"

DCPRTC serialComRTC;
DCPwifi serialWifi;
DCPSIM800 serialSIM800;
SPIFFSManager commSpiffsManager;
DCPDht serialDHT;
DCPRainGauge serialdcpRainGauge;
DCPVoltage serialdcpVoltage;
DCPSDCard serialSDCard;
SPIFFSManager serialSpiffs;
DCPLeds serialLeds;
DCPMQTT serialMQTT;

CicadaWizard serialWizard;
hw_timer_t * timeoutSerialWizard = NULL;

DCPSelfUpdate serialSelfUpdate;

DCPSerialCommands::DCPSerialCommands() {
}

void DCPSerialCommands::initSerialCommands(String firmware, String firmwareDate) {
    FIRMWARE = firmware;
    FIRMWARE_DATE = firmwareDate;
}

String DCPSerialCommands::getArguments(String data, int index, char separator) {
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;
    char oldSeparator = separator;
    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == '"' && oldSeparator != '"') {
            separator = '#';
        }

        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex && data.charAt(i) != separator) ? i + 1 : i;
            separator = oldSeparator;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

/**
 * Read Serial Commands
 */
void DCPSerialCommands::readSerialCommands(xTaskHandle coreTask) {

    // Holds Serial Monitor commands
    String serialCommand = "";

    // Serial availability
    if (Serial.available()) {

        // Read the incoming command
        serialCommand = Serial.readString();
        serialCommand.trim();
        Serial.flush();

        String mandatoryCommand = getArguments(serialCommand, 0);
        mandatoryCommand.trim();

        if (mandatoryCommand == "help") {
            printCommands();
        } else if (mandatoryCommand == "reboot") {
            rebootComm();
        } else if (mandatoryCommand == "time") {
            timeComm(serialCommand);
        } else if (mandatoryCommand == "nextslottime") {
            nextslottimeComm();
        } else if (mandatoryCommand == "status") {
            statusComm();
        } else if (mandatoryCommand == "factoryreset") {
            factoryresetComm(coreTask);
        } else if (mandatoryCommand == "sconfig") {
            sconfigComm();
        } else if (mandatoryCommand == "wizard") {
            wizardComm(coreTask);
        } else if (mandatoryCommand == "forceupdate") {
            forceUpdateComm(serialCommand);
        } else if (mandatoryCommand == "wifi") {
            wifiComm(serialCommand);
        } else if (mandatoryCommand == "sim") {
            simComm(serialCommand);
        } else if (mandatoryCommand == "ls") {
            lsComm(serialCommand);
        } else if (mandatoryCommand == "cat") {
            catComm(serialCommand);
        } else if (mandatoryCommand == "fsformat") {
            serialSDCard.formatSD();
        } else if (mandatoryCommand == "fsstatus") {
            fsstatusComm();
        } else if (mandatoryCommand == "deloldfiles") {
            serialSDCard.deleteOldFiles();
        } else if (mandatoryCommand == "weather") {
            weatherComm(serialCommand);
        } else {
            Serial.print(F("ERROR! Command not recognized: "));
            Serial.println(serialCommand);
            printCommands();
        }
    }
}

/**
 * Print the serial available commands
 */
void DCPSerialCommands::printCommands() {

    Serial.println(F(""));
    Serial.println(F("======================================================"));
    Serial.println(F("           CICADA DATA COLLECTION PLATFORM            "));
    Serial.print(F("                 Version: "));
    Serial.println(FIRMWARE);
    Serial.print(F("             Date: "));
    Serial.println(FIRMWARE_DATE);
    Serial.println(F("      https://github.com/andreivo/CicadaProject       "));
    Serial.println(F("======================================================"));
    Serial.println(F(""));
    Serial.println(F("SYSTEM AVALIABLE COMMANDS"));
    Serial.println(F("----------------------------------------------------------"));

    Serial.println(F("help              Show all available commands"));

    Serial.println(F("\nSystem commands:"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.println(F("reboot             Reboot system."));
    Serial.println(F("time               Print the current system datetime."));
    Serial.println(F("nextslottime       Prints when the next read slot."));
    Serial.println(F("   -s \"[value]\"    Set a new system datetime. Format value YYYY-mm-ddTHH:MM:SSZ."));
    Serial.println(F("status             Print the current system status to serial monitor including,"));
    Serial.println(F("                     wifi connectivity, Sim connectivity, etc."));
    Serial.println(F("factoryreset       Reset system completely, format configuration file system,"));
    Serial.println(F("                     format weather file system, reset all configuration and"));
    Serial.println(F("                     reboot system."));
    Serial.println(F("                     (WARNING: cannot be undone)"));
    Serial.println(F("sconfig            Print the current system configuration."));
    Serial.println(F("wizard             Enable Cicada Wizard on the access point network."));
    Serial.println(F("forceupdate        Starts checking for new firmware and updates the system if it exists."));
    Serial.println(F("   -f \"[value]\"    Updates the firmware based on the specified file."));


    Serial.println(F("\nWifi system commands:"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.println(F("wifi               Print the current system status."));
    Serial.println(F("   -r              Reset WiFi appliance settings, SSID and password."));
    Serial.println(F("   -d              Print available SSID."));
    Serial.println(F("   -s \"[value]\"    Set a new SSID."));
    Serial.println(F("   -p \"[value]\"    Set a new Password."));


    Serial.println(F("\nSim Card system commands:"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.println(F("sim                Print the current system status."));
    Serial.println(F("   -on             Moden turn on."));
    Serial.println(F("   -off            Moden turn off."));
    Serial.println(F("   -r              Reset SIM configuration. (WARNING: This action disconnect to the wifi.)"));
    Serial.println(F("   -a \"[value]\"    Set a new Carrier APN."));
    Serial.println(F("   -u \"[value]\"    Set a new User."));
    Serial.println(F("   -p \"[value]\"    Set a new Password."));
    Serial.println(F("   -c \"[value]\"    Send AT command. Do not include the AT prefix, e.g +CSQ."));

    Serial.println(F("\nWeather files commands:"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.println(F("ls                 Print the current weather file list."));
    Serial.println(F("   -u              Print the update file list."));
    Serial.println(F("cat -f \"[file]\"    Print the file content."));
    Serial.println(F("                   Print the file content e.g 1: cat -f \"20210106.dht\""));
    Serial.println(F("deloldfiles        Delete files older than 1 year."));
    Serial.println(F("fsformat           Format weather file system. (WARNING: cannot be undone)"));
    Serial.println(F("fsstatus           Print the current weather file system status."));

    Serial.println(F("\nWeather data commands:"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.println(F("weather            Print the current sensor values."));
    Serial.println(F("   -ht             Print the current humidity and temperature."));
    Serial.println(F("   -r              Print the current rain gauge tip bucket count."));
    Serial.println(F("   -b              Print the current battery voltage."));
    Serial.println(F("   -s              Print the current solar cell voltage."));

    Serial.println(F(""));
}

void DCPSerialCommands::rebootComm() {
    ESP.restart();
}

void DCPSerialCommands::statusComm() {
    printSystemEnvironmentStatus();
    printSystemWiFiStatus();
    printSystemSimStatus();
    printSystemWeathers();
    fsstatusComm();
}

void DCPSerialCommands::timeComm(String serialCommand) {
    Serial.print("Datetime: ");
    Serial.println(serialComRTC.now());
    String args = getArguments(serialCommand, 1);
    if (args != "") {
        String madatoryArg = getArguments(args, 0, ' ');
        if (madatoryArg == "s") {
            String newValue = getArguments(args, 1, ' ');
            newValue = getArguments(newValue, 1, '"');
            if (serialComRTC.checkFormat(newValue)) {
                serialComRTC.setupRTCModule(newValue);
            } else {
                Serial.print(F("ERROR! Value argument not recognized: "));
                Serial.println(newValue);
            }
        } else {
            Serial.print(F("ERROR! Argument not recognized: "));
            Serial.println(madatoryArg);
        }
    }
}

void DCPSerialCommands::nextslottimeComm() {
    Serial.println(F("\nNEXT READ SLOT"));
    Serial.println(F("----------------------------------------------------------"));
    serialDHT.printNextSlot();
    serialdcpRainGauge.printNextSlot();
    serialdcpVoltage.printNextVccInSlot();
    serialdcpVoltage.printNextVccSolSlot();
    serialMQTT.printNextSlot();
}

void DCPSerialCommands::factoryresetComm(xTaskHandle coreTask) {
    Serial.println(F("\n\nFACTORY RESET"));
    Serial.println(F("=========================================================="));
    Serial.println(F("WARNING.....WARNING.....WARNING.....\n"
            "Reset system completely, format configuration file system,\n"
            "format weather file system, reset all configuration and \n"
            "reboot system. \n\n"
            "(WARNING: cannot be undone)\n\n"
            "Enter 'Y' to continue: "));
    while (!Serial.available()) {
        SysCall::yield();
    }
    char command = Serial.read();
    if (command != 'Y') {
        Serial.println(F("Quiting, you did not enter 'Y'."));
        // Read any existing Serial data.
        clearSerialInput();
        return;
    }
    // Read any existing Serial data.
    clearSerialInput();

    esp_task_wdt_delete(NULL);
    vTaskDelete(coreTask);
    delay(100);

    serialSpiffs.FSFormat();
    serialSDCard.formatSD(false);
    delay(1000);
    rebootComm();
}

void DCPSerialCommands::sconfigComm() {
    Serial.println(F("\nSYSTEM CONFIGURATION"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.println(F("To change these values, please access Cicada Wizard AP."));
    Serial.println(F(""));

    Serial.println(F("System"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.print(F("Firmware                   : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_FIRMWARE_VERSION, false));
    Serial.println(F(""));

    Serial.println(F("Station"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.print(F("ID                         : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_STATION_ID, false));
    Serial.print(F("Name                       : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_STATION_NAME, false));
    Serial.print(F("Password                   : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_STATION_PASS, false));
    Serial.print(F("Latitude                   : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_STATION_LATITUDE, false));
    Serial.print(F("Longitude                  : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_STATION_LONGITUDE, false));
    Serial.print(F("Bucket Calibration         : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_STATION_BUCKET_VOL, false));
    Serial.print(F("Time slot to send          : "));
    Serial.print(serialSpiffs.getSettings(".", DIR_STATION_SENDTIMEINTERVAL, false));
    Serial.println(F(" minutes"));
    Serial.print(F("Time slot to store metadata: "));
    Serial.print(serialSpiffs.getSettings(".", DIR_STATION_STOREMETADATA, false));
    Serial.println(F(" minutes"));
    Serial.print(F("Self update-Host           : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_STATION_SEHOST, true));
    Serial.print(F("Self update-Path           : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_STATION_SEPATH, true));
    Serial.print(F("Self update-Port           : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_STATION_SEPORT, false));
    Serial.print(F("Self update-Daily Time     : "));
    Serial.print(serialSpiffs.getSettings(".", DIR_STATION_SETIME, false));
    Serial.println(F(":00 hours"));

    Serial.println(F(""));
    Serial.println(F("DHT Sensor"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.print(F("Time slot to collection    : "));
    Serial.print(serialSpiffs.getSettings(".", DIR_SENSOR_COLLTINTDHT, false));
    Serial.println(F(" minutes"));
    Serial.print(F("Temperature Code           : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_SENSOR_CODETEMP, false));
    Serial.print(F("Temperature Data Type      : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_SENSOR_DATATYPETEMP, false));
    Serial.print(F("Humidity Code              : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_SENSOR_CODEHUM, false));
    Serial.print(F("Humidity Data Type         : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_SENSOR_DATATYPEHUM, false));

    Serial.println(F(""));
    Serial.println(F("Pluviometer"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.print(F("Code                       : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_SENSOR_CODEPLUV, false));
    Serial.print(F("Data Type      : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_SENSOR_DATATYPEPLUV, false));
    Serial.print(F("Time slot to collection    : "));
    Serial.print(serialSpiffs.getSettings(".", DIR_SENSOR_COLLTINTPLUV, false));
    Serial.println(F(" minutes"));

    Serial.println(F(""));
    Serial.println(F("Input Battery Vcc"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.print(F("Code                       : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_SENSOR_CODEVIN, false));
    Serial.print(F("Data Type                  : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_SENSOR_DATATYPEVIN, false));
    Serial.print(F("Time slot to collection    : "));
    Serial.print(serialSpiffs.getSettings(".", DIR_SENSOR_COLLTINTVIN, false));
    Serial.println(F(" minutes"));

    Serial.println(F(""));
    Serial.println(F("Solar Cell Vcc"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.print(F("Code                       : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_SENSOR_CODEVSO, false));
    Serial.print(F("Data Type                  : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_SENSOR_DATATYPEVSO, false));
    Serial.print(F("Time slot to collection    : "));
    Serial.print(serialSpiffs.getSettings(".", DIR_SENSOR_COLLTINTVSO, false));
    Serial.println(F(" minutes"));

    Serial.println(F(""));
    Serial.println(F("MQTT server"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.print(F("MQTT host server           : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_MQTT_SERVER, true));
    Serial.print(F("MQTT port                  : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_MQTT_PORT, false));
    Serial.print(F("MQTT user                  : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_MQTT_USER, false));
    Serial.print(F("MQTT password              : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_MQTT_PWD, false));
    Serial.print(F("MQTT password              : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_MQTT_PWD, true));
    Serial.print(F("MQTT Topic                 : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_MQTT_TOPIC, false));

    Serial.println(F(""));
    Serial.println(F("SIM Card"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.print(F("Carrier APN                : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_SIMCARD_APN, true));
    Serial.print(F("Carrier APN User           : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_SIMCARD_USER, false));
    Serial.print(F("Carrier APN Password       : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_SIMCARD_PWD, true));

    Serial.println(F(""));
    Serial.println(F("Wifi"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.print(F("SSID                       : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_WIFI_SSID, false));
    Serial.print(F("Password                   : "));
    Serial.println(serialSpiffs.getSettings(".", DIR_WIFI_PWD, false));
}

void DCPSerialCommands::initSelfUpdate() {
    CIC_DEBUG_HEADER(F("INIT FORCE SELF UPDATE"));

    // Get Host
    String host = commSpiffsManager.getSettings("Self Up Host", DIR_STATION_SEHOST, true);
    // Get HostPath
    String hostpath = commSpiffsManager.getSettings("Self Up Host Path", DIR_STATION_SEPATH, true);
    // Get Port
    String port = commSpiffsManager.getSettings("Self Up Port", DIR_STATION_SEPORT, false);

    String sttName = commSpiffsManager.FSReadString(DIR_STATION_NAME);

    String timeToCheckUp = commSpiffsManager.FSReadString(DIR_STATION_SETIME);
    if (timeToCheckUp == "") {
        timeToCheckUp = "10";
        commSpiffsManager.FSDeleteFiles(DIR_STATION_SETIME);
        commSpiffsManager.FSCreateFile(DIR_STATION_SETIME, timeToCheckUp);
    }

    serialSelfUpdate.setupSelfUpdate(timeToCheckUp.toInt(), host, hostpath, port.toInt(), FIRMWARE_DATE, sttName);
}

void DCPSerialCommands::forceUpdateComm(String serialCommand) {
    initSelfUpdate();

    String args = getArguments(serialCommand, 1);
    if (args != "") {
        String madatoryArg = getArguments(args, 0, ' ');
        String value = getArguments(args, 1, ' ');
        value = getArguments(value, 1, '"');
        if (madatoryArg == "f") {
            Serial.println(F("\n\nSTARTING FIRMWARE UPDATE"));
            Serial.println(F("=========================================================="));
            Serial.println(F("WARNING.....WARNING.....WARNING.....\n"
                    "This operation will update the firmware and is irreversible.\n"
                    "(WARNING: cannot be undone)\n\n"
                    "Enter 'Y' to continue: "));
            while (!Serial.available()) {
                SysCall::yield();
            }
            char command = Serial.read();
            if (command != 'Y') {
                Serial.println(F("Quiting, you did not enter 'Y'."));
                // Read any existing Serial data.
                clearSerialInput();
                return;
            }
            // Read any existing Serial data.
            clearSerialInput();
            serialSelfUpdate.startUpdate(value);
            updateAllSlots();
        } else {
            Serial.print(F("ERROR! Argument not recognized: "));
            Serial.println(madatoryArg);
        }
    } else {
        serialSelfUpdate.updateFirmware(true);
        updateAllSlots();
    }
}

void DCPSerialCommands::wizardComm(xTaskHandle coreTask) {
    Serial.println(F("WIZARD CICADA DCP"));
    Serial.println(F("----------------------------------------------------------"));
    Serial.println(F("Timeout: 10 minutes"));

    serialLeds.blueTurnOn();
    setupTimeoutWizard();

    esp_task_wdt_delete(NULL);
    vTaskDelete(coreTask);
    delay(100);

    serialWizard.setDebugOutput(true);
    String ssid = serialSpiffs.getSettings(".", DIR_STATION_ID, false);
    String pwd = serialSpiffs.getSettings(".", DIR_STATION_PASS, false);
    serialWizard.startWizardPortal(ssid.c_str(), pwd.c_str());
}

void IRAM_ATTR onTimeoutSerialWizard() {
    //If the Wizard is active and the timeout has been reached reboot the module.
    //This causes an exception for access to global variables
    //without a critical section and forces a reboot.
    ESP.restart();
}

void DCPSerialCommands::setupTimeoutWizard() {
    /****
     * Time interrupt for timeout to AP Wizard mode
     */
    // Configure Prescaler to 80, as our timer runs @ 80Mhz
    // Giving an output of 80,000,000 / 80 = 1,000,000 ticks / second
    timeoutSerialWizard = timerBegin(0, 80, true);
    timerAttachInterrupt(timeoutSerialWizard, &onTimeoutSerialWizard, true);
    // Fire Interrupt every 1m ticks, so 1s
    // ticks * (seconds * minutes) = 10 minutos
    uint64_t timeoutWiz = 1000000 * (60 * 10);
    //uint64_t timeoutWiz = 1000000 * (10);
    timerAlarmWrite(timeoutSerialWizard, timeoutWiz, true);
    timerAlarmEnable(timeoutSerialWizard);
}

float DCPSerialCommands::bytesConverter(float bytes, char prefix) {

    // Kilobyte (KB)
    if (prefix == 'K') {
        return bytes / 1000;

        // Megabyte (MB)
    } else if (prefix == 'M') {
        return bytes / 1000000;
    }
}

/**
 * Print system environment status to serial
 */
void DCPSerialCommands::printSystemEnvironmentStatus() {

    Serial.println(F("\nSYSTEM ENVIRONMENT STATUS"));
    Serial.println(F("----------------------------------------------------------"));

    Serial.print(F("Free Heap size: "));
    Serial.print(ESP.getFreeHeap());
    Serial.print(F(" bytes ("));
    Serial.print(bytesConverter(ESP.getFreeHeap(), 'K'));
    Serial.println(F(" KB)"));

    Serial.print(F("Flash Chip Size: "));
    Serial.print(ESP.getFlashChipSize());
    Serial.print(F(" bytes ("));
    Serial.print(bytesConverter(ESP.getFlashChipSize(), 'M'));
    Serial.println(F(" MB)"));

    Serial.print(F("Flash Chip Speed: "));
    Serial.print(ESP.getFlashChipSpeed());
    Serial.println(F(" Hz"));

    Serial.print(F("Cycle Count: "));
    Serial.println(ESP.getCycleCount());
}

/******************************************************************************/

void DCPSerialCommands::wifiComm(String serialCommand) {
    String args = getArguments(serialCommand, 1);
    if (args != "") {
        String madatoryArg = getArguments(args, 0, ' ');
        String value = getArguments(args, 1, ' ');
        value = getArguments(value, 1, '"');
        if (madatoryArg == "r") {
            //reset
            serialWifi.deleteWifiCredentials();
            Serial.println(F("Reset WIFI configuration is done!"));
        } else if (madatoryArg == "d") {
            //show available ssid
            String result = serialWifi.scanNetworks();
            Serial.println(result);
        } else if (madatoryArg == "s") {
            //set ssid
            serialWifi.setSSID(value);
            Serial.println(F("Set SSID is done!"));
        } else if (madatoryArg == "p") {
            //set pass
            serialWifi.setPWD(value);
            Serial.println(F("Set Password is done!"));
        } else {
            Serial.print(F("ERROR! Argument not recognized: "));
            Serial.println(madatoryArg);
        }
    } else {
        printSystemWiFiStatus();
    }
}

/**
 * Print system environment status to serial
 */
void DCPSerialCommands::printSystemWiFiStatus() {

    Serial.println(F("\nSYSTEM WIFI STATUS"));
    Serial.println(F("----------------------------------------------------------"));

    Serial.print(F("Status:                                "));
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("CONNECTED - Wi-Fi connection successful established."));
    } else if (WiFi.status() == WL_NO_SSID_AVAIL) {
        Serial.println(F("NO SSID AVAILABLE - Configured SSID cannot be reached."));
    } else if (WiFi.status() == WL_CONNECT_FAILED) {
        Serial.println(F("CONNECT FAILED - Password is incorrect."));
    } else if (WiFi.status() == WL_IDLE_STATUS) {
        Serial.println(F("IDLE STATUS - Wi-Fi is in process of changing between statuses."));
    } else if (WiFi.status() == WL_DISCONNECTED) {
        Serial.println(F("DISCONNECTED - Wi-Fi module is not configured in station mode."));
    } else {
        Serial.println(F(""));
    }

    Serial.print(F("SSID:                                  "));
    Serial.println(WiFi.SSID().c_str());

    Serial.print(F("MAC address:                           "));
    Serial.println(WiFi.macAddress().c_str());

    Serial.print(F("Local Ip:                              "));
    Serial.println(WiFi.localIP());

    Serial.print(F("Signal Quality:                        "));
    Serial.print(serialWifi.getSignalQuality());
    Serial.println(F("%"));

    Serial.print(F("Subnet Mask:                           "));
    Serial.println(WiFi.subnetMask());

    Serial.print(F("Gateway IP:                            "));
    Serial.println(WiFi.gatewayIP());

    Serial.print(F(""));
}

/*******************************************************************************/

void DCPSerialCommands::simComm(String serialCommand) {
    String args = getArguments(serialCommand, 1);
    if (args != "") {
        String madatoryArg = getArguments(args, 0, ' ');
        String value = getArguments(args, 1, ' ');
        value = getArguments(value, 1, '"');
        if (madatoryArg == "on") {
            //Turn on
            serialSIM800.setupSIM800Module();
        } else if (madatoryArg == "off") {
            //Turn off
            serialSIM800.turnOff();
            Serial.println(F("Modem turnoff is done!"));
        } else if (madatoryArg == "r") {
            //Reset
            serialSIM800.resetConfig();
            Serial.println(F("Reset SIM configuration is done!"));
        } else if (madatoryArg == "a") {
            //Set a new Carrier APN
            serialSIM800.setAPN(value);
            Serial.println(F("Set APN is done!"));
        } else if (madatoryArg == "u") {
            //Set a new User.
            serialSIM800.setUSER(value);
            Serial.println(F("Set USER is done!"));
        } else if (madatoryArg == "p") {
            //Set a new Password.
            serialSIM800.setPWD(value);
            Serial.println(F("Set PWD is done!"));
        } else if (madatoryArg == "c") {
            //Send AT command
            String result = serialSIM800.sendAT(value);
            Serial.print(F("Command: AT"));
            Serial.println(value);
            Serial.print(F("Return: "));
            Serial.println(result);
        } else {
            Serial.print(F("ERROR! Argument not recognized: "));
            Serial.println(madatoryArg);
        }
    } else {
        printSystemSimStatus();
    }
}

/**
 * Print system environment status to serial
 */
void DCPSerialCommands::printSystemSimStatus() {

    Serial.println(F("\nSYSTEM SIM CARD STATUS"));
    Serial.println(F("----------------------------------------------------------"));

    bool res = serialSIM800.isConnected();
    Serial.print(F("GPRS status:                           "));
    Serial.println(res ? "CONNECTED" : "DISCONNECTED");

    if (res) {
        String ccid = serialSIM800.getSimCCID();
        Serial.print(F("CCID:                                  "));
        Serial.println(ccid);

        String imei = serialSIM800.getIMEI();
        Serial.print(F("IMEI:                                  "));
        Serial.println(imei);

        String imsi = serialSIM800.getIMSI();
        Serial.print(F("IMSI:                                  "));
        Serial.println(imsi);

        String cop = serialSIM800.getOperator();
        Serial.print(F("Operator:                              "));
        Serial.println(cop);

        IPAddress local = serialSIM800.getLocalIP();
        Serial.print(F("Local IP:                              "));
        Serial.println(local);

        String csq = serialSIM800.getSignalQuality();
        Serial.print(F("Signal Quality:                        "));
        Serial.print(csq);
        Serial.println(F("%"));
    }

    // Get MQTT Host
    String apn = commSpiffsManager.getSettings(F("SIM Carrier APN"), DIR_SIMCARD_APN, true);
    Serial.print(F("APN:                                   "));
    Serial.println(apn);

    // Get MQTT User
    String user = commSpiffsManager.getSettings(F("SIM Carrier APN User"), DIR_SIMCARD_USER, false);
    Serial.print(F("USER:                                  "));
    Serial.println(user);

    // Get MQTT Password
    String pwd = commSpiffsManager.getSettings(F("SIM  Carrier APN Pwd"), DIR_SIMCARD_PWD, true);
    Serial.print(F("PWD:                                   "));
    Serial.println(pwd);
}

/******************************************************************************/

void DCPSerialCommands::weatherComm(String serialCommand) {
    String args = getArguments(serialCommand, 1);
    if (args != "") {
        String madatoryArg = getArguments(args, 0, ' ');
        String value = getArguments(args, 1, ' ');
        value = getArguments(value, 1, '"');
        if (madatoryArg == "ht") {
            //temperature humidity
            Serial.println(serialDHT.printDHT());
        } else if (madatoryArg == "r") {
            //tip bucket count
            Serial.print("Rain Gauge Tip Bucket: ");
            Serial.println(serialdcpRainGauge.printTipBucket());
        } else if (madatoryArg == "b") {
            //battery voltage
            Serial.print("Battery voltage: ");
            Serial.println(serialdcpVoltage.printVccIn());
        } else if (madatoryArg == "s") {
            //solar cell
            Serial.print("Solar cell voltage: ");
            Serial.println(serialdcpVoltage.printVccSol());

        } else {
            Serial.print(F("ERROR! Argument not recognized: "));
            Serial.println(madatoryArg);
        }
    } else {
        printSystemWeathers();
    }
}

/**
 * Print system environment status to serial
 */
void DCPSerialCommands::printSystemWeathers() {
    Serial.println(F("\nSYSTEM WEATHER ACTUAL VALUES"));
    Serial.println(F("----------------------------------------------------------"));
    //temperature //humidity
    Serial.println(serialDHT.printDHT());
    //tip bucket count
    Serial.print("Rain Gauge Tip Bucket: ");
    Serial.println(serialdcpRainGauge.printTipBucket());
    //battery voltage
    Serial.print("Battery voltage: ");
    Serial.println(serialdcpVoltage.printVccIn());
    //solar cell
    Serial.print("Solar cell voltage: ");
    Serial.println(serialdcpVoltage.printVccSol());
}

/******************************************************************************/

void DCPSerialCommands::lsComm(String serialCommand) {
    String args = getArguments(serialCommand, 1);
    if (args != "") {
        String madatoryArg = getArguments(args, 0, ' ');
        String value = getArguments(args, 1, ' ');
        value = getArguments(value, 1, '"');
        if (madatoryArg == "u") {
            Serial.println(F("\nUPDATE FILES LIST"));
            Serial.println(F("----------------------------------------------------------"));
            serialSDCard.printDirectory("update");
        } else {
            Serial.print(F("ERROR! Argument not recognized: "));
            Serial.println(madatoryArg);
        }
    } else {
        Serial.println(F("\nWEATHER FILES LIST"));
        Serial.println(F("----------------------------------------------------------"));
        serialSDCard.printDirectory("/");
    }
}

void DCPSerialCommands::catComm(String serialCommand) {
    String args = getArguments(serialCommand, 1);
    if (args != "") {
        String madatoryArg = getArguments(args, 0, ' ');
        String value = getArguments(args, 1, ' ');
        value = getArguments(value, 1, '"');
        if (madatoryArg == "f") {
            Serial.print(F("\n"));
            Serial.print(value);
            Serial.println(F(" CONTENT"));
            Serial.println(F("----------------------------------------------------------"));
            serialSDCard.printContentFile(value);
        } else {
            Serial.print(F("ERROR! Argument not recognized: "));
            Serial.println(madatoryArg);
        }
    }
}

void DCPSerialCommands::fsstatusComm() {

    Serial.println(F("\nWEATHER FILES STATUS"));
    Serial.println(F("----------------------------------------------------------"));

    Serial.println(serialSDCard.getCardType());
    Serial.println(serialSDCard.cidDmp());
    Serial.println(serialSDCard.csdDmp());
    Serial.println(serialSDCard.dmpVol());
}

void DCPSerialCommands::clearSerialInput() {
    uint32_t m = micros();
    do {
        if (Serial.read() >= 0) {
            m = micros();
        }
    } while (micros() - m < 10000);
}

String DCPSerialCommands::padL(int len, String inS) {
    char buffer[len];
    String format = "%" + String(len) + "s";
    sprintf(buffer, format.c_str(), inS.c_str());
    return buffer;
}

