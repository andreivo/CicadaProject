/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */
#include "DCPLeds.h"

#define TIME_TO_STATUS_BLINK 5

DCPRTC ledsRTC;

DCPLeds::DCPLeds() {
    lastEpoch = ledsRTC.nowEpoch();
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

void DCPLeds::blinkStatusOk() {
    greenTurnOff();
    greenTurnOn();
    delay(200);
    greenTurnOff();
    lastEpoch = ledsRTC.nowEpoch();
}

boolean DCPLeds::timeToBlinkStatus() {
    int32_t actualEpoch = ledsRTC.nowEpoch();
    int32_t periodEpoch = actualEpoch - lastEpoch;
    return (periodEpoch >= TIME_TO_STATUS_BLINK);
}

void DCPLeds::blinkStatusError() {
    int32_t actualEpoch = ledsRTC.nowEpoch();
    int32_t periodEpoch = actualEpoch - lastEpoch;

    if (periodEpoch >= TIME_TO_STATUS_BLINK) {
        redTurnOff();
        redTurnOn();
        delay(200);
        redTurnOff();
        lastEpoch = ledsRTC.nowEpoch();
    }

}