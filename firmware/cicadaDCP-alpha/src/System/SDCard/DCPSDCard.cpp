/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */
#include "DCPSDCard.h"

// SdFat software SPI template
SoftSpiDriver<PIN_SDCARD_MISO, PIN_SDCARD_MOSI, PIN_SDCARD_SCK> softSpi;
// Speed argument is ignored for software SPI.
#define SD_CONFIG SdSpiConfig(PIN_SDCARD_CHIP_SELECT, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)

DCPRTC sdRTC;

//Mutex
SemaphoreHandle_t SDMutex = xSemaphoreCreateMutex();

DCPSDCard::DCPSDCard() {
}

boolean DCPSDCard::takeSDMutex(String function) {
    return (xSemaphoreTake(SDMutex, 1) == pdTRUE);
}

void DCPSDCard::giveSDMutex(String function) {
    xSemaphoreGive(SDMutex);
}

//------------------------------------------------------------------------------
// Call back for file timestamps.  Only called for file create and sync().

void dateTime(uint16_t* date, uint16_t* time, uint8_t* ms10) {

    uint16_t year;
    uint8_t month, day, hour, minute, second;

    // User gets date and time from GPS or real-time clock here
    struct tm *newtime = sdRTC.convEpoch(sdRTC.nowEpoch());

    year = newtime->tm_year + 1900;
    month = newtime->tm_mon + 1;
    day = newtime->tm_mday;
    hour = newtime->tm_hour;
    minute = newtime->tm_min;
    second = newtime->tm_sec;

    // return date using FAT_DATE macro to format fields
    *date = FAT_DATE(year, month, day);

    // return time using FAT_TIME macro to format fields
    *time = FAT_TIME(hour, minute, second);

    // Return low time bits in units of 10 ms, 0 <= ms10 <= 199.
    *ms10 = second & 1 ? 100 : 0;
}

/**
 * Setup SDCARD module
 */
boolean DCPSDCard::setupSDCardModule() {
    if (!sd.begin(SD_CONFIG)) {
        sd.initErrorHalt();
        CIC_DEBUGWL(F("initialization failed!"));
        return false;
    }

    if (!sd.exists("log")) {
        if (!sd.mkdir("log")) {
            CIC_DEBUGWL(F("Create log folder failed"));
            printSDError();
            return false;
        }
    }

    CIC_DEBUG_HEADERWL(F("Initializing SD card..."));

    FsDateTime::setCallback(dateTime);

    CIC_DEBUGWL(F("initialization done."));
    return true;
}

void DCPSDCard::printDirectory(String path) {
    int attempts = 0;
    File32 dir;
    File32 myFile;

    // Open root directory
    if (!dir.open(path.c_str())) {
        CIC_DEBUGWL(F("Error dir.open failed"));
    }
    dir.rewindDirectory();

    while (myFile.openNext(&dir, O_RDONLY)) {
        char fileName[40];
        int len = 15;
        myFile.getName(fileName, sizeof (fileName));
        if (myFile.isDir()) {
            // Indicate a directory.
            Serial.write('/');
            len = 14;
        }

        CIC_DEBUGWL_(padR(len, fileName));
        myFile.printModifyDateTime(&Serial);
        CIC_DEBUGWL_(" ");
        myFile.printFileSize(&Serial);
        CIC_DEBUGWL(" bytes");

        myFile.close();
    }

    if (dir.getError()) {
        CIC_DEBUGWL(F("OpenNext failed"));
    }
}

