/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#ifndef DCPDht_h
#define DCPDht_h

#include <SimpleDHT.h>
#include "../system/DCPSystem.h"
#include "../RTC/DCPRTC.h"

class DCPDht {
public:
    DCPDht();
    void initDHTSensor(String _codeTemp, String _typeTemp, String _codeHum, String _typeHum, int temp, int hum);
    void readDHT();
private:
    String codeTemp;
    String typeTemp;
    String codeHum;
    String typeHum;
    int TIME_TO_READ_TEMP = (10);
    int TIME_TO_READ_HUM = (10);
    int nextSlotTimeToRead(int TIME_TO_READ);
    int nextSlotTemp;
    boolean timeToReadTemp();
    void nextSlotTimeToReadTemp();
    int nextSlotHum;
    boolean timeToReadHum();
    void nextSlotTimeToReadHum();
};

#endif