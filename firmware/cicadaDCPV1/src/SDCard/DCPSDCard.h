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
#include <PubSubClient.h>
#include "../SIM800/DCPSIM800.h"

#define SD_ATTEMPTS 3
#define SD_ATTEMPTS_DELAY 100

class DCPSDCard {
public:
    DCPSDCard();
    boolean setupSDCardModule();
    void printDirectory(String path, int numTabs);
    boolean writeFile(String filename, String content);
    String readFile(String filename);
    boolean readPublishFile(String filename, boolean(*callback)(String msg, PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC), PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC);
    boolean deleteFile(String filename);
    String prepareData(String sensorCode, String dataType, String collectionDate, String value);
    boolean storeData(String sensor, String measures);
    String getFirstFile(String path);
    boolean storeMetadadosStation(String la, String lo, String bucket, String comType, String simICCID, String simOpera, String comLocalIP, String comSQ);
private:
    boolean takeSDMutex();
    void giveSDMutex();
};

#endif
