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

class DCPDht {
public:
    DCPDht();

    void initDHTSensor();
};

#endif