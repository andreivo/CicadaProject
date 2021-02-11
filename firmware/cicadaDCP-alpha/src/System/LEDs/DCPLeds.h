/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

#ifndef DCPLeds_h
#define DCPLeds_h

#include "../DCPSystem.h"

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
    boolean timeToBlinkStatus();
    void blinkStatusOk();
    void blinkStatusError();
private:
    boolean redOn = false;
    boolean greenOn = false;
    boolean blueOn = false;
    int32_t lastEpoch;

};

#endif
