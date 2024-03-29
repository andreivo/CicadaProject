/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

#ifndef DCPRainGauge_h
#define DCPRainGauge_h

#include "../../System/DCPSystem.h"

#define RG_DEBOUNCETIME 200

void IRAM_ATTR handleBucketInterrupt();

class DCPRainGauge {
public:
    DCPRainGauge();
    void setupRGSensor();
    void initRGSensor(String _codeRG, String _typeRG, int timeSlotRG, float _bucketVol, float _collectionArea);
    void readRG();
    String printTipBucket();
    void updateNextSlot();
    void printNextSlot();
private:
    String codeRG;
    String typeRG;
    float BUCKET_VOLUME;
    float CONTRIBUTION_AREA;
    int TIME_TO_READ_RG = (10);
    void nextSlotTimeToRead();
    int nextSlotRG;
    boolean timeToReadRG();
};

#endif