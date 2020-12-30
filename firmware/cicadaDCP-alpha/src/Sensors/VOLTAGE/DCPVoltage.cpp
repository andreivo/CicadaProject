/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */
#include "DCPVoltage.h"

DCPRTC vccRTC;
DCPSDCard vccSdCard;

DCPVoltage::DCPVoltage() {

}

/**
 * Setup Input Vcc Sensor
 */
void DCPVoltage::setupVccInSensor() {
    CIC_DEBUG_HEADER(F("SETUP INPUT VCC READ SENSOR"));
}

/**
 * Setup Solar Cell Vcc Sensor
 */
void DCPVoltage::setupVccSolSensor() {
    CIC_DEBUG_HEADER(F("SETUP SOLAR CELL VCC READ SENSOR"));
}

void DCPVoltage::initVccSensor(String _codeVccIn, String _typeVccIn, int timeSlotVccIn, String _codeVccSol, String _typeVccSol, int timeSlotVccSol) {
    initVccInSensor(_codeVccIn, _typeVccIn, timeSlotVccIn);
    initVccSolarCellSensor(_codeVccSol, _typeVccSol, timeSlotVccSol);
}

/**
 * Initialize Input Vcc Sensor
 */
void DCPVoltage::initVccInSensor(String _codeVccIn, String _typeVccIn, int timeSlotVccIn) {
    CIC_DEBUG_HEADER(F("INIT INPUT VCC READ SENSOR"));

    codeVccIn = _codeVccIn;
    typeVccIn = _typeVccIn;

    TIME_TO_READ_VCCIN = timeSlotVccIn;

    CIC_DEBUG_(F("Slot Time Input Vcc read: "));
    CIC_DEBUG_(TIME_TO_READ_VCCIN);
    CIC_DEBUG(F(" min."));

    nextVccInSlotTimeToRead();
}

/**
 * Initialize Solar Cell Sensor
 */
void DCPVoltage::initVccSolarCellSensor(String _codeVccSol, String _typeVccSol, int timeSlotVccSol) {
    CIC_DEBUG_HEADER(F("INIT SOLAR CELL VCC READ SENSOR"));

    codeVccSol = _codeVccSol;
    typeVccSol = _typeVccSol;

    TIME_TO_READ_VCCSOL = timeSlotVccSol;

    CIC_DEBUG_(F("Slot Time Solar Cell Vcc read: "));
    CIC_DEBUG_(TIME_TO_READ_VCCSOL);
    CIC_DEBUG(F(" min."));

    nextVccSolSlotTimeToRead();
}

int DCPVoltage::nextSlotTimeToRead(int TIME_TO_READ) {
    int actualMinutes = vccRTC.now("%M").toInt() + 1;

    int rSlot = actualMinutes % TIME_TO_READ;
    int iSlot = (int) (actualMinutes / TIME_TO_READ);

    if (rSlot > 0) {
        iSlot = iSlot + 1;
    }

    int nextSlot = iSlot*TIME_TO_READ;
    if (nextSlot >= 60) {
        nextSlot = nextSlot - 60;
    }
    return nextSlot;
}

void DCPVoltage::nextVccInSlotTimeToRead() {
    nextSlotVccIn = nextSlotTimeToRead(TIME_TO_READ_VCCIN);
    CIC_DEBUG_(F("Next slot to read Input Vcc: "));
    CIC_DEBUG_(nextSlotVccIn);
    CIC_DEBUG(F(" min."));
}

void DCPVoltage::nextVccSolSlotTimeToRead() {
    nextSlotVccSol = nextSlotTimeToRead(TIME_TO_READ_VCCSOL);
    CIC_DEBUG_(F("Next slot to read Solar Cell Vcc: "));
    CIC_DEBUG_(nextSlotVccSol);
    CIC_DEBUG(F(" min."));
}

boolean DCPVoltage::timeToReadVccIn() {
    int actualMinutes = vccRTC.now("%M").toInt();
    return actualMinutes == nextSlotVccIn;
}

boolean DCPVoltage::timeToReadVccSol() {
    int actualMinutes = vccRTC.now("%M").toInt();
    return actualMinutes == nextSlotVccSol;
}

void DCPVoltage::readVccIn() {
    if (timeToReadVccIn()) {
        CIC_DEBUG_HEADER(F("READ INPUT VCC"));
        int adcValue = analogRead(PIN_ADC_VCC_IN);
        float vccin = ADC2VOLTS(adcValue);

        String collectionDate = vccRTC.now("%Y-%m-%d %H:%M:%SZ");
        String dataContent = vccSdCard.prepareData(codeVccIn, typeVccIn, collectionDate, String(vccin));
        if (!vccSdCard.storeData("vcc", dataContent)) {
            CIC_DEBUG(F("Error store INPUT VCC Data!"));
        } else {
            CIC_DEBUG(F("Store INPUT VCC Data!"));
        }

        nextVccInSlotTimeToRead();
    }
}

void DCPVoltage::readVccSol() {
    if (timeToReadVccSol()) {
        CIC_DEBUG_HEADER(F("READ SOLAR CELL VCC"));
        int adcValue = analogRead(PIN_ADC_VCC_SO);
        float vccin = ADC2VOLTS(adcValue);

        String collectionDate = vccRTC.now("%Y-%m-%d %H:%M:%SZ");
        String dataContent = vccSdCard.prepareData(codeVccSol, typeVccSol, collectionDate, String(vccin));
        if (!vccSdCard.storeData("vcc", dataContent)) {
            CIC_DEBUG(F("Error store SOLAR CELL VCC Data!"));
        } else {
            CIC_DEBUG(F("Store SOLAR CELL VCC Data!"));
        }

        nextVccSolSlotTimeToRead();
    }
}