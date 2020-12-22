/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */
#include "DCPLeds.h"

DCPLeds::DCPLeds() {
}

void DCPLeds::redTurnOn() {
    digitalWrite(PIN_LED_RED, HIGH);
    redOn = true;
}

void DCPLeds::redTurnOff() {
    digitalWrite(PIN_LED_RED, LOW);
    redOn = false;
}

void DCPLeds::redBlink() {
    digitalWrite(PIN_LED_RED, redOn);
    redOn = !redOn;
}

void DCPLeds::redBlink(int times) {
    for (int i = 0; i < times; i++) {
        redBlink();
        delay(LEDDELAYTIME);
    }
}

void DCPLeds::redBlink(int times, int delaytime) {
    for (int i = 0; i < times; i++) {
        redBlink();
        delay(delaytime);
    }
}

void DCPLeds::greenTurnOn() {
    digitalWrite(PIN_LED_GREEN, HIGH);
    greenOn = true;
}

void DCPLeds::greenTurnOff() {
    digitalWrite(PIN_LED_GREEN, LOW);
    greenOn = false;
}

void DCPLeds::greenBlink() {
    digitalWrite(PIN_LED_GREEN, greenOn);
    greenOn = !greenOn;
}

void DCPLeds::greenBlink(int times) {
    for (int i = 0; i < times; i++) {
        greenBlink();
        delay(LEDDELAYTIME);
    }
}

void DCPLeds::greenBlink(int times, int delaytime) {
    for (int i = 0; i < times; i++) {
        greenBlink();
        delay(delaytime);
    }
}

void DCPLeds::blueTurnOn() {
    digitalWrite(PIN_LED_BLUE, HIGH);
    blueOn = true;
}

void DCPLeds::blueTurnOff() {
    digitalWrite(PIN_LED_BLUE, LOW);
    blueOn = false;
}

void DCPLeds::blueBlink() {
    digitalWrite(PIN_LED_BLUE, blueOn);
    blueOn = !blueOn;
}

void DCPLeds::blueBlink(int times) {
    for (int i = 0; i < times; i++) {
        blueBlink();
        delay(LEDDELAYTIME);
    }
}

void DCPLeds::blueBlink(int times, int delaytime) {
    for (int i = 0; i < times; i++) {
        blueBlink();
        delay(delaytime);
    }
}
