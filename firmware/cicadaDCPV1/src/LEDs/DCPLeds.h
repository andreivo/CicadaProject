/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#ifndef DCPLeds_h
#define DCPLeds_h

#include "../system/DCPSystem.h"

#define LEDDELAYTIME 200

class DCPLeds {
public:
    DCPLeds();

    void redTurnOn();
    void redTurnOff();
    void redBlink();
    void redBlink(int times);
    void redBlink(int times, int delaytime);
    void greenTurnOn();
    void greenTurnOff();
    void greenBlink();
    void greenBlink(int times);
    void greenBlink(int times, int delaytime);
    void blueTurnOn();
    void blueTurnOff();
    void blueBlink();
    void blueBlink(int times);
    void blueBlink(int times, int delaytime);
private:
    boolean redOn = false;
    boolean greenOn = false;
    boolean blueOn = false;

};

#endif
