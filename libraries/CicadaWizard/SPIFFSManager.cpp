/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#include <FS.h> // FS must be the first
#include "SPIFFSManager.h"
#include <SPIFFS.h>

SPIFFSManager::SPIFFSManager() {
}

/**
 * Create a file in File System
 *
 * @param directory Directory path
 * @param filename File Name
 * @return true if file was created successfully, otherwise false.
 */
boolean SPIFFSManager::FSCreateFile(String directory, String filename) {

    CIC_DEBUG_(F("Creating file: \""));
    CIC_DEBUG_(directory);
    CIC_DEBUG_(F("/"));
    CIC_DEBUG_(filename);
    CIC_DEBUG(F("\""));

    // Mounts SPIFFS file system
    if (!SPIFFS.begin()) {
        CIC_DEBUG(F("FATAL: Error mounting SPIFFS file system."));
    }

    boolean success = true;

    // Mount filepath
    String filepath = directory;
    filepath += "/";
    filepath += filename;

    // Open/Create file
    File f = SPIFFS.open(filepath, "w");
    if (f) {
        CIC_DEBUG(F("SUCCESS: File created."));
        f.close();
    } else {
        success = false;
        CIC_DEBUG(F("ERROR: Fail creating file."));
    }

    return success;
}

/**
 * Create a file in File System
 *
 * @param directory Directory path
 * @param filename File Name
 * @return true if file was created successfully, otherwise false.
 */
boolean SPIFFSManager::FSCreateFile(String directory, int filename) {

    CIC_DEBUG_(F("Creating file: \""));
    CIC_DEBUG_(directory);
    CIC_DEBUG_(F("/"));
    CIC_DEBUG_(filename);
    CIC_DEBUG(F("\""));

    // Mounts SPIFFS file system
    if (!SPIFFS.begin()) {
        CIC_DEBUG(F("FATAL: Error mounting SPIFFS file system."));
    }

    boolean success = true;

    // Mount filepath
    String filepath = directory;
    filepath += "/";
    filepath += filename;

    // Open/Create file
    File f = SPIFFS.open(filepath, "w");
    if (f) {
        CIC_DEBUG(F("SUCCESS: File created."));
        f.close();
    } else {
        success = false;
        CIC_DEBUG(F("ERROR: Fail creating file."));
    }

    return success;
}

/**
 * Create a file in File System
 *
 * @param directory Directory path
 * @param filename File Name
 * @return true if file was created successfully, otherwise false.
 */
boolean SPIFFSManager::FSCreateFile(String directory, unsigned long filename) {

    CIC_DEBUG_(F("Creating file: \""));
    CIC_DEBUG_(directory);
    CIC_DEBUG_(F("/"));
    CIC_DEBUG_(filename);
    CIC_DEBUG(F("\""));

    // Mounts SPIFFS file system
    if (!SPIFFS.begin()) {
        CIC_DEBUG(F("FATAL: Error mounting SPIFFS file system."));
    }

    boolean success = true;

    // Mount filepath
    String filepath = directory;
    filepath += "/";
    filepath += filename;

    // Open/Create file
    File f = SPIFFS.open(filepath, "w");
    if (f) {
        CIC_DEBUG(F("SUCCESS: File created."));
        f.close();
    } else {
        success = false;
        CIC_DEBUG(F("ERROR: Fail creating file."));
    }

    return success;
}

/**
 * Write content to a file
 *
 * @param directory Directory path
 * @param filename File Name
 * @param content File Content
 * @return true if content was created successfully, otherwise false.
 */
