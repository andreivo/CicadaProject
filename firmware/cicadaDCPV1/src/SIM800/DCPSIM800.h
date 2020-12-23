/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#ifndef DCPSIM800_h
#define DCPSIM800_h

#define TINY_GSM_MODEM_SIM800 //Tipo de modem que estamos usando
#include "../system/DCPSystem.h"
#include <TinyGsmClient.h>

class DCPSIM800 {
public:
    DCPSIM800();
    void turnOn();
    void turnOff();
    boolean setupSIM800Module();
    String getNetworkDate();
private:

};


#endif
