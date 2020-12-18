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

#include <FS.h> // FS must be the first
#include "DCPSystem.h"

DCPSystem::DCPSystem() {
}

boolean DCPSystem::test() {
    Serial.println("teste");
    return true;
}
