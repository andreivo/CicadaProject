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

DCPRTC sdRTC;

//Mutex
SemaphoreHandle_t SDMutex = xSemaphoreCreateMutex();

DCPSDCard::DCPSDCard() {
}

boolean DCPSDCard::takeSDMutex() {
    //CIC_DEBUG("Get SDMutex");
    return (xSemaphoreTake(SDMutex, 1) == pdTRUE);
}

void DCPSDCard::giveSDMutex() {
    //CIC_DEBUG("Give SDMutex");
    xSemaphoreGive(SDMutex);
}

/**
 * Setup SDCARD module
 */
boolean DCPSDCard::setupSDCardModule() {
    CIC_DEBUG_HEADER(F("Initializing SD card..."));
    // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
    // Note that even if it's not used as the CS pin, the hardware SS pin
    // (10 on Arduino Uno boards, 53 on the Mega) must be left as an output
    // or the SD library functions will not work.
    pinMode(SS, OUTPUT);

    if (!SD.begin(PIN_SDCARD_CHIP_SELECT, PIN_SDCARD_MOSI, PIN_SDCARD_MISO, PIN_SDCARD_SCK)) {
        CIC_DEBUG(F("initialization failed!"));
        return false;
    }
    CIC_DEBUG(F("initialization done."));
    return true;
}

void DCPSDCard::printDirectory(String path, int numTabs) {
    int attempts = 0;
    while (attempts <= SD_ATTEMPTS) {
        if (takeSDMutex()) {
            FileSD dir = SD.open(path.c_str());
            // Begin at the start of the directory
            dir.rewindDirectory();

            CIC_DEBUG(path);

            while (true) {
                FileSD entry = dir.openNextFile();
                if (!entry) {
                    // no more files
                    //Serial.println("**nomorefiles**");
                    break;
                }
                for (uint8_t i = 0; i < numTabs; i++) {
                    CIC_DEBUG_(F('\t')); // we'll have a nice indentation
                }
                // Print the 8.3 name
                CIC_DEBUG_(entry.name());
                // Recurse for directories, otherwise print the file size
                if (entry.isDirectory()) {
                    CIC_DEBUG("/");
                    printDirectory(entry.name(), numTabs + 1);
                } else {
                    // files have sizes, directories do not
                    CIC_DEBUG_("\t\t");
                    CIC_DEBUG(entry.size());
                }
                entry.close();
            }
            giveSDMutex();
            return;
        } else {
            CIC_DEBUG("Waiting to print directory...");
        }
        attempts = attempts + 1;
        delay(SD_ATTEMPTS_DELAY);
    }
}

String DCPSDCard::getFirstFile(String path) {
    int attempts = 0;
    while (attempts <= SD_ATTEMPTS) {
        if (takeSDMutex()) {
            FileSD dir = SD.open(path.c_str());
            // Begin at the start of the directory
            dir.rewindDirectory();

            FileSD entry = dir.openNextFile();
            if (!entry) {
                entry.close();
                giveSDMutex();
                return "";
            }
            String filename = entry.name();
            entry.close();
            giveSDMutex();
            return filename;
        } else {
            CIC_DEBUG("Waiting to getting first file...");
        }
        attempts = attempts + 1;
        vTaskDelay(SD_ATTEMPTS_DELAY);
    }
}

boolean DCPSDCard::writeFile(String filename, String content) {
    int attempts = 0;
    while (attempts <= SD_ATTEMPTS) {
        if (takeSDMutex()) {
            // open the file. note that only one file can be open at a time,
            // so you have to close this one before opening another.
            FileSD myFile = SD.open(filename.c_str(), FILE_WRITE);

            // if the file opened okay, write to it:
            if (myFile) {
                myFile.println(content);
                // close the file:
                myFile.close();
                CIC_DEBUG_(sdRTC.now());
                CIC_DEBUG_(F(" - Write file: "));
                CIC_DEBUG(filename);
                CIC_DEBUG_("content: ");
                CIC_DEBUG(content);
                giveSDMutex();
                return true;
            } else {
                // if the file didn't open, print an error:
                CIC_DEBUG_(F("Error opening "));
                CIC_DEBUG(filename);
            }
            giveSDMutex();
        } else {
            CIC_DEBUG("Waiting to write File...");
        }
        attempts = attempts + 1;
        delay(SD_ATTEMPTS_DELAY);
    }
    return false;
}