boolean DCPSDCard::writeFile(String filename, String content) {
    int attempts = 0;
    while (attempts <= SD_ATTEMPTS) {
        if (takeSDMutex("writeFile")) {
            // open the file. note that only one file can be open at a time,
            // so you have to close this one before opening another.
            //File32 myFile = sd.open(filename.c_str(), FILE_WRITE, dateTime);
            File32 myFile;
            if (myFile.open(filename.c_str(), O_APPEND | O_RDWR | O_CREAT)) {
                myFile.println(content);
                // close the file:
                myFile.close();
                CIC_DEBUGWL_(sdRTC.now());
                CIC_DEBUGWL_(F(" - Write file: "));
                CIC_DEBUGWL(filename);
                CIC_DEBUGWL_("content: ");
                CIC_DEBUGWL(content);
                giveSDMutex("writeFile");
                return true;
            } else {
                // if the file didn't open, print an error:
                CIC_DEBUGWL_(F("Error opening "));
                CIC_DEBUGWL(filename);
            }
            giveSDMutex("writeFile");
        } else {

            CIC_DEBUGWL("Waiting to write File...");
        }
        attempts = attempts + 1;
        vTaskDelay(pdMS_TO_TICKS(SD_ATTEMPTS_DELAY));
    }
    return false;
}

boolean DCPSDCard::mqttPublishFiles(boolean(*callback)(String msg, PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC), PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC) {
    int attempts = 0;
    while (attempts <= SD_ATTEMPTS) {
        if (takeSDMutex("mqttPublishFiles")) {
            File32 dir;
            File32 myFile;
            // Open root directory
            if (!dir.open("/")) {
                CIC_DEBUGWL(F("Dir. open failed"));
            }

            dir.rewindDirectory();
            while (myFile.openNext(&dir, O_RDONLY)) {
                char fileName[40];
                myFile.getName(fileName, sizeof (fileName));
                if (!myFile.isDirectory()) {
                    myFile.close();

                    if (readPublishFile(fileName, callback, _clientPub, tknDCP, pwdDCP, TOPIC)) {
                        if (!dir.remove(fileName)) {
                            CIC_DEBUGWL_("Delete file error: ");
                            CIC_DEBUGWL(fileName);
                        }
                    } else {
                        CIC_DEBUGWL_(F("Error on Mqtt publish: "));
                        CIC_DEBUGWL(fileName);
                    }
                } else {
                    myFile.close();
                }
            }

            boolean result = false;
            if (dir.getError()) {
                CIC_DEBUGWL(F("Mqtt publish failed"));
                result = false;
            } else {
                result = true;
            }
            dir.close();
            giveSDMutex("mqttPublishFiles");
            return result;

        } else {

            CIC_DEBUGWL("Waiting to mqttPublishFiles...");
        }
        attempts = attempts + 1;
        vTaskDelay(pdMS_TO_TICKS(SD_ATTEMPTS_DELAY));
    }
}

boolean DCPSDCard::readPublishFile(String filename, boolean(*callback)(String msg, PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC), PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC) {
    CIC_DEBUGWL_(F("Publishing messages from file "));
    CIC_DEBUGWL(filename.c_str());
    File32 myFile;
    if (myFile.open(filename.c_str(), O_RDONLY)) {
        // read from the file until there's nothing else in it:
        String msg = "";
        int msgNumber = 0;
        while (myFile.available()) {
            char cMsg = (char) myFile.read();
            if (cMsg == '\n') {
                msgNumber += 1;
                CIC_DEBUGWL_(F("Message #"));
                CIC_DEBUGWL_(msgNumber);
                CIC_DEBUGWL_(F(": "));
                (*callback)(msg, _clientPub, tknDCP, pwdDCP, TOPIC);
                msg = "";
            } else {
                msg = msg + cMsg;
            }
        }
        // close the file:
        myFile.close();
        return true;
    } else {
        // if the file didn't open, print an error:
        CIC_DEBUGWL_(F("Error opening "));
        CIC_DEBUGWL(filename);

        return false;
    }
}

