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

DCPSerialCommands::DCPSerialCommands() {
}

void DCPSerialCommands::initSerialCommands(String firmware) {
    FIRMWARE = firmware;
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
void DCPSerialCommands::readSerialCommands() {

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
        } else if (mandatoryCommand == "status") {
            statusComm();
        } else if (mandatoryCommand == "wifi") {
            wifiComm(serialCommand);
        } else if (mandatoryCommand == "sim") {
            simComm(serialCommand);
        } else if (mandatoryCommand == "ls") {
            lsComm();
        } else if (mandatoryCommand == "cat") {
            catComm(serialCommand);
        } else if (mandatoryCommand == "fsstatus") {
            fsstatusComm();
        } else if (mandatoryCommand == "weather") {
            weatherComm(serialCommand);
        }//        else if (serialCommand == "reset")
            //        {
            //            fullReset();
            //        }
            //        else if (serialCommand == "status")
            //        {
            //            printSystemStatus();
            //        }
            //        else if (serialCommand == "resetwifi")
            //        {
            //            resetWiFiSettings();
            //        }
            //        else if (serialCommand == "filelist")
            //        {
            //            FSPrintFileList();
            //        }
            //        else if (serialCommand == "fsformat")
            //        {
            //            FSFormat();
            //        }
            //        else if (serialCommand == "cleardatadir")
            //        {
            //
            //            CIC_DEBUG_HEADER(F("DELETING WEATHER DATA"));
            //            FSDeleteFiles(DIR_WEATHER_DATA);
            //        }
            //        else if (serialCommand == "fsstatus")
            //        {
            //            printFileSystemStatus();
            //        }
            //        else if (serialCommand == "dump")
            //        {
            //            dump();
            //        }
            //        else if (serialCommand == "deletedata")
            //        {
            //            deleteWeatherData();
            //        }
        else {

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

    Serial.println(F("======================================================"));
    Serial.println(F("           CICADA DATA COLLECTION PLATFORM            "));
    Serial.print(F("                 Version: "));
    Serial.println(FIRMWARE);
    Serial.println(F("      https://github.com/andreivo/CicadaProject       "));
    Serial.println(F("======================================================"));
    Serial.println(F(""));
    Serial.println(F("SYSTEM AVALIABLE COMMANDS"));
    Serial.println(F("------------------------------------------------------"));

    Serial.println(F("help              Show all available commands"));

    Serial.println(F("\nSystem commands:"));
    Serial.println(F("----------------"));
    Serial.println(F("reboot             Reboot system."));
    Serial.println(F("time               Print the current system datetime."));
    Serial.println(F("   -s \"[value]\"    Set a new system datetime. Format value YYYY-mm-ddTHH:MM:SSZ."));
    Serial.println(F("status             Print the current system status to serial monitor including,"));
    Serial.println(F("                     wifi connectivity, Sim connectivity, etc."));
    Serial.println(F("factoryreset       Reset system completely, format file system, reset message,"));
    Serial.println(F("                     delete weather data. (WARNING: cannot be undone)"));
    Serial.println(F("sconfig            Print the current system configuration."));
    Serial.println(F("  -\"[var]\" \"[val]\" Update the system variable configuration."));
    Serial.println(F("                     The [var] indicates the variable to be updated."));
    Serial.println(F("                     The [val] indicates a new value."));

    Serial.println(F("\nWifi system commands:"));
    Serial.println(F("---------------------"));
    Serial.println(F("wifi               Print the current system status."));
    Serial.println(F("   -r                Reset WiFi appliance settings, SSID and password."));
    Serial.println(F("   -d                Print available SSID."));
    Serial.println(F("   -s \"[value]\"    Set a new SSID."));
    Serial.println(F("   -p \"[value]\"    Set a new Password."));


    Serial.println(F("\nSim Card system commands:"));
    Serial.println(F("-------------------------"));
    Serial.println(F("sim               Print the current system status."));
    Serial.println(F("   -on               Moden turn on."));
    Serial.println(F("   -off              Moden turn off."));
    Serial.println(F("   -r                Reset SIM configuration. (WARNING: This action disconnect to the wifi.)"));
    Serial.println(F("   -a \"[value]\"    Set a new Carrier APN."));
    Serial.println(F("   -u \"[value]\"    Set a new User."));
    Serial.println(F("   -p \"[value]\"    Set a new Password."));
    Serial.println(F("   -c AT\"[value]\"  Send AT command. Do not include the AT prefix."));

    Serial.println(F("\nWeather files commands:"));
    Serial.println(F("---------------------"));
    Serial.println(F("ls                 Print the current weather file list."));
    Serial.println(F("cat -f \"[file]\"  Print the file content."));
    Serial.println(F("fsformat           Format weather file system. (WARNING: cannot be undone)"));
    Serial.println(F("fsstatus           Print the current weather file system status."));

    Serial.println(F("\nWeather data commands:"));
    Serial.println(F("----------------------"));
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
    Serial.print("Actual: ");
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
    Serial.println(F("-------------------------"));

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
    Serial.println(F("------------------"));

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

    Serial.print(F("RSSI (dBm):                            "));
    Serial.print(WiFi.RSSI());
    Serial.println(F(" (Signal Strength)"));

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
    Serial.println(F("----------------------"));

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
        Serial.print(F("Signal quality:                        "));
        Serial.println(csq);
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
    Serial.println(F("----------------------------"));
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

void DCPSerialCommands::lsComm() {
    Serial.println(F("\nWEATHER FILES LIST"));
    Serial.println(F("----------------------------"));
    serialSDCard.printDirectory("/", 0);
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
            Serial.println(F("----------------------------"));
            serialSDCard.printContentFile(value);
        } else {
            Serial.print(F("ERROR! Argument not recognized: "));
            Serial.println(madatoryArg);
        }
    }
}