boolean SPIFFSManager::FSWriteToFile(String directory, String filename, String content) {

    CIC_DEBUG_(F("Creating file: \""));
    CIC_DEBUG_(directory);
    CIC_DEBUG_(F("/"));
    CIC_DEBUG_(filename);
    CIC_DEBUG(F("\""));

    CIC_DEBUG_(F("Content to write: \""));
    CIC_DEBUG_(content);
    CIC_DEBUG(F("\""));

    // Mounts SPIFFS file system
    if (!SPIFFS.begin()) {
        CIC_DEBUG(F("FATAL: Error mounting SPIFFS file system."));
    }

    boolean success = true;

    // Mount filepath
    String filepath = directory;
    filepath += "/";
    filepath += filename;

    // Open/Create file
    File f = SPIFFS.open(filepath, "w");
    if (f) {
        CIC_DEBUG(F("SUCCESS: File created."));

        // Check if the content was saved successfully
        if (f.print(content)) {
            CIC_DEBUG(F("SUCCESS: Content saved to file."));
        } else {
            success = false;
        }

        f.close();
    } else {
        success = false;
        CIC_DEBUG(F("ERROR: Fail creating file."));
    }

    return success;
}

String SPIFFSManager::FSReadToFile(String directory, String filename) {
    // Mounts SPIFFS file system
    if (!SPIFFS.begin()) {
        CIC_DEBUG(F("FATAL: Error mounting SPIFFS file system."));
    }

    // Mount filepath
    String filepath = directory;
    filepath += "/";
    filepath += filename;

    // Open the longitude directory
    File file = SPIFFS.open(filepath, "r");

    if (file) {

        CIC_DEBUG_(F("File located: "));
        CIC_DEBUG(file.name());

        String line = file.readStringUntil('\n');
        CIC_DEBUG_("Content: ");
        CIC_DEBUG(line);
        file.close();

        return line;
    } else {
        CIC_DEBUG(F("EMPTY DIRECTORY."));
    }
    return "";
}

/**
 * Deletes all files from the given directory.
 *
 * @param directory Directory absolute path
 *
 * @return true if all files was deleted successfully, otherwise false.
 */
boolean SPIFFSManager::FSDeleteFiles(String directory) {

    CIC_DEBUG_(F("Deleting files from directory: \""));
    CIC_DEBUG_(directory);
    CIC_DEBUG(F("\""));

    // Mounts SPIFFS file system
    if (!SPIFFS.begin()) {
        CIC_DEBUG(F("FATAL: Error mounting SPIFFS file system."));
    }

    boolean success = true;

    // Open directory
    File dir = SPIFFS.open(directory);
    File file = dir.openNextFile();

    // Remove all files
    while (file) {

        CIC_DEBUG_(F("Removing file : "));
        CIC_DEBUG(file.name());

        if (!SPIFFS.remove(file.name())) {
            CIC_DEBUG(F("ERROR during file deletion!"));
            success = false;
        }
        file = dir.openNextFile();
    }

    return success;
}

/**
 * Delete file.
 *
 * @param directory Directory absolute path
 * @param filename Filename
 *
 * @return true if file was deleted successfully, otherwise false.
 */
boolean SPIFFSManager::FSDeleteFile(String directory, int filename) {

    CIC_DEBUG_(F("Deleting file: \""));
    CIC_DEBUG_(directory);
    CIC_DEBUG_(F("/"));
    CIC_DEBUG_(filename);
    CIC_DEBUG(F("\""));

    // Mounts SPIFFS file system
    if (!SPIFFS.begin()) {
        CIC_DEBUG(F("FATAL: Error mounting SPIFFS file system."));
    }

    boolean success = true;

    // Mount filepath
    String filepath = directory;
    filepath += "/";
    filepath += filename;

    CIC_DEBUG_(F("Removing file : "));
    CIC_DEBUG(filepath);

    // Remove file
    if (!SPIFFS.remove(filepath)) {
        success = false;
        CIC_DEBUG(F("ERROR: Fail deleting file."));
    }

    return success;
}

/**
 * Read int value from File System
 *
 * @param filepath Directory filepath
 * @return The number readed, or 0 if it fails
 */