String DCPSDCard::readFile(String filename) {
    int attempts = 0;
    while (attempts <= SD_ATTEMPTS) {
        if (takeSDMutex("readFile")) {
            // re-open the file for reading:

            String result = "";
            File32 myFile;
            if (myFile.open(filename.c_str(), O_RDONLY)) {
                // read from the file until there's nothing else in it:
                while (myFile.available()) {
                    result = result + (char) myFile.read();
                }
                // close the file:
                myFile.close();

            } else {
                // if the file didn't open, print an error:
                CIC_DEBUGWL_(F("Error opening: "));
                CIC_DEBUGWL(filename);
            }
            giveSDMutex("readFile");
            return result;
        } else {

            CIC_DEBUGWL_(F("Waiting to read File: "));
            CIC_DEBUGWL(filename);
        }
        attempts = attempts + 1;
        vTaskDelay(pdMS_TO_TICKS(SD_ATTEMPTS_DELAY));
    }
    return "";
}

void DCPSDCard::printContentFile(String filename) {
    int attempts = 0;
    while (attempts <= SD_ATTEMPTS) {
        if (takeSDMutex("printContentFile")) {
            // re-open the file for reading:
            File32 myFile;
            if (myFile.open(filename.c_str(), O_RDONLY)) {
                // read from the file until there's nothing else in it:
                while (myFile.available()) {
                    CIC_DEBUGWL_((char) myFile.read());
                }
                // close the file:
                myFile.close();
            } else {
                // if the file didn't open, print an error:
                CIC_DEBUGWL_(F("Error opening "));
                CIC_DEBUGWL(filename);
            }
            giveSDMutex("printContentFile");
            break;
        } else {

            CIC_DEBUGWL("Waiting to read File...");
        }
        attempts = attempts + 1;
        vTaskDelay(pdMS_TO_TICKS(SD_ATTEMPTS_DELAY));
    }
}

boolean DCPSDCard::deleteFile(String filename) {
    int attempts = 0;
    while (attempts <= SD_ATTEMPTS) {
        CIC_DEBUGWL_(F("Delete file "));
        CIC_DEBUGWL(filename);
        if (takeSDMutex("deleteFile")) {
            if (!sd.chdir("/")) {
                CIC_DEBUGWL("Error chdir to root failed.\n");
            }

            if (sd.exists(filename.c_str())) {
                if (!sd.remove(filename.c_str())) {
                    CIC_DEBUGWL_("Error remove file: ");
                    CIC_DEBUGWL(filename);
                    giveSDMutex("deleteFile");
                    return false;
                } else {
                    CIC_DEBUGWL_(F("Delete file successful "));
                    CIC_DEBUGWL(filename);
                    giveSDMutex("deleteFile");
                    return true;
                }
            } else {
                CIC_DEBUGWL_(F("Delete fail: File not exists: "));
                CIC_DEBUGWL(filename);
                giveSDMutex("deleteFile");
            }
        } else {

            CIC_DEBUGWL_("Waiting to delete File: ");
            CIC_DEBUGWL(filename);
        }
        attempts = attempts + 1;
        vTaskDelay(pdMS_TO_TICKS(SD_ATTEMPTS_DELAY));
    }
}

