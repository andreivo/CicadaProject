/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

#ifndef DCPDht_h
#define DCPDht_h

#include <SimpleDHT.h>
#include "../../System/DCPSystem.h"
#include "../../System/RTC/DCPRTC.h"

class DCPDht {
public:
    DCPDht();
    void initDHTSensor(String _codeTemp, String _typeTemp, String _codeHum, String _typeHum, int timeSlotDHT);
    void readDHT();
    String printDHT();
    void updateNextSlot();
    void printNextSlot();
private:
    String codeTemp;
    String typeTemp;
    String codeHum;
    String typeHum;
    int TIME_TO_READ_DHT = (10);
    void nextSlotTimeToRead();
    int nextSlotDHT;
    boolean timeToReadDHT();

};

#endif