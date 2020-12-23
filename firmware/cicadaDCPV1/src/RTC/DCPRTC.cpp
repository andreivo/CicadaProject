/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */
#include "DCPRTC.h"

DCPRTC::DCPRTC() {
}

/**
 * Setup RTC module
 */
boolean DCPRTC::setupRTCModule(String calTimestamp) {
    CIC_DEBUG_HEADER(F("SETUP RTC MODULE"));
    CIC_DEBUG_(F("Time: "));
    CIC_DEBUG(calTimestamp);

    time_t tt = stringToTime(calTimestamp);
    CIC_DEBUG_("Unix Time: ");
    CIC_DEBUG(int32_t(tt));
    CIC_DEBUG_("Antes do RTC: ");
    CIC_DEBUG(printTime(tt));
    timeval tv; //Cria a estrutura temporaria para funcao abaixo.
    tv.tv_sec = int32_t(tt); //Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo!
    settimeofday(&tv, NULL); //Configura o RTC para manter a data atribuida atualizada.
    CIC_DEBUG_("depois do RTC: ");
    tt = time(NULL); //Obtem o tempo atual em segundos. Utilize isso sempre que precisar obter o tempo atual
    CIC_DEBUG(printTime(tt));

    return true;
}

/**
 * Setup RTC module
 */
boolean DCPRTC::setupRTCModule(int32_t tt) {
    CIC_DEBUG_HEADER(F("SETUP RTC MODULE"));
    CIC_DEBUG_("Unix Time: ");
    CIC_DEBUG(int32_t(tt));
    CIC_DEBUG_("Before RTC: ");
    CIC_DEBUG(printTime(tt));
    timeval tv; //Cria a estrutura temporaria para funcao abaixo.
    tv.tv_sec = int32_t(tt); //Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo!
    settimeofday(&tv, NULL); //Configura o RTC para manter a data atribuida atualizada.
    CIC_DEBUG_("After RTC: ");
    tt = time(NULL); //Obtem o tempo atual em segundos. Utilize isso sempre que precisar obter o tempo atual
    CIC_DEBUG(printTime(tt));

    return true;
}

time_t DCPRTC::stringToTime(String calTimestamp) {
    struct tm tm;
    String year = calTimestamp.substring(0, 4);
    String month = calTimestamp.substring(5, 7);
    if (month.startsWith("0")) {
        month = month.substring(1);
    }
    String day = calTimestamp.substring(8, 10);
    if (day.startsWith("0")) {
        month = day.substring(1);
    }
    tm.tm_year = year.toInt() - 1900;
    tm.tm_mon = month.toInt() - 1;
    tm.tm_mday = day.toInt();
    tm.tm_hour = calTimestamp.substring(11, 13).toInt();
    tm.tm_min = calTimestamp.substring(14, 16).toInt();
    CIC_DEBUG(calTimestamp.substring(14, 16));
    tm.tm_sec = calTimestamp.substring(17, 20).toInt();
    return mktime(&tm);
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

int32_t DCPRTC::nowEpoch() {
    time_t tt = time(NULL); //Obtem o tempo atual em segundos. Utilize isso sempre que precisar obter o tempo atual
    return int32_t(tt);
}