void DCPSDCard::deleteOldFiles(String path) {
    int attempts = 0;
    String text = "files";
    if (path != "/") {
        text = path;
    }
    CIC_DEBUGWL_(F("Delete "));
    CIC_DEBUGWL_(text);
    CIC_DEBUGWL(F(" older than 1 year."));
    while (attempts <= SD_ATTEMPTS) {
        if (takeSDMutex("deleteOldFiles")) {
            File32 myDir;
            File32 myFile;
            // Open root directory
            if (!myDir.open(path.c_str())) {
                CIC_DEBUGWL(F("Error dir.open failed"));
            }
            myDir.rewindDirectory();

            while (myFile.openNext(&myDir, O_RDONLY)) {
                if (!myFile.isDirectory()) {
                    char fileName[40];
                    myFile.getName(fileName, sizeof (fileName));
                    uint16_t date;
                    uint16_t time;
                    myFile.getModifyDateTime(&date, &time);
                    //%Y-%m-%dT%H:%M:%SZ
                    String datetime = String(SDFAT_YEAR(date)) + "-" + String(SDFAT_MONTH(date)) + "-" + String(SDFAT_DAY(date)) + "T" + String(SDFAT_HOUR(time)) + ":" + String(SDFAT_MINUTE(time)) + ":" + String(SDFAT_SECOND(time)) + "Z";
                    time_t ttFile = sdRTC.stringToTime(datetime);

                    time_t ttNow = sdRTC.nowEpoch();
                    //files older than 1 year.
                    time_t ttOlder = ttNow - ((60 * 60 * 24)*365);

                    myFile.close();

                    if (ttFile < ttOlder) {
                        if (myDir.remove(fileName)) {
                            CIC_DEBUGWL_(F("Old file delete: "));
                            CIC_DEBUGWL(fileName);
                        } else {
                            CIC_DEBUGWL_(F("Delete file error: "));
                            CIC_DEBUGWL(fileName);
                        }
                    }
                } else {
                    myFile.close();
                }
            }

            if (myDir.getError()) {
                CIC_DEBUGWL(F("deleteOldFiles: OpenNext failed"));
            }
            myDir.close();
            giveSDMutex("deleteOldFiles");
            break;
        } else {

            CIC_DEBUGWL(F("Waiting to deleteOldFiles..."));
        }
        attempts = attempts + 1;
        vTaskDelay(pdMS_TO_TICKS(SD_ATTEMPTS_DELAY));
    }
}

void DCPSDCard::cleanOlderFiles() {
    deleteOldFiles();
    deleteOldFiles("log");
}

String DCPSDCard::prepareData(String sensorCode, String dataType, String collectionDate, String value) {

    return "{\"snsEC\":" + sensorCode + ",\"dtT\":\"" + dataType + "\",\"colDT\":\"" + collectionDate + "\",\"val\":\"" + value + "\"}";
}

boolean DCPSDCard::storeData(String sensor, String measures) {
    String content = "\"measures\":[" + measures + "]";
    String filename = sdRTC.now("%Y%m%d") + "." + sensor;
    writeFile(filename, content);

    return true;
}

String DCPSDCard::prepareDataMetadata(String dataType, String collectionDate, String value, String context) {
    String cxt = "";
    if (context != "") {
        cxt = ",\"context\":" + context;
    }
    return "{\"dtT\":\"" + dataType + "\",\"colDT\":\"" + collectionDate + "\",\"val\":\"" + value + "\"" + cxt + "}";
}

boolean DCPSDCard::storeMetadadosStation(String la, String lo, String bucket, String comType, String simICCID, String simOpera, String comLocalIP, String comSQ) {
    CIC_DEBUGWL(F("Store Metadata!"));

    String filename = sdRTC.now("%Y%m%d") + ".mtd";

    String collectionDate = sdRTC.now("%Y-%m-%d %H:%M:%SZ");

    //part 1
    String dataContent = prepareDataMetadata("la", collectionDate, la);
    dataContent += "," + prepareDataMetadata("lo", collectionDate, lo);

    String content = "\"metadata\":[" + dataContent + "]";
    writeFile(filename, content);


    //part 2
    dataContent = prepareDataMetadata("bkt", collectionDate, bucket);
    content = "\"metadata\":[" + dataContent + "]";
    writeFile(filename, content);


    //part 3
    String context = "\"{'cty':'" + comType + "','icc':'" + simICCID + "','lip':'" + comLocalIP + "'}\"";
    dataContent = prepareDataMetadata("cqs", collectionDate, comSQ, context);

    content = "\"metadata\":[" + dataContent + "]";
    writeFile(filename, content);

    return true;
}

/*******************************************************************************
 *                        SERIAL COMMANDS                                      *
 *******************************************************************************/