unsigned int SPIFFSManager::FSReadInt(String filepath) {

    CIC_DEBUG_(F("Reading unsigned int from filepath: \""));
    CIC_DEBUG_(filepath);
    CIC_DEBUG(F("\""));

    // Mounts SPIFFS file system
    if (!SPIFFS.begin()) {
        CIC_DEBUG(F("FATAL: Error mounting SPIFFS file system."));
    }


    // Open the longitude directory
    File dir = SPIFFS.open(filepath);
    File file = dir.openNextFile();

    if (file) {

        CIC_DEBUG_(F("File located: \""));
        CIC_DEBUG_(file.name());
        CIC_DEBUG(F("\""));

        String filename = file.name();

        return filename.substring(filepath.length() + 1).toInt();
    } else {
        CIC_DEBUG(F("EMPTY DIRECTORY."));
    }

    return 0;
}

/**
 * Read float value from File System
 *
 * @param filepath Directory filepath
 * @return The number readed, or 0 if it fails
 */
float SPIFFSManager::FSReadFloat(String filepath) {

    CIC_DEBUG_(F("Reading float from filepath: \""));
    CIC_DEBUG_(filepath);
    CIC_DEBUG(F("\""));

    // Mounts SPIFFS file system
    if (!SPIFFS.begin()) {
        CIC_DEBUG(F("FATAL: Error mounting SPIFFS file system."));
    }


    // Open the longitude directory
    File dir = SPIFFS.open(filepath);
    File file = dir.openNextFile();

    if (file) {

        CIC_DEBUG_(F("File located: \""));
        CIC_DEBUG_(file.name());
        CIC_DEBUG(F("\""));

        String filename = file.name();

        return filename.substring(filepath.length() + 1).toFloat();
    } else {
        CIC_DEBUG(F("EMPTY DIRECTORY."));
    }



    return 0;
}

/**
 * Read unsigned long value from File System
 *
 * @param filepath Directory filepath
 * @return The number readed, or 0 if it fails
 */
unsigned long SPIFFSManager::FSReadULong(String filepath) {

    CIC_DEBUG_(F("Reading unsigned long from filepath: \""));
    CIC_DEBUG_(filepath);
    CIC_DEBUG(F("\""));

    // Mounts SPIFFS file system
    if (!SPIFFS.begin()) {
        CIC_DEBUG(F("FATAL: Error mounting SPIFFS file system."));
    }

    // Open the longitude directory
    File dir = SPIFFS.open(filepath);
    File file = dir.openNextFile();

    if (file) {

        CIC_DEBUG_(F("File located: \""));
        CIC_DEBUG_(file.name());
        CIC_DEBUG(F("\""));

        String filename = file.name();

        return strtoul(filename.substring(filepath.length() + 1).c_str(), NULL, 10);
    } else {
        CIC_DEBUG(F("EMPTY DIRECTORY."));
    }

    return 0;
}

/**
 * Read String value from File System
 *
 * @param filepath Directory filepath
 * @return The String readed, or "" (blank) if it fails
 */
String SPIFFSManager::FSReadString(String filepath) {

    CIC_DEBUG_(F("Reading String from filepath: \""));
    CIC_DEBUG_(filepath);
    CIC_DEBUG(F("\""));

    // Mounts SPIFFS file system
    if (!SPIFFS.begin()) {
        CIC_DEBUG(F("FATAL: Error mounting SPIFFS file system."));
    }

    // Open the longitude directory
    File dir = SPIFFS.open(filepath);
    File file = dir.openNextFile();

    if (file) {

        CIC_DEBUG_(F("File located: \""));
        CIC_DEBUG_(file.name());
        CIC_DEBUG(F("\""));

        String filename = file.name();

        return filename.substring(filepath.length() + 1);
    } else {
        CIC_DEBUG(F("EMPTY DIRECTORY."));
    }


    return "";
}

/**
 * Print the system's list of files and directories
 */
