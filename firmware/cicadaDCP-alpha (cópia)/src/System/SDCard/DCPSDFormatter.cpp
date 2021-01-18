/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */
#include "DCPSDFormatter.h"

//// SdFat software SPI template
SoftSpiDriver<PIN_SDCARD_MISO, PIN_SDCARD_MOSI, PIN_SDCARD_SCK> softSpiFormatter;
//// Speed argument is ignored for software SPI.
#define SD_CONFIGFORMATTER SdSpiConfig(PIN_SDCARD_CHIP_SELECT, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpiFormatter)

DCPSDFormatter::DCPSDFormatter() {
}

void DCPSDFormatter::formatSD(boolean ask) {
    char command;
    Serial.println(F("\n\nFORMATTING WEATHER FILE SYSTEM"));
    Serial.println(F("=========================================================="));

    if (ask) {
        Serial.println(F("WARNING.....WARNING.....WARNING.....\n"
                "Warning, all data on the card will be erased.\n\n"
                "Enter 'Y' to continue: "));
        while (!Serial.available()) {
            SysCall::yield();
        }
        command = Serial.read();
        if (command != 'Y') {
            Serial.println(F("Quiting, you did not enter 'Y'."));
            // Read any existing Serial data.
            clearSerialInput();
            return;
        }

        // Read any existing Serial data.
        clearSerialInput();
    }

    // SdCardFactory constructs and initializes the appropriate card.
    SdCardFactory cardFactory;
    // Pointer to generic SD card.
    SdCard* m_card = nullptr;

    // Select and initialize proper card driver.
    m_card = cardFactory.newCard(SD_CONFIGFORMATTER);
    if (!m_card || m_card->errorCode()) {
        Serial.println(F("Card init failed."));
        return;
    }

    uint32_t cardSectorCount = m_card->sectorCount();
    if (!cardSectorCount) {
        Serial.println(F("Get sector count failed."));
        return;
    }

    Serial.print(F("\nCard size: "));
    Serial.print(cardSectorCount * 5.12e-7);
    Serial.print(F(" GB (GB = 1E9 bytes)\n"));
    Serial.print(F("Card size: "));
    Serial.print(cardSectorCount / 2097152.0);
    Serial.println(F(" GiB (GiB = 2^30 bytes)\n"));

    FatFormatter fatFormatter;
    uint8_t sectorBuffer[512];

    //if larger than 32GB.
    bool rtn = cardSectorCount > 67108864;
    if (rtn) {
        Serial.println(F("WARNING.....WARNING.....WARNING.....\n"
                "The card is larger than 32G. If you continue it will be formatted in Fat32.\n\n"
                "Enter 'Y' to continue: "));
        command = Serial.read();
        if (command != 'Y') {
            Serial.println(F("Quiting, you did not enter 'Y'."));
            // Read any existing Serial data.
            clearSerialInput();
            return;
        }
        // Read any existing Serial data.
        clearSerialInput();
    }

    fatFormatter.format(m_card, sectorBuffer, &Serial);

    if (!rtn) {
        if (!m_card) {
            Serial.println(F("Invalid SD_CONFIG"));
        } else if (m_card->errorCode()) {
            if (m_card->errorCode() == SD_CARD_ERROR_CMD0) {
                Serial.println(F("No card, wrong chip select pin, or wiring error?"));
            }
            printSdErrorSymbol(&Serial, m_card->errorCode());
            Serial.print(F(" = "));
            Serial.println(int(m_card->errorCode()));

            Serial.print(F("SD errorData = "));

            Serial.print(int(m_card->errorData()));
        }
    }
}

void DCPSDFormatter::clearSerialInput() {
    uint32_t m = micros();
    do {
        if (Serial.read() >= 0) {
            m = micros();
        }
    } while (micros() - m < 10000);
}