void DCPSDCard::formatSD(boolean ask) {
    int attempts = 0;
    while (attempts <= SD_ATTEMPTS) {
        if (takeSDMutex("formatSD")) {
            DCPSDFormatter::formatSD(ask);
            setupSDCardModule();
            giveSDMutex("formatSD");
            break;
        } else {

            Serial.println(F("Waiting to formatSD..."));
        }
        attempts = attempts + 1;
        vTaskDelay(pdMS_TO_TICKS(SD_ATTEMPTS_DELAY));
    }
}

String DCPSDCard::getCardType() {
    // SdCardFactory constructs and initializes the appropriate card.
    SdCardFactory cardFactory;
    SdCard* m_card = nullptr;
    // Select and initialize proper card driver.
    m_card = cardFactory.newCard(SD_CONFIG);

    csd_t m_csd;
    if (!m_card->readCSD(&m_csd)) {
        return "ReadInfo failed: " + sd.sdErrorCode();
    }

    String result = F("Card type                :");

    switch (m_card->type()) {
        case SD_CARD_TYPE_SD1:
            result += padL(10, F("SD1"));
            break;
        case SD_CARD_TYPE_SD2:
            result += padL(10, F("SD2"));
            break;
        case SD_CARD_TYPE_SDHC:
            if (sdCardCapacity(&m_csd) < 70000000) {
                result += padL(10, F("SDHC"));
            } else {

                result += padL(10, F("SDXC"));
            }
            break;
        default:
            result += padL(10, F("Unknown"));
    }
    return result;
}

String DCPSDCard::cidDmp() {
    // SdCardFactory constructs and initializes the appropriate card.
    SdCardFactory cardFactory;
    SdCard* m_card = nullptr;
    // Select and initialize proper card driver.
    m_card = cardFactory.newCard(SD_CONFIG);

    cid_t m_cid;
    if (!m_card->readCID(&m_cid)) {
        return "ReadInfo failed: " + sd.sdErrorCode();
    }

    String result = "";
    result += F("\nManufacturer\n");
    result += F("----------------------------------------------------------\n");
    result += F("Manufacturer ID          :");
    String mmHex = String("0X" + String(int(m_cid.mid), HEX));
    mmHex.toUpperCase();
    result += padL(10, mmHex);
    result += F("\n");
    result += F("OEM ID                   :");
    result += padL(10, String(m_cid.oid[0]) + String(m_cid.oid[1]));
    result += F("\n");
    result += F("Product                  :");
    result += padL(10, String(m_cid.pnm[0]) + String(m_cid.pnm[1]) + String(m_cid.pnm[2]) + String(m_cid.pnm[3]) + String(m_cid.pnm[4]));
    result += F("\n");
    result += F("Version                  :");
    result += padL(10, String(int(m_cid.prv_n)) + '.' + String(int(m_cid.prv_m)));
    result += F("\n");
    result += F("Serial number            :");
    String snHex = String("0X" + String(int(m_cid.psn), HEX));
    snHex.toUpperCase();
    result += padL(10, snHex);
    result += F("\n");
    result += F("Manufacturing date       :");
    result += padL(10, String(int(m_cid.mdt_month)) + '/' + String((2000 + m_cid.mdt_year_low + 10 * m_cid.mdt_year_high)));

    return result;
}