void DCPSerialCommands::fsstatusComm() {

    Serial.println(F("\nWEATHER FILES STATUS"));
    Serial.println(F("----------------------------"));

    Serial.print("Clusters:          ");
    Serial.println(serialSDCard.clusterCount());
    Serial.print("Blocks x Cluster:  ");
    Serial.println(serialSDCard.blocksPerCluster());

    Serial.print("Total Blocks:      ");
    Serial.println(serialSDCard.blocksPerCluster() * serialSDCard.clusterCount());
    Serial.println();

    // print the type and size of the first FAT-type volume
    float volumesize;
    float usedSpace = serialSDCard.usedSpace();
    usedSpace /= 1000.00;


    Serial.print("Volume type is:    FAT");
    Serial.println(serialSDCard.fatType(), DEC);
    volumesize = serialSDCard.blocksPerCluster(); // clusters are collections of blocks
    volumesize *= serialSDCard.clusterCount(); // we'll have a lot of clusters
    volumesize /= 2.00; // SD card blocks are always 512 bytes (2 blocks are 1KB)
    float freeSpace = (float) volumesize - (float) usedSpace;

    Serial.print("Volume size (Kb):  ");
    Serial.println(volumesize);
    Serial.print("Volume size (Mb):  ");
    volumesize /= 1024.00;
    Serial.println(volumesize);
    Serial.print("Volume size (Gb):  ");
    Serial.println((float) volumesize / 1024.00);

    Serial.print("Used (Kb)       :  ");
    Serial.println(usedSpace);
    Serial.print("Used (Mb)       :  ");
    usedSpace /= 1024.00;
    Serial.println(usedSpace);
    Serial.print("Used (Gb)       :  ");
    Serial.println((float) usedSpace / 1024.00);

    Serial.print("Available (Kb)  :  ");
    Serial.println(freeSpace);
    Serial.print("Available (Mb)  :  ");
    freeSpace /= 1024.0;
    Serial.println(freeSpace);
    Serial.print("Available (Gb)  :  ");
    Serial.println((float) freeSpace / 1024.00);



}