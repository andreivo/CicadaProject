/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#ifndef SPIFFSManager_h
#define SPIFFSManager_h

#define CIC_DEBUG_ENABLED true

#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_(text) { Serial.print( (text) ); }
#else
#define CIC_DEBUG_(text) {}
#endif

#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_HEADER(text) {  Serial.println(F("\n")); Serial.println((text)); Serial.println(F("===========================================")); }
#else
#define CIC_DEBUG_HEADER(text) {}
#endif

#if CIC_DEBUG_ENABLED
#define CIC_DEBUG(text) { Serial.println( (text) ); }
#else
#define CIC_DEBUG(text) {}
#endif

// Serial debug
#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_SETUP(baudrate) { Serial.begin( (baudrate) );  delay(200);}
#else
#define CIC_DEBUG_SETUP(baudrate) {}
#endif


class SPIFFSManager {
public:
    SPIFFSManager();


    /**
     * Create a file in File System
     *
     * @param directory Directory path
     * @param filename File Name
     * @return true if file was created successfully, otherwise false.
     */
    boolean FSCreateFile(String directory, String filename);

    /**
     * Create a file in File System
     *
     * @param directory Directory path
     * @param filename File Name
     * @return true if file was created successfully, otherwise false.
     */
    boolean FSCreateFile(String directory, int filename);

    /**
     * Create a file in File System
     *
     * @param directory Directory path
     * @param filename File Name
     * @return true if file was created successfully, otherwise false.
     */
    boolean FSCreateFile(String directory, unsigned long filename);

    /**
     * Write content to a file
     *
     * @param directory Directory path
     * @param filename File Name
     * @param content File Content
     * @return true if content was created successfully, otherwise false.
     */
    boolean FSWriteToFile(String directory, String filename, String content);

    String FSReadToFile(String directory, String filename);

    /**
     * Deletes all files from the given directory.
     *
     * @param directory Directory absolute path
     *
     * @return true if all files was deleted successfully, otherwise false.
     */
    boolean FSDeleteFiles(String directory);

    /**
     * Delete file.
     *
     * @param directory Directory absolute path
     * @param filename Filename
     *
     * @return true if file was deleted successfully, otherwise false.
     */
    boolean FSDeleteFile(String directory, int filename);

    /**
     * Read int value from File System
     *
     * @param filepath Directory filepath
     * @return The number readed, or 0 if it fails
     */
    unsigned int FSReadInt(String filepath);

    /**
     * Read float value from File System
     *
     * @param filepath Directory filepath
     * @return The number readed, or 0 if it fails
     */
    float FSReadFloat(String filepath);

    /**
     * Read unsigned long value from File System
     *
     * @param filepath Directory filepath
     * @return The number readed, or 0 if it fails
     */
    unsigned long FSReadULong(String filepath);

    /**
     * Read String value from File System
     *
     * @param filepath Directory filepath
     * @return The String readed, or "" (blank) if it fails
     */
    String FSReadString(String filepath);

    /**
     * Print the system's file list
     */
    void FSPrintFileList();

    /**
     * Format file system
     */
    void FSFormat();

    /**
     * Convert bytes in to KB and MB.
     *
     * @param bytes float number to be converted
     * @param prefix the unit to be converted.
     * @return the number formatted acording to the specified unit
     */
    float bytesConverter(float bytes, char prefix);
};

#endif
