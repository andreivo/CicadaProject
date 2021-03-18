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

#define SD_ATTEMPTS 3
#define SD_ATTEMPTS_DELAY 500

#define SD_WRITE_DELAY 10

/** year part of FAT directory date field
 * \param[in] fatDate Date in packed dir format.
 *
 * \return Extracted year [1980,2107]
 */
static inline uint16_t SDFAT_YEAR(uint16_t fatDate) {
    return 1980 + (fatDate >> 9);
}

/** month part of FAT directory date field
 * \param[in] fatDate Date in packed dir format.
 *
 * \return Extracted month [1,12]
 */
static inline uint8_t SDFAT_MONTH(uint16_t fatDate) {
    return (fatDate >> 5) & 0XF;
}

/** day part of FAT directory date field
 * \param[in] fatDate Date in packed dir format.
 *
 * \return Extracted day [1,31]
 */
static inline uint8_t SDFAT_DAY(uint16_t fatDate) {
    return fatDate & 0X1F;
}

/** hour part of FAT directory time field
 * \param[in] fatTime Time in packed dir format.
 *
 * \return Extracted hour [0,23]
 */
static inline uint8_t SDFAT_HOUR(uint16_t fatTime) {
    return fatTime >> 11;
}

/** minute part of FAT directory time field
 * \param[in] fatTime Time in packed dir format.
 *
 * \return Extracted minute [0,59]
 */
static inline uint8_t SDFAT_MINUTE(uint16_t fatTime) {
    return (fatTime >> 5) & 0X3F;
}

/** second part of FAT directory time field
 * Note second/2 is stored in packed time.
 *
 * \param[in] fatTime Time in packed dir format.
 *
 * \return Extracted second [0,58]
 */
static inline uint8_t SDFAT_SECOND(uint16_t fatTime) {
    return 2 * (fatTime & 0X1F);
}

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
    String prepareData(String sensorCode, String dataType, String collectionDate, String value, String context = "");
    boolean storeData(String sensor, String measures);
    String prepareDataMetadata(String dataType, String collectionDate, String value, String context = "");
    boolean storeMetadadosStation(String oname, String oemail, String ophone, String la, String lo, String comType, String simICCID, String simOpera, String comLocalIP, String comSQ, String firmware, String dateFirmware);
    void deleteOldFiles(String path = "/");
    void cleanOlderFiles();
    boolean fileExists(String file);
    boolean deleteUpdate();
    String getCardType();
    String cidDmp();
    String csdDmp();
    String mbrDmp();
    String dmpVol();
    void formatSD(boolean ask = true);
    boolean writeLog(String log, boolean ln = true);
    boolean writeBinFile(String filename, uint8_t buff[128], int len);
    boolean takeSDMutex(String function);
    void giveSDMutex(String function);
private:
    int ctrlLog = 1;
    boolean readPublishFile(String filename, boolean(*callback)(String msg, PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC), PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC);
    String padL(int len, String inS);
    String padR(int len, String inS);
    boolean printSDError(boolean restart = false);
};

#else  // SPI_DRIVER_SELECT
#error SPI_DRIVER_SELECT must be two in SdFat/SdFatConfig.h
#endif  //SPI_DRIVER_SELECT

#endif
