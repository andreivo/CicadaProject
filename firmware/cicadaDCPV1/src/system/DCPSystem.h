/**
 * CICADA DCP Firmware for the ESP32
 *
 *       FILE: DCPSystem.h
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://www.pluvion.com.br
 */

#define DCPSystem_h

#if PLV_DEBUG_ENABLED
#define PLV_DEBUG_(text) { Serial.print( (text) ); }
#else
#define PLV_DEBUG_(text) {}
#endif

#if PLV_DEBUG_ENABLED
#define PLV_DEBUG_HEADER(text) {  Serial.println(F("\n")); Serial.println((text)); Serial.println(F("===========================================")); }
#else
#define PLV_DEBUG_HEADER(text) {}
#endif

#if PLV_DEBUG_ENABLED
#define PLV_DEBUG(text) { Serial.println( (text) ); }
#else
#define PLV_DEBUG(text) {}
#endif

// Serial debug
#if PLV_DEBUG_ENABLED
#define PLV_DEBUG_SETUP(baudrate) { Serial.begin( (baudrate) ); }
#else
#define PLV_DEBUG_SETUP(baudrate) {}
#endif

class DCPSystem {
public:
    DCPSystem();

    boolean test();
};