boolean DCPSDCard::readPublishFile(String filename, boolean(*callback)(String msg, PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC), PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC) {
    int attempts = 0;
    while (attempts <= SD_ATTEMPTS) {
        if (takeSDMutex()) {
            // re-open the file for reading:
            FileSD myFile = SD.open(filename.c_str());
            CIC_DEBUG(filename.c_str());
            if (myFile) {
                // read from the file until there's nothing else in it:
                String msg = "";
                while (myFile.available()) {
                    char cMsg = (char) myFile.read();
                    if (cMsg == '\n') {
                        CIC_DEBUG(msg);
                        (*callback)(msg, _clientPub, tknDCP, pwdDCP, TOPIC);
                        msg = "";
                    } else {
                        msg = msg + cMsg;
                    }
                }
                // close the file:
                myFile.close();
                giveSDMutex();
                return true;
            } else {
                // if the file didn't open, print an error:
                CIC_DEBUG_(F("Error opening "));
                CIC_DEBUG(filename);
                giveSDMutex();
                return false;
            }

        } else {
            CIC_DEBUG("Waiting to read File...");
        }
        attempts = attempts + 1;
        vTaskDelay(SD_ATTEMPTS_DELAY);
    }
}

String DCPSDCard::readFile(String filename) {
    int attempts = 0;
    while (attempts <= SD_ATTEMPTS) {
        if (takeSDMutex()) {
            // re-open the file for reading:
            FileSD myFile = SD.open(filename.c_str());
            String result = "";
            if (myFile) {
                // read from the file until there's nothing else in it:
                while (myFile.available()) {
                    result = result + (char) myFile.read();

                }
                // close the file:
                myFile.close();

            } else {
                // if the file didn't open, print an error:
                CIC_DEBUG_(F("Error opening "));
                CIC_DEBUG(filename);
            }
            giveSDMutex();
            return result;
        } else {
            CIC_DEBUG("Waiting to read File...");
        }
        attempts = attempts + 1;
        delay(SD_ATTEMPTS_DELAY);
    }
    return "";
}

boolean DCPSDCard::deleteFile(String filename) {
    int attempts = 0;
    while (attempts <= SD_ATTEMPTS) {
        if (takeSDMutex()) {
            char ff[filename.length() + 1];
            filename.toCharArray(ff, filename.length() + 1);
            CIC_DEBUG_(F("Delete file "));
            CIC_DEBUG(filename);
            giveSDMutex();
            return SD.remove(ff);
        } else {
            CIC_DEBUG("Waiting to delete File...");
        }
        attempts = attempts + 1;
        delay(SD_ATTEMPTS_DELAY);
    }
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

boolean DCPSDCard::storeMetadadosStation(String la, String lo, String bucket, String comType, String simICCID, String simOpera, String comLocalIP, String comSQ) {
    CIC_DEBUG_HEADER(F("Store Metadata!"));
    String content = "\"mtd\":{\"LA\":" + la + ",\"LO\":" + lo + ",\"bkt\":" + bucket;
    content = content + ",\"cty:\":\"" + comType + "\"";
    content = content + ",\"icc:\":\"" + simICCID + "\"";
    content = content + ",\"opr:\":\"" + simOpera + "\"";
    content = content + ",\"lip:\":\"" + comLocalIP + "\"";
    content = content + ",\"csq:\":\"" + comSQ + "\"}";
    String filename = sdRTC.now("%Y%m%d") + ".mtd";
    writeFile(filename, content);
    return true;
}