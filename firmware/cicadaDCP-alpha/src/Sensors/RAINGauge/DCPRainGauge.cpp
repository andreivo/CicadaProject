/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */
#include "DCPRainGauge.h"

DCPRTC rgRTC;
DCPSDCard rgSdCard;

portMUX_TYPE rgMux = portMUX_INITIALIZER_UNLOCKED;
volatile int tipBucketCounter = 0;
volatile uint32_t lastDebounceTimeout = 0;
volatile float rainVolume = 0; // The rain accumulated volume

void IRAM_ATTR handleBucketInterrupt() {
    portENTER_CRITICAL_ISR(&rgMux);
    uint32_t actualDebounceTimeout = xTaskGetTickCount();
    uint32_t periodDebounceTimeout = actualDebounceTimeout - lastDebounceTimeout;
    if (periodDebounceTimeout >= RG_DEBOUNCETIME) {
        tipBucketCounter++;
        lastDebounceTimeout = xTaskGetTickCount();
    }
    portEXIT_CRITICAL_ISR(&rgMux);
}

DCPRainGauge::DCPRainGauge() {

}

/**
 * Setup RG Sensor
 */
void DCPRainGauge::setupRGSensor() {
    CIC_DEBUG_HEADER(F("SETUP RAIN GAUGE SENSOR"));
    pinMode(PIN_RG, INPUT_PULLUP); // Pull up to 3.3V on input - some buttons already have this done
    attachInterrupt(digitalPinToInterrupt(PIN_RG), handleBucketInterrupt, FALLING); //configura a interrupção do botão no evento CHANGE para a função handleButtonInterrupt
}

/**
 * Initialize RG Sensor
 */
void DCPRainGauge::initRGSensor(String _codeRG, String _typeRG, int timeSlotRG, float _bucketVol, float _collectionArea) {
    CIC_DEBUG_HEADER(F("INIT RAIN GAUGE SENSOR"));

    codeRG = _codeRG;
    typeRG = _typeRG;
    BUCKET_VOLUME = _bucketVol;
    CONTRIBUTION_AREA = _collectionArea;
    TIME_TO_READ_RG = timeSlotRG;

    CIC_DEBUG_(F("Slot Time Rain Gauge: "));
    CIC_DEBUG_(String(TIME_TO_READ_RG));
    CIC_DEBUG(F(" min."));

    nextSlotTimeToRead();
}

void DCPRainGauge::nextSlotTimeToRead() {
    int actualMinutes = rgRTC.now("%M").toInt() + 1;

    int rSlot = actualMinutes % TIME_TO_READ_RG;
    int iSlot = (int) (actualMinutes / TIME_TO_READ_RG);

    if (rSlot > 0) {
        iSlot = iSlot + 1;
    }

    int nextSlot = iSlot*TIME_TO_READ_RG;
    if (nextSlot >= 60) {
        nextSlot = nextSlot - 60;
    }
    nextSlotRG = nextSlot;
    CIC_DEBUG_(F("Next slot to read Rain Gauge: "));
    CIC_DEBUG_(String(nextSlotRG));
    CIC_DEBUG(F(" min."));
}

boolean DCPRainGauge::timeToReadRG() {
    int actualMinutes = rgRTC.now("%M").toInt();
    return actualMinutes == nextSlotRG;
}

void DCPRainGauge::readRG() {
    if (timeToReadRG()) {
        CIC_DEBUG_HEADER(F("READ RAIN GAUGE"));

        portENTER_CRITICAL_ISR(&rgMux);
        int tbCounter = tipBucketCounter;
        portEXIT_CRITICAL_ISR(&rgMux);

        if (tbCounter > 0) {
            String collectionDate = rgRTC.now("%Y-%m-%d %H:%M:%SZ");

            rainVolume = tbCounter * BUCKET_VOLUME * 1000 / CONTRIBUTION_AREA;

            String context = "\"{'tip':'" + String(tbCounter) + "','bkt':'" + String(BUCKET_VOLUME) + "','are':'" + String(CONTRIBUTION_AREA) + "'}\"";
            String dataContent = rgSdCard.prepareData(codeRG, typeRG, collectionDate, String(rainVolume), context);
            if (!rgSdCard.storeData("rgs", dataContent)) {
                CIC_DEBUG(F("Error store RAIN GAUGE Data!"));
            } else {
                CIC_DEBUG(F("Store RAIN GAUGE Data!"));
            }

            portENTER_CRITICAL_ISR(&rgMux);
            tipBucketCounter = 0;
            portEXIT_CRITICAL_ISR(&rgMux);
        } else {
            CIC_DEBUG(F("No RAIN GAUGE Data to be read!"));
        }

        nextSlotTimeToRead();
    }
}

String DCPRainGauge::printTipBucket() {
    portENTER_CRITICAL_ISR(&rgMux);
    int tbCounter = tipBucketCounter;
    portEXIT_CRITICAL_ISR(&rgMux);

    if (tbCounter > 0) {
        return String(tbCounter);
    } else {
        return F("No Rain Gauge Data");
    }
}

void DCPRainGauge::updateNextSlot() {
    if (timeToReadRG()) {
        nextSlotTimeToRead();
    }
}

void DCPRainGauge::printNextSlot() {
    CIC_DEBUG_(F("Next slot to read Rain Gauge: "));
    CIC_DEBUG_(String(nextSlotRG));
    CIC_DEBUG(F(" Min."));
}