String DCPSDCard::csdDmp() {
    bool eraseSingleBlock;
    uint32_t m_eraseSize;

    // SdCardFactory constructs and initializes the appropriate card.
    SdCardFactory cardFactory;
    SdCard* m_card = nullptr;
    // Select and initialize proper card driver.
    m_card = cardFactory.newCard(SD_CONFIG);

    csd_t m_csd;
    if (!m_card->readCSD(&m_csd)) {
        return "ReadInfo failed: " + sd.sdErrorCode();
    }

    if (m_csd.v1.csd_ver == 0) {
        eraseSingleBlock = m_csd.v1.erase_blk_en;
        m_eraseSize = (m_csd.v1.sector_size_high << 1) | m_csd.v1.sector_size_low;
    } else if (m_csd.v2.csd_ver == 1) {
        eraseSingleBlock = m_csd.v2.erase_blk_en;
        m_eraseSize = (m_csd.v2.sector_size_high << 1) | m_csd.v2.sector_size_low;
    } else {
        return F("m_csd version error");
    }

    m_eraseSize++;

    String result = "";

    result += F("\nSDCards Blocks\n");
    result += F("----------------------------------------------------------\n");

    float volumesize = 0.000512 * sdCardCapacity(&m_csd);
    result += F("Card Size (Mb)           :");
    result += padL(10, String(volumesize));
    result += F("Mb");
    result += F("\n");
    result += F("Card Size (Gb)           :");
    result += padL(10, String((float) volumesize / 1024.00));
    result += F("Gb");
    result += F("\n");


    result += F("Flash Erase Size (bloks) :");
    result += padL(10, String(int(m_eraseSize)));
    result += F("\n");

    result += F("Erase Single Block       :");

    if (eraseSingleBlock) {
        result += padL(11, String(F("true\n")));
    } else {
        result += padL(11, String(F("false\n")));
    }

    if (sd.dataStartSector() % m_eraseSize) {

        result += F("ALERT: ");
        result += F("Data area is not aligned on flash erase boundary!\n");
        result += F("Consider running the fsformat command.!\n");
    }

    return result;
}

String DCPSDCard::dmpVol() {
    String result = "";

    if (!sd.cardBegin(SD_CONFIG)) {
        Serial.println(F(
                "\nSD initialization failed.\n"
                "Do not reformat the card!\n"
                "Is the card correctly inserted?\n"
                "Is there a wiring/soldering problem?\n"));
        printSDError();
        return "";
    }


    if (!sd.volumeBegin()) {
        Serial.println(F("\nvolumeBegin failed. Is the card formatted?"));
        printSDError();
        return result;
    }

    Serial.println(F("Scanning FAT, please wait."));

    result += F("\nVolume Info\n");
    result += F("----------------------------------------------------------\n");

    uint32_t freeClusterCount = sd.freeClusterCount();
    if (sd.fatType() <= 32) {
        result += F("Volume is FAT");
        result += int(sd.fatType());
        result += F("\n");
    } else {
        result += F("Volume is exFAT\n");
    }
    result += F("sectorsPerCluster        :");
    result += padL(10, String(sd.sectorsPerCluster()));
    result += F("\n");
    result += F("clusterCount             :");
    result += padL(10, String(sd.clusterCount()));
    result += F("\n");
    result += F("freeClusterCount         :");
    result += padL(10, String(freeClusterCount));
    result += F("\n");
    result += F("fatStartSector           :");
    result += padL(10, String(sd.fatStartSector()));
    result += F("\n");
    result += F("dataStartSector          :");
    result += padL(10, String(sd.dataStartSector()));
    result += F("\n");
    result += F("512 byte blocks          :");
    result += padL(10, String(sd.card()->cardSize()));
    result += F("\n");
    result += F("blocksPerCluster         :");
    result += padL(10, String(sd.vol()->blocksPerCluster()));
    result += F("\n\n");

    float volumesize = sd.clusterCount() * 0.000512 * sd.sectorsPerCluster();
    float freeSpace = freeClusterCount * 0.000512 * sd.sectorsPerCluster();
    float used = volumesize - freeSpace;

    result += F("Space\n");
    result += F("----------------------------------------------------------\n");

    result += F("Real card Size (Mb)      :");
    result += (padL(10, String(volumesize)));
    result += F("Mb");
    result += F("\n");
    result += F("Real card Size (Gb)      :");
    result += (padL(10, String((float) volumesize / 1024.00)));
    result += F("Gb");
    result += F("\n\n");

    result += F("Used (Mb)                :");
    result += (padL(10, String(used)));
    result += F("Mb");
    result += F("\n");
    result += F("Used (Gb)                :");
    result += (padL(10, String((float) used / 1024.00)));
    result += F("Gb");
    result += F("\n\n");

    result += F("Free (Mb)                :");
    result += (padL(10, String(freeSpace)));
    result += F("Mb");
    result += F("\n");
    result += F("Free (Gb)                :");
    result += (padL(10, String((float) freeSpace / 1024.00)));
    result += F("Gb");
    result += F("\n");

    return result;
}

