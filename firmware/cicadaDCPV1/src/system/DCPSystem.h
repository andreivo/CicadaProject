/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#ifndef DCPSystem_h
#define DCPSystem_h

#define CIC_DEBUG_ENABLED true

#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_(text) { Serial.print( (text) ); }
#else
#define CIC_DEBUG_(text) {}
#endif

#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_HEADER(text) {  Serial.println(F("\n")); Serial.println((text)); Serial.println(F("===========================================")); }
#else
#define CIC_DEBUG_HEADER(text) {}
#endif

#if CIC_DEBUG_ENABLED
#define CIC_DEBUG(text) { Serial.println( (text) ); }
#else
#define CIC_DEBUG(text) {}
#endif

// Serial debug
#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_SETUP(baudrate) { Serial.begin( (baudrate) );  delay(200);}
#else
#define CIC_DEBUG_SETUP(baudrate) {}
#endif

class DCPSystem {
public:
    DCPSystem();

    void setupWizard();

    void initStationID();

    void initStationName();

    void initFirmwareVersion();

    void initStationCoordinates();

    void initBucketVolume();

    void initSystem();

    String getFwmVersion();

    String getSSIDAP();

    void printConfiguration();
};

#endif