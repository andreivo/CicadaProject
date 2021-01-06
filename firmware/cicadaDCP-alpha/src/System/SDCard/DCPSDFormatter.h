/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

#ifndef DCPSDFormatter_h
#define DCPSDFormatter_h

#include "../DCPSystem.h"
#include "SdFat.h"
#include "sdios.h"

class DCPSDFormatter {
public:
    DCPSDFormatter();
    static void formatSD(boolean ask = true);
private:
    static void clearSerialInput();


};


#endif
