/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#ifndef DCPRainGauge_h
#define DCPRainGauge_h

#include "../system/DCPSystem.h"

#define RG_DEBOUNCETIME 200

void IRAM_ATTR handleBucketInterrupt();

class DCPRainGauge {
public:
    DCPRainGauge();
    void setupRGSensor();
    void initRGSensor(String _codeRG, String _typeRG, int timeSlotRG);
    void readRG();
private:
    String codeRG;
    String typeRG;
    int TIME_TO_READ_RG = (10);
    void nextSlotTimeToRead();
    int nextSlotRG;
    boolean timeToReadRG();
};

#endif