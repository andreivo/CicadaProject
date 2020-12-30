/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

#ifndef DCPRTC_h
#define DCPRTC_h

#include "../DCPSystem.h"
#include <time.h>
#include <sys/time.h>

class DCPRTC {
public:
    DCPRTC();
    void setupRTCModule(String calTimestamp);
    String now(String format = "%Y-%m-%dT%H:%M:%SZ");
    String printTime(time_t tt, String format = "%Y-%m-%dT%H:%M:%SZ");
    time_t nowEpoch();
private:
    void setupRTCModule(time_t tt);
    time_t stringToTime(String calTimestamp);
    String getPartOfSplit(String data, char separator, int index);

};

#endif