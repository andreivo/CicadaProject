/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#ifndef DCPRTC_h
#define DCPRTC_h

#include "../system/DCPSystem.h"
#include <time.h>
#include <sys/time.h>

class DCPRTC {
public:
    DCPRTC();
    boolean setupRTCModule(String calTimestamp);
    boolean setupRTCModule(time_t tt);
    String now(String format = "%Y-%m-%dT%H:%M:%SZ");
    String printTime(time_t tt, String format = "%Y-%m-%dT%H:%M:%SZ");
    time_t nowEpoch();
private:
    time_t stringToTime(String calTimestamp);

};

#endif