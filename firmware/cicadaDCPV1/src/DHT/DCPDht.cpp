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

SimpleDHT22 dht;

DCPDht::DCPDht() {
}

/**
 * Initialize DHT Sensor
 */
void DCPDht::initDHTSensor() {

    CIC_DEBUG_HEADER(F("INIT DHT22"));

}