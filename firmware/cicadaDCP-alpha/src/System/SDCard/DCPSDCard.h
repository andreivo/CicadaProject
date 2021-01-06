/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

#ifndef DCPSDCard_h
#define DCPSDCard_h

#include "../DCPSystem.h"
#include <SPI.h>
#include <PubSubClient.h>
#include "../SIM800/DCPSIM800.h"
#include "SdFat.h"
#include "../SDCard/DCPSDFormatter.h"

#if SPI_DRIVER_SELECT == 2  // Must be set in SdFat/SdFatConfig.h

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 1

#define SD_ATTEMPTS 3
#define SD_ATTEMPTS_DELAY 100

class DCPSDCard {
public:
    DCPSDCard();
    boolean setupSDCardModule();
    void printDirectory(String path);
    boolean writeFile(String filename, String content);
    String readFile(String filename);
    boolean mqttPublishFiles(boolean(*callback)(String msg, PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC), PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC);
    void printContentFile(String filename);
    boolean deleteFile(String filename);
    String prepareData(String sensorCode, String dataType, String collectionDate, String value);
    boolean storeData(String sensor, String measures);
    boolean storeMetadadosStation(String la, String lo, String bucket, String comType, String simICCID, String simOpera, String comLocalIP, String comSQ);
    void deleteOldFiles();
    String getCardType();
    String cidDmp();
    String csdDmp();
    String mbrDmp();
    String dmpVol();
    void formatSD(boolean ask = true);
private:
    boolean readPublishFile(String filename, boolean(*callback)(String msg, PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC), PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC);
    boolean takeSDMutex(String function);
    void giveSDMutex(String function);
    String padL(int len, String inS);

#if SD_FAT_TYPE == 0
    SdFat sd;
    File file;
#elif SD_FAT_TYPE == 1
    SdFat32 sd;
    File32 file;
#elif SD_FAT_TYPE == 2
    SdExFat sd;
    ExFile file;
#elif SD_FAT_TYPE == 3
    SdFs sd;
    FsFile file;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE
};

#else  // SPI_DRIVER_SELECT
#error SPI_DRIVER_SELECT must be two in SdFat/SdFatConfig.h
#endif  //SPI_DRIVER_SELECT

#endif
