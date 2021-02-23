/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */
#include "DCPMQTT.h"

DCPSDCard mqttSdCard;
DCPRTC mqttRTC;
DCPLeds mqttLeds;
// Initialize DCPSIM800
DCPSIM800 mqttSIM800;
DCPwifi mqttWifi;

DCPMQTT::DCPMQTT() {

}

/**
 * Setup WiFi module
 */
boolean DCPMQTT::setupMQTTModule(int timeToSend, String _DEVICE_ID, String _MQTT_SERVER, String _MQTT_PORT, String _MQTT_USER, String _MQTT_PWD, String _TOPIC, String _tknDCP, String _pwdDCP, String _LA, String _LO) {
    CIC_DEBUG_HEADER(F("SETUP MQTT MODULE"));

    TIME_TO_SEND = timeToSend;
    CIC_DEBUG_(F("Slot Time to send: "));
    CIC_DEBUG_(String(TIME_TO_SEND));
    CIC_DEBUG(F(" min."));

    nextSlotTimeToSend();

    DEVICE_ID = _DEVICE_ID;
    MQTT_SERVER = _MQTT_SERVER;
    MQTT_PORT = _MQTT_PORT;
    MQTT_USER = _MQTT_USER;
    MQTT_PWD = _MQTT_PWD;
    TOPIC = _TOPIC;
    tknDCP = _tknDCP;
    pwdDCP = _pwdDCP;
    LA = _LA;
    LO = _LO;
}

void DCPMQTT::nextSlotTimeToSend() {
    int actualMinutes = mqttRTC.now("%M").toInt() + 1;

    int rSlot = actualMinutes % TIME_TO_SEND;
    int iSlot = (int) (actualMinutes / TIME_TO_SEND);

    if (rSlot > 0) {
        iSlot = iSlot + 1;
    }

    int nextSlot = iSlot*TIME_TO_SEND;
    if (nextSlot >= 60) {
        nextSlot = nextSlot - 60;
    }

    nextSlotToSend = nextSlot;
    CIC_DEBUG_(F("Next slot to send: "));
    CIC_DEBUG_(String(nextSlotToSend));
    CIC_DEBUG(F(" min."));
}

boolean DCPMQTT::onTimeToSend() {
    int actualMinutes = mqttRTC.now("%M").toInt();
    int actualSeconds = mqttRTC.now("%S").toInt();
    return (actualMinutes == nextSlotToSend) && (actualSeconds >= 30) && (actualSeconds <= 59);
}

boolean DCPMQTT::sendAllMessagesDataSim(TinyGsmSim800 modem) {
    if (onTimeToSend()) {
        mqttLeds.blueTurnOn();
        TinyGsmClient _clientTransport(modem);
        PubSubClient _clientPub(MQTT_SERVER.c_str(), MQTT_PORT.toInt(), _clientTransport);
        clientPub = &_clientPub;
        clientPub->setSocketTimeout(20);
        sendMessagesData();
        mqttLeds.blueTurnOff();
        return true;
    } else {
        return true;
    }
}

boolean DCPMQTT::sendAllMessagesDataWifi() {
    if (onTimeToSend()) {
        mqttLeds.blueTurnOn();
        WiFiClient _clientTransport;
        PubSubClient _clientPub(MQTT_SERVER.c_str(), MQTT_PORT.toInt(), _clientTransport);
        clientPub = &_clientPub;
        clientPub->setSocketTimeout(5);
        sendMessagesData();
        mqttLeds.blueTurnOff();
        return true;
    } else {
        return true;
    }
}

void DCPMQTT::sendMessagesData() {
    CIC_DEBUG_HEADER(F("SEND MQTT MESSAGE"));
    CIC_DEBUG_("Time: ");
    CIC_DEBUG(mqttRTC.now());
    CIC_DEBUG_("On core: ");
    CIC_DEBUG(String(xPortGetCoreID()));

    if (connectMQTTServer()) {
        if (!mqttSdCard.mqttPublishFiles(publishMessage, clientPub, tknDCP, pwdDCP, TOPIC)) {
            CIC_DEBUG(F("Error on MQTT messages publication."));
        } else {
            CIC_DEBUG(F("Successful MQTT messages publication."));
        }
    } else {
        CIC_DEBUG(F("MQTT Server connection failure!"));
    }

    nextSlotTimeToSend();
    CIC_DEBUG(F("Finished send MQTT messages!"));
}

boolean DCPMQTT::connectMQTTServer() {
    CIC_DEBUG("Connecting to MQTT Server...");
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutex("connectMQTTServer")) {
            if (!clientPub->connected()) {
                int attemptsConn = 1;
                while (attemptsConn <= 5) {
                    if (clientPub->connect(DEVICE_ID.c_str(), MQTT_USER.c_str(), MQTT_PWD.c_str())) {
                        CIC_DEBUG("Connected!");
                        giveCommunicationMutex("connectMQTTServer");
                        return true;
                    } else {
                        CIC_DEBUG_("Error: ");
                        CIC_DEBUG(String(clientPub->state()));
                        mqttLeds.redTurnOff();
                        mqttLeds.redBlink(2, 200);
                        mqttLeds.redTurnOff();
                    }
                    attemptsConn = attemptsConn + 1;
                }
                giveCommunicationMutex("connectMQTTServer");
                return false;
            } else {
                CIC_DEBUG("Connected!");
                giveCommunicationMutex("connectMQTTServer");
                return true;
            }
        } else {
            CIC_DEBUG("Waiting modem to connectMQTTServer ...");
        }
        attempts = attempts + 1;
        vTaskDelay(pdMS_TO_TICKS(SIM_ATTEMPTS_DELAY));
    }
    return false;
}

/******************************************************************************
 ******************************************************************************/

String prepareMessage(String payload, String tknDCP, String pwdDCP) {
    String sntDT = mqttRTC.now("%Y-%m-%d %H:%M:%SZ");
    String result = "{\"tknDCP\":\"" + tknDCP + "\",\"pwdDCP\":\"" + pwdDCP + "\",\"sntDT\":\"" + sntDT + "\",";
    result = result + payload + "}";
    return result;
}

boolean publishMessage(String sendMessage, PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC) {
    String pkgMsg = prepareMessage(sendMessage, tknDCP, pwdDCP);
    int attempts = 0;
    while (attempts <= SIM_ATTEMPTS) {
        if (takeCommunicationMutex("publishMessage")) {
            int status = _clientPub->publish(TOPIC.c_str(), pkgMsg.c_str());
            if (status == 1) {
                CIC_DEBUG(F("Published successful")); //Status 1 se sucesso ou 0 se deu erro
                giveCommunicationMutex("publishMessage");
                return true;
            } else {
                CIC_DEBUG_(F("Published unsuccessful! Error status: "));
                CIC_DEBUG(String(status)); //Status 1 se sucesso ou 0 se deu erro
                giveCommunicationMutex("publishMessage");
                return false;
            }
        } else {
            CIC_DEBUG("Waiting modem to publishMessage ...");
        }
        attempts = attempts + 1;
        vTaskDelay(pdMS_TO_TICKS(SIM_ATTEMPTS_DELAY));
    }
    CIC_DEBUG("Published unsuccessful!");
    return false;
}

void DCPMQTT::updateNextSlot() {
    if (onTimeToSend()) {
        nextSlotTimeToSend();
    }
}

void DCPMQTT::printNextSlot() {
    Serial.print(F("Next slot to send: "));
    Serial.print(nextSlotToSend);
    Serial.println(F(" Min."));
}