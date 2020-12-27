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

}

/**
 * Initialize DHT Sensor
 */
void DCPDht::initDHTSensor(String _codeTemp, String _typeTemp, String _codeHum, String _typeHum, int temp, int hum) {
    CIC_DEBUG_HEADER(F("INIT DHT22"));

    codeTemp = _codeTemp;
    typeTemp = _typeTemp;
    codeHum = _codeHum;
    typeHum = _typeHum;

    //TIME_TO_READ_TEMP = 30;
    //TIME_TO_READ_HUM = 30;
    //TIME_TO_READ_TEMP = 60 * temp;
    //TIME_TO_READ_HUM = 60 * hum;
    TIME_TO_READ_TEMP = temp;
    TIME_TO_READ_HUM = hum;

    nextSlotTimeToReadTemp();
    nextSlotTimeToReadHum();

    CIC_DEBUG_(F("Slot Time temp: "));
    CIC_DEBUG_(TIME_TO_READ_TEMP);
    CIC_DEBUG(F(" min."));
    CIC_DEBUG_(F("Slot Time hum: "));
    CIC_DEBUG_(TIME_TO_READ_HUM);
    CIC_DEBUG(F(" min."));
}

int DCPDht::nextSlotTimeToRead(int TIME_TO_READ) {
    int actualMinutes = dhtRTC.now("%M").toInt() + 1;

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

void DCPDht::nextSlotTimeToReadTemp() {
    nextSlotTemp = nextSlotTimeToRead(TIME_TO_READ_TEMP);
    CIC_DEBUG_(F("Next slot to read temp: "));
    CIC_DEBUG_(nextSlotTemp);
    CIC_DEBUG(F("min."));
}

boolean DCPDht::timeToReadTemp() {
    int actualMinutes = dhtRTC.now("%M").toInt();
    return actualMinutes == nextSlotTemp;
}

void DCPDht::nextSlotTimeToReadHum() {
    nextSlotHum = nextSlotTimeToRead(TIME_TO_READ_HUM);
    CIC_DEBUG_(F("Next slot to read hum: "));
    CIC_DEBUG_(nextSlotHum);
    CIC_DEBUG(F("min."));
}

boolean DCPDht::timeToReadHum() {
    int actualMinutes = dhtRTC.now("%M").toInt();
    return actualMinutes == nextSlotHum;
}

void DCPDht::readDHT() {
    if (timeToReadTemp()) {
        CIC_DEBUG_HEADER(F("READ DHT22"));
        float t, h;
        if (dht.read2(PIN_DHT, &t, &h, NULL) == SimpleDHTErrSuccess) {
            CIC_DEBUG(F("Prepare DT Data!"));
            String collectionDate = dhtRTC.now("%Y-%m-%d %H:%M:%SZ");
            String dataContent = dhtSdCard.prepareData(codeTemp, typeTemp, collectionDate, String(t));

            if (timeToReadHum()) {
                CIC_DEBUG(F("Prepare DH Data!"));
                dataContent = dataContent + "," + dhtSdCard.prepareData(codeHum, typeHum, collectionDate, String(h));
                nextSlotTimeToReadHum();
            }

            if (!dhtSdCard.storeData("dht", dataContent)) {
                CIC_DEBUG(F("Error store DHT Data!"));
            } else {
                CIC_DEBUG(F("Store DHT Data!"));
            }
        }
        nextSlotTimeToReadTemp();
    } else {
        if (timeToReadHum()) {
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
            nextSlotTimeToReadHum();
        }
    }
}