/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
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
void DCPDht::initDHTSensor(String _codeTemp, String _typeTemp, String _codeHum, String _typeHum, int timeSlotDHT) {
    CIC_DEBUG_HEADER(F("INIT DHT22"));

    codeTemp = _codeTemp;
    typeTemp = _typeTemp;
    codeHum = _codeHum;
    typeHum = _typeHum;

    TIME_TO_READ_DHT = timeSlotDHT;

    CIC_DEBUG_(F("Slot Time DHT: "));
    CIC_DEBUG_(String(TIME_TO_READ_DHT));
    CIC_DEBUG(F(" min."));

    nextSlotTimeToRead();
}

void DCPDht::nextSlotTimeToRead() {
    int actualMinutes = dhtRTC.now("%M").toInt() + 1;

    int rSlot = actualMinutes % TIME_TO_READ_DHT;
    int iSlot = (int) (actualMinutes / TIME_TO_READ_DHT);

    if (rSlot > 0) {
        iSlot = iSlot + 1;
    }

    int nextSlot = iSlot*TIME_TO_READ_DHT;
    if (nextSlot >= 60) {
        nextSlot = nextSlot - 60;
    }
    nextSlotDHT = nextSlot;
    CIC_DEBUG_(F("Next slot to read DHT: "));
    CIC_DEBUG_(String(nextSlotDHT));
    CIC_DEBUG(F(" min."));
}

boolean DCPDht::timeToReadDHT() {
    int actualMinutes = dhtRTC.now("%M").toInt();
    return actualMinutes == nextSlotDHT;
}

void DCPDht::readDHT() {
    if (timeToReadDHT()) {
        CIC_DEBUG_HEADER(F("READ DHT22"));
        float t, h;
        int attempts = 0;
        while (attempts <= 5) {
            if (dht.read2(PIN_DHT, &t, &h, NULL) == SimpleDHTErrSuccess) {
                CIC_DEBUG(F("Prepare DT Data!"));
                String collectionDate = dhtRTC.now("%Y-%m-%d %H:%M:%SZ");
                String dataContent = dhtSdCard.prepareData(codeTemp, typeTemp, collectionDate, String(t));

                CIC_DEBUG(F("Prepare DH Data!"));
                dataContent = dataContent + "," + dhtSdCard.prepareData(codeHum, typeHum, collectionDate, String(h));

                if (!dhtSdCard.storeData("dht", dataContent)) {
                    CIC_DEBUG(F("Error store DHT Data!"));
                } else {
                    CIC_DEBUG(F("Store DHT Data!"));
                }
                break;
            } else {
                CIC_DEBUG(F("Error reading DHT Sensor!"));
                delay(10);
            }
            attempts = attempts + 1;
        }
        nextSlotTimeToRead();
    }
}

String DCPDht::printDHT() {
    float t, h;
    delay(200);
    if (dht.read2(PIN_DHT, &t, &h, NULL) == SimpleDHTErrSuccess) {
        return "Temperature: " + String(t) + "\nHumidity: " + String(h);
    } else {
        return F("Error reading DHT Sensor!");
    }
}

void DCPDht::updateNextSlot() {
    if (timeToReadDHT()) {
        nextSlotTimeToRead();
    }
}

void DCPDht::printNextSlot() {
    Serial.print(F("Next slot to read DHT: "));
    Serial.print(nextSlotDHT);
    Serial.println(F(" Min."));
}