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

DCPRTC dhtRTC;
DCPSDCard dhtSdCard;

DCPDht::DCPDht() {
    lastEpTemp = dhtRTC.nowEpoch();
    lastEpHum = lastEpTemp;
}

/**
 * Initialize DHT Sensor
 */
//void DCPDht::initDHTSensor(String _codeTemp, String _typeTemp, String _codeHum, String _typeHum, int temp, int hum, DCPSDCard& dhtSDCard) {

void DCPDht::initDHTSensor(String _codeTemp, String _typeTemp, String _codeHum, String _typeHum, int temp, int hum) {
    CIC_DEBUG_HEADER(F("INIT DHT22"));

    codeTemp = _codeTemp;
    typeTemp = _typeTemp;
    codeHum = _codeHum;
    typeHum = _typeHum;

    //TIME_TO_READ_TEMP = 30;
    //TIME_TO_READ_HUM = 30;
    TIME_TO_READ_TEMP = 60 * temp;
    TIME_TO_READ_HUM = 60 * hum;

    CIC_DEBUG_(F("Time interval temp: "));
    CIC_DEBUG_(TIME_TO_READ_TEMP);
    CIC_DEBUG(F(" sec."));
    CIC_DEBUG_(F("Time interval hum: "));
    CIC_DEBUG_(TIME_TO_READ_HUM);
    CIC_DEBUG(F(" sec."));
}

void DCPDht::readDHT() {
    int32_t actualEpoch = dhtRTC.nowEpoch();
    int32_t periodEpochTemp = actualEpoch - lastEpTemp;
    int32_t periodEpochHum = actualEpoch - lastEpHum;

    if (periodEpochTemp >= TIME_TO_READ_TEMP) {
        CIC_DEBUG_HEADER(F("READ DHT22"));
        float t, h;
        if (dht.read2(PIN_DHT, &t, &h, NULL) == SimpleDHTErrSuccess) {

            CIC_DEBUG(F("Prepare DT Data!"));
            String collectionDate = dhtRTC.now("%Y-%m-%d %H:%M:%SZ");
            String dataContent = dhtSdCard.prepareData(codeTemp, typeTemp, collectionDate, String(t));

            if (periodEpochHum >= TIME_TO_READ_HUM) {
                CIC_DEBUG(F("Prepare DH Data!"));
                dataContent = dataContent + "," + dhtSdCard.prepareData(codeHum, typeHum, collectionDate, String(h));
                lastEpHum = dhtRTC.nowEpoch();
            }

            if (!dhtSdCard.storeData("dht", dataContent)) {
                CIC_DEBUG(F("Error store DHT Data!"));
            } else {
                CIC_DEBUG(F("Store DHT Data!"));
            }
        }
        lastEpTemp = dhtRTC.nowEpoch();
        if (periodEpochHum >= TIME_TO_READ_HUM) {
            lastEpHum = dhtRTC.nowEpoch();
        }
    } else {
        if (periodEpochHum >= TIME_TO_READ_HUM) {
            CIC_DEBUG_HEADER(F("READ DHT22"));
            float t, h;
            if (dht.read2(PIN_DHT, &t, &h, NULL) == SimpleDHTErrSuccess) {
                CIC_DEBUG(F("Prepare DH Data!"));
                String collectionDate = dhtRTC.now("%Y-%m-%d %H:%M:%SZ");
                String dataContent = dataContent + dhtSdCard.prepareData(codeHum, typeHum, collectionDate, String(h));

                if (!dhtSdCard.storeData("dht", dataContent)) {
                    CIC_DEBUG(F("Error store DHT Data!"));
                } else {
                    CIC_DEBUG(F("Store DH Data!"));
                }
            }
            lastEpHum = dhtRTC.nowEpoch();
        }
    }
}