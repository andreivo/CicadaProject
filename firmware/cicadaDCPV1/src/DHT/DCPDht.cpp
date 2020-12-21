/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */
#include "DCPDht.h"
#include <SimpleDHT.h>
#include "../system/DCPSystem.h"

#define DHT_PIN 27
SimpleDHT22 dht;

DCPDht::DCPDht() {
}

/**
 * Initialize DHT Sensor
 */
void DCPDht::initDHTSensor() {

    CIC_DEBUG_HEADER(F("INIT DHT22"));

}