void SPIFFSManager::FSPrintFileList() {

    CIC_DEBUG(F("\n\nFILE LIST"));
    CIC_DEBUG(F("==========================================="));

    // Mounts SPIFFS file system
    if (!SPIFFS.begin()) {
        CIC_DEBUG(F("FATAL: Error mounting SPIFFS file system."));
    }


    // Open directory
    File dir = SPIFFS.open("/");
    File file = dir.openNextFile();

    int fc = 0;
    while (file) {
        ++fc;
        CIC_DEBUG_(fc);
        CIC_DEBUG_(" - ");
        CIC_DEBUG(file.name());
        file = dir.openNextFile();
    }
}

/**
 * Format file system
 */
void SPIFFSManager::FSFormat() {

    CIC_DEBUG(F("\n\nFORMATTING FILE SYSTEM"));
    CIC_DEBUG(F("==========================================="));

    // Mounts SPIFFS file system
    if (!SPIFFS.begin()) {
        CIC_DEBUG(F("FATAL: Error mounting SPIFFS file system."));
    }

    // Format
    CIC_DEBUG(F("Formatting file system... (relax, this could take a while)"));
    CIC_DEBUG(F("Ok, \"a while\" is  -- uh -- too much generic, it takes 1m 20sec on average. Better, no? ;)"));

    int start = millis();
    if (SPIFFS.format()) {
        CIC_DEBUG(F("SUCCESS: File system formatted successfully."));
        CIC_DEBUG_(F("Total time: "));
        CIC_DEBUG_((millis() - start));
        CIC_DEBUG(F(" ms"));
    } else {
        CIC_DEBUG(F("ERROR: Fail formatting file system."));
    }
}

/**
 * Convert bytes in to KB and MB.
 *
 * @param bytes float number to be converted
 * @param prefix the unit to be converted.
 * @return the number formatted acording to the specified unit
 */
float SPIFFSManager::bytesConverter(float bytes, char prefix) {

    // Kilobyte (KB)
    if (prefix == 'K') {
        return bytes / 1000;

        // Megabyte (MB)
    } else if (prefix == 'M') {
        return bytes / 1000000;
    }
}

/**
 * Get Settings
 */
String SPIFFSManager::getSettings(String dsc, String DIR, boolean inContent) {

    CIC_DEBUG_(F("\n\nGET "));
    CIC_DEBUG_(dsc);
    CIC_DEBUG(F("==========================================="));
    String settings = FSReadString(DIR);
    if (inContent) {
        settings = FSReadToFile(DIR, settings);
    }

    CIC_DEBUG_(F("Getting "));
    CIC_DEBUG_(dsc);
    CIC_DEBUG("....");

    if (settings != "") {
        CIC_DEBUG_(dsc);
        CIC_DEBUG_(": ");
        CIC_DEBUG(settings);
    } else {

        CIC_DEBUG_(F("ERROR during reading: "));
        CIC_DEBUG(dsc);
    }
    return settings;
}

/**
 * Remove settings
 */
void SPIFFSManager::deleteSettings(String dsc, String DIR) {

    CIC_DEBUG_(F("\n\nDELETING: "));
    CIC_DEBUG_(dsc);
    CIC_DEBUG(F("==========================================="));

    FSDeleteFiles(DIR);
}

/** Save next settings to File System */
void SPIFFSManager::saveSettings(String dsc, String DIR, String value) {

    deleteSettings(dsc, DIR);

    CIC_DEBUG_(F("\n\nSAVING: "));
    CIC_DEBUG_(dsc);
    CIC_DEBUG(F("==========================================="));

    CIC_DEBUG(F("Saving..."));

    if (FSCreateFile(DIR, value)) {
        CIC_DEBUG(F("Saved success!"));
    }
}

/** Save next settings to File System */
void SPIFFSManager::saveSettings(String dsc, String DIR, String value, String content) {

    deleteSettings(dsc, DIR);

    CIC_DEBUG_(F("\n\nSAVING: "));
    CIC_DEBUG_(dsc);
    CIC_DEBUG(F("==========================================="));

    CIC_DEBUG(F("Saving..."));

    if (FSWriteToFile(DIR, value, content)) {
        CIC_DEBUG(F("Saved success!"));
    }
}
