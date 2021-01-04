/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */
#include "DCPRTC.h"

DCPRTC::DCPRTC() {
}

/**
 * Setup RTC module
 */
void DCPRTC::setupRTCModule(String calTimestamp) {
    CIC_DEBUG_HEADER(F("SETUP RTC MODULE"));
    CIC_DEBUG_(F("Time: "));
    CIC_DEBUG(calTimestamp);

    time_t tt = stringToTime(calTimestamp);
    setupRTCModule(tt);

    CIC_DEBUG_("Unix Time: ");
    CIC_DEBUG(tt);
    CIC_DEBUG_("Before RTC: ");
    CIC_DEBUG(printTime(tt));

    CIC_DEBUG_("After  RTC: ");
    tt = time(NULL); //Obtem o tempo atual em segundos. Utilize isso sempre que precisar obter o tempo atual
    CIC_DEBUG(printTime(tt));
}

/**
 * Setup RTC module
 */
void DCPRTC::setupRTCModule(time_t tt) {
    timeval epoch = {tt, 0};
    const timeval *tv = &epoch;
    timezone utc = {0, 0};
    const timezone *tz = &utc;
    settimeofday(tv, tz);
}

time_t DCPRTC::stringToTime(String calTimestamp) {
    struct tm tm;

    String datat = getPartOfSplit(calTimestamp, 'T', 0);
    String yeart = getPartOfSplit(datat, '-', 0);
    String montht = getPartOfSplit(datat, '-', 1);
    String dayt = getPartOfSplit(datat, '-', 2);

    String timet = getPartOfSplit(calTimestamp, 'T', 1);
    int indexof = timet.indexOf('Z');
    timet = timet.substring(0, indexof);

    String hourt = getPartOfSplit(timet, ':', 0);
    String mint = getPartOfSplit(timet, ':', 1);
    String sect = getPartOfSplit(timet, ':', 2);

    tm.tm_year = yeart.toInt() - 1900;
    tm.tm_mon = montht.toInt() - 1;
    tm.tm_mday = dayt.toInt();
    tm.tm_hour = hourt.toInt();
    tm.tm_min = mint.toInt();
    tm.tm_sec = sect.toInt();
    return mktime(&tm);
}

String DCPRTC::getPartOfSplit(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String DCPRTC::printTime(time_t tt, String format) {
    struct tm date = *gmtime(&tt); //Converte o tempo atual e atribui na estrutura
    char data_formatada[64];
    strftime(data_formatada, 64, format.c_str(), &date); //Cria uma String formatada da estrutura "data"
    return data_formatada;
}

String DCPRTC::now(String format) {
    time_t tt = time(NULL); //Obtem o tempo atual em segundos. Utilize isso sempre que precisar obter o tempo atual
    return printTime(tt, format);
}

time_t DCPRTC::nowEpoch() {
    time_t tt = time(NULL); //Obtem o tempo atual em segundos. Utilize isso sempre que precisar obter o tempo atual
    return tt;
}

struct tm *DCPRTC::convEpoch(time_t in_time) {
    struct tm *convTime = gmtime(&in_time);
    return convTime;
}

boolean DCPRTC::checkFormat(String calTimestamp) {
    time_t tt = stringToTime(calTimestamp);
    return printTime(tt) == calTimestamp;
}