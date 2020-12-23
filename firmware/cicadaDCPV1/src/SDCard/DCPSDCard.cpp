/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */
#include "DCPSDCard.h"

DCPRTC sdRTC;

DCPSDCard::DCPSDCard() {
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
}

boolean DCPSDCard::writeFile(String filename, String content) {
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
        return true;
    } else {
        // if the file didn't open, print an error:
        CIC_DEBUG_(F("Error opening "));
        CIC_DEBUG(filename);
    }
    return false;
}

String DCPSDCard::readFile(String filename) {
    // re-open the file for reading:
    FileSD myFile = myFile = SD.open(filename.c_str());
    String result = "";
    if (myFile) {
        // read from the file until there's nothing else in it:
        while (myFile.available()) {
            result = myFile.read();
        }
        // close the file:
        myFile.close();

    } else {
        // if the file didn't open, print an error:
        CIC_DEBUG_(F("Error opening "));
        CIC_DEBUG(filename);
    }
    return result;
}

boolean DCPSDCard::deleteFile(String filename) {
    char ff[filename.length() + 1];
    filename.toCharArray(ff, filename.length() + 1);
    CIC_DEBUG_(F("Delete file "));
    CIC_DEBUG(filename);
    return SD.remove(ff);
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