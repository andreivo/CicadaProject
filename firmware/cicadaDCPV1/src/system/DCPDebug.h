/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#ifndef DCPDebug_h
#define DCPDebug_h

#include <FreeRTOS.h>

#define CIC_DEBUG_ENABLED true

//Mutex
SemaphoreHandle_t SerialMutex;

//inline boolean takeSerialMutex() {
//    return (xSemaphoreTake(SerialMutex, portMAX_DELAY) == pdTRUE);
//}
//
//inline void giveSerialMutex() {
//    xSemaphoreGive(SerialMutex);
//}

//boolean takeSerialMutex();

//void giveSerialMutex();


#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_(text) {Serial.print( (text) ); }
#else
#define CIC_DEBUG_(text) {}
#endif

#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_HEADER(text) {  Serial.println(F("\n")); Serial.println((text)); Serial.println(F("===========================================")); }
#else
#define CIC_DEBUG_HEADER(text) {}
#endif

#if CIC_DEBUG_ENABLED
#define CIC_DEBUG(text) { Serial.print( (text) ); Serial.print(F(" (Core: ")); Serial.print(xPortGetCoreID()); Serial.println(F(")")); }
#else
#define CIC_DEBUG(text) {}
#endif

// Serial debug
#if CIC_DEBUG_ENABLED
#define CIC_DEBUG_SETUP(baudrate) { Serial.begin( (baudrate) );  delay(200);}
#else
#define CIC_DEBUG_SETUP(baudrate) {}
#endif

#endif
