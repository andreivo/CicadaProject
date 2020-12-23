/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#ifndef DCPSDCard_h
#define DCPSDCard_h

#include "../system/DCPSystem.h"
#include <SPI.h>
#include <mySD.h>

class DCPSDCard {
public:
    DCPSDCard();
    boolean setupSDCardModule();
    void printDirectory(String path, int numTabs);
    boolean writeFile(String filename, String content);
    String readFile(String filename);
    boolean deleteFile(String filename);
    String prepareData(String sensorCode, String dataType, String collectionDate, String value);
    boolean storeData(String sensor, String measures);
};

#endif