String DCPSDCard::padL(int len, String inS) {
    char buffer[len];
    String format = "%" + String(len) + "s";
    sprintf(buffer, format.c_str(), inS.c_str());

    return buffer;
}

String DCPSDCard::padR(int len, String inS) {
    char buffer[len];
    int strLen = len;
    strLen -= inS.length();
    String format = "%s %" + String(strLen) + "s";
    sprintf(buffer, format.c_str(), inS.c_str(), " ");

    return buffer;
}

/******************************************************************************/
boolean DCPSDCard::printSDError() {
    if (sd.sdErrorCode()) {

        Serial.print(F("SD errorCode: "));
        printSdErrorSymbol(&Serial, sd.sdErrorCode());
        Serial.print(F(" = "));
        Serial.println(int(sd.sdErrorCode()));
        Serial.print(F("SD errorData = "));
        Serial.println(int(sd.sdErrorData()));
    }
}

/*******************************************************************************
 *                                 LOG                                         *
 *******************************************************************************/

boolean DCPSDCard::writeLog(String log, boolean ln) {
    String filename = "log/";
    filename += sdRTC.now("%Y%m%d") + ".lc" + xPortGetCoreID();
    int attempts = 0;
    while (attempts <= SD_ATTEMPTS) {
        if (takeSDMutex("writeLog")) {

            // Change volume working directory to root.
            if (!sd.chdir()) {
                Serial.println(F("chdir failed for root Folder."));
                printSDError();
                giveSDMutex("writeLog");
                return false;
            }

            if (!sd.exists("log")) {
                if (!sd.mkdir("log")) {
                    Serial.println(F("Create log folder failed"));
                    printSDError();
                    giveSDMutex("writeLog");
                    return false;
                }
            }

            // Change volume working directory to log.
            if (!sd.chdir("log")) {
                Serial.println(F("chdir failed for log Folder."));
                printSDError();
                giveSDMutex("writeLog");
                return false;
            }

            // open the file. note that only one file can be open at a time,
            // so you have to close this one before opening another.
            File32 myFile;
            if (myFile.open(filename.c_str(), O_APPEND | O_RDWR | O_CREAT)) {
                String logContent = "";
                if (ctrlLog == 1) {
                    logContent += sdRTC.now();
                    logContent += ": ";
                }
                logContent += log;

                if (ln) {
                    myFile.println(logContent);
                    ctrlLog = 1;
                } else {
                    myFile.print(logContent);
                    ctrlLog++;
                }

                // close the file:
                myFile.close();
                // Change volume working directory to root.
                if (!sd.chdir()) {
                    Serial.println(F("chdir failed for root Folder."));
                    printSDError();
                    giveSDMutex("writeLog");
                    return false;
                }

                giveSDMutex("writeLog");
                return true;
            } else {
                // if the file didn't open, print an error:
                Serial.print(F("Error opening log "));
                Serial.println(filename);
                printSDError();
                // close the file:
                myFile.close();
                giveSDMutex("writeLog");
                return false;
            }

            // Change volume working directory to root.
            if (!sd.chdir()) {
                Serial.print(F("chdir failed for root Folder."));
                printSDError();
            }
            giveSDMutex("writeLog");
        } else {
            //Serial.println("Waiting to writeLog...");
        }
        attempts = attempts + 1;
        vTaskDelay(pdMS_TO_TICKS(SD_ATTEMPTS_DELAY));
    }

    return false;
}