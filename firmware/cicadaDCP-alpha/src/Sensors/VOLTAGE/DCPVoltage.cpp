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
 * Setup Battery Vcc Sensor
 */
void DCPVoltage::setupVccBatSensor() {
    CIC_DEBUG_HEADER(F("SETUP BATTERY VCC READ SENSOR"));
}

void DCPVoltage::initVccSensor(String _codeVccIn, String _typeVccIn, int timeSlotVccIn, String _codeVccBat, String _typeVccBat, int timeSlotVccBat) {
    pinMode(PIN_ADC_VCC_EN, OUTPUT);
    digitalWrite(PIN_ADC_VCC_EN, HIGH);

    initVccInSensor(_codeVccIn, _typeVccIn, timeSlotVccIn);
    initVccBatarCellSensor(_codeVccBat, _typeVccBat, timeSlotVccBat);
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
    CIC_DEBUG_(String(TIME_TO_READ_VCCIN));
    CIC_DEBUG(F(" min."));

    nextVccInSlotTimeToRead();
}

/**
 * Initialize Battery Sensor
 */
void DCPVoltage::initVccBatarCellSensor(String _codeVccBat, String _typeVccBat, int timeSlotVccBat) {
    CIC_DEBUG_HEADER(F("INIT BATTERY VCC READ SENSOR"));

    codeVccBat = _codeVccBat;
    typeVccBat = _typeVccBat;

    TIME_TO_READ_VCCSOL = timeSlotVccBat;

    CIC_DEBUG_(F("Slot Time Battery Vcc read: "));
    CIC_DEBUG_(String(TIME_TO_READ_VCCSOL));
    CIC_DEBUG(F(" min."));

    nextVccBatSlotTimeToRead();
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
    CIC_DEBUG_(String(nextSlotVccIn));
    CIC_DEBUG(F(" min."));
}

void DCPVoltage::nextVccBatSlotTimeToRead() {
    nextSlotVccBat = nextSlotTimeToRead(TIME_TO_READ_VCCSOL);
    CIC_DEBUG_(F("Next slot to read Battery Vcc: "));
    CIC_DEBUG_(String(nextSlotVccBat));
    CIC_DEBUG(F(" min."));
}

boolean DCPVoltage::timeToReadVccIn() {
    int actualMinutes = vccRTC.now("%M").toInt();
    return actualMinutes == nextSlotVccIn;
}

boolean DCPVoltage::timeToReadVccBat() {
    int actualMinutes = vccRTC.now("%M").toInt();
    return actualMinutes == nextSlotVccBat;
}

void DCPVoltage::readVccIn() {
    if (timeToReadVccIn()) {
        CIC_DEBUG_HEADER(F("READ INPUT VCC"));
        digitalWrite(PIN_ADC_VCC_EN, LOW);
        delay(100);
        int adcValue = analogRead(PIN_ADC_VCC_IN);
        digitalWrite(PIN_ADC_VCC_EN, HIGH);
        float vccin = ADC2VOLTS(adcValue);

        String collectionDate = vccRTC.now("%Y-%m-%d %H:%M:%SZ");
        String dataContent = vccSdCard.prepareData(codeVccIn, typeVccIn, collectionDate, String(vccin));
        if (!vccSdCard.storeData("vin", dataContent)) {
            CIC_DEBUG(F("Error store INPUT VCC Data!"));
        } else {
            CIC_DEBUG(F("Store INPUT VCC Data!"));
        }

        nextVccInSlotTimeToRead();
    }
}

void DCPVoltage::readVccBat() {
    if (timeToReadVccBat()) {
        CIC_DEBUG_HEADER(F("READ BATTERY VCC"));
        digitalWrite(PIN_ADC_VCC_EN, LOW);
        delay(100);
        int adcValue = analogRead(PIN_ADC_VCC_BA);
        digitalWrite(PIN_ADC_VCC_EN, HIGH);
        float vccin = ADC2VOLTS(adcValue);

        String collectionDate = vccRTC.now("%Y-%m-%d %H:%M:%SZ");
        String dataContent = vccSdCard.prepareData(codeVccBat, typeVccBat, collectionDate, String(vccin));
        if (!vccSdCard.storeData("vso", dataContent)) {
            CIC_DEBUG(F("Error store BATTERY VCC Data!"));
        } else {
            CIC_DEBUG(F("Store BATTERY VCC Data!"));
        }

        nextVccBatSlotTimeToRead();
    }
}

String DCPVoltage::printVccIn() {
    digitalWrite(PIN_ADC_VCC_EN, LOW);
    delay(100);
    int adcValue = analogRead(PIN_ADC_VCC_IN);
    digitalWrite(PIN_ADC_VCC_EN, HIGH);
    float vccin = ADC2VOLTS(adcValue);
    return String(vccin) + "v";
}

String DCPVoltage::printVccBat() {
    digitalWrite(PIN_ADC_VCC_EN, LOW);
    delay(100);
    int adcValue = analogRead(PIN_ADC_VCC_BA);
    digitalWrite(PIN_ADC_VCC_EN, HIGH);
    float vccin = ADC2VOLTS(adcValue);
    return String(vccin) + "v";
}

void DCPVoltage::updateNextSlotIn() {
    if (timeToReadVccIn()) {
        nextVccInSlotTimeToRead();
    }
}

void DCPVoltage::updateNextSlotBat() {
    if (timeToReadVccBat()) {
        nextVccBatSlotTimeToRead();
    }
}

void DCPVoltage::printNextVccInSlot() {
    Serial.print(F("Next slot to read Input Vcc: "));
    Serial.print(nextSlotVccIn);
    Serial.println(F(" Min."));
}

void DCPVoltage::printNextVccBatSlot() {
    Serial.print(F("Next slot to read Battery Vcc: "));
    Serial.print(nextSlotVccBat);
    Serial.println(F(" Min."));
}