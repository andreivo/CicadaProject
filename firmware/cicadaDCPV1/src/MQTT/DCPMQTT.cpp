/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */
#include "DCPMQTT.h"

DCPSDCard mqttSdCard;
DCPRTC mqttRTC;

DCPMQTT::DCPMQTT() {

}

/**
 * Setup WiFi module
 */
boolean DCPMQTT::setupMQTTModule(int timeToSend, String _DEVICE_ID, String _MQTT_SERVER, String _MQTT_PORT, String _MQTT_USER, String _MQTT_PWD, String _TOPIC, String _tknDCP, String _pwdDCP, String _LA, String _LO) {
    CIC_DEBUG_HEADER(F("SETUP MQTT MODULE"));

    TIME_TO_SEND = timeToSend;
    //TIME_TO_SEND = 5;
    CIC_DEBUG_(F("Slot Time to send: "));
    CIC_DEBUG_(TIME_TO_SEND);
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
    CIC_DEBUG_(nextSlotToSend);
    CIC_DEBUG(F(" min."));
}

boolean DCPMQTT::onTimeToSend() {
    int actualMinutes = mqttRTC.now("%M").toInt();
    return actualMinutes == nextSlotToSend;
}

void DCPMQTT::sendAllMessagesData(TinyGsmSim800 modem) {
    if (onTimeToSend()) {
        TinyGsmClient _clientTransport(modem);
        PubSubClient _clientPub(MQTT_SERVER.c_str(), MQTT_PORT.toInt(), _clientTransport);
        clientPub = &_clientPub;
        sendMessagesData();
    }
}

void DCPMQTT::sendAllMessagesData() {
    if (onTimeToSend()) {
        WiFiClient _clientTransport;
        PubSubClient _clientPub(MQTT_SERVER.c_str(), MQTT_PORT.toInt(), _clientTransport);
        clientPub = &_clientPub;
        sendMessagesData();
    }
}

void DCPMQTT::sendMessagesData() {
    CIC_DEBUG_HEADER(F("SEND MQTT MESSAGE"));
    CIC_DEBUG_("Time: ");
    CIC_DEBUG(mqttRTC.now());
    CIC_DEBUG_("On core: ");
    CIC_DEBUG(xPortGetCoreID());

    CIC_DEBUG("------------- (0)--------------");
    String fileData = mqttSdCard.getFirstFile("/");
    CIC_DEBUG("------------- (1)--------------");
    if (fileData != "") {
        if (connectMQTTServer()) {
            CIC_DEBUG("------------- (2)--------------");
            while (fileData != "") {
                CIC_DEBUG("------------- (3)--------------");
                if (mqttSdCard.readPublishFile(fileData, publishMessage, clientPub, tknDCP, pwdDCP, TOPIC)) {
                    CIC_DEBUG("------------- (4)--------------");
                    mqttSdCard.deleteFile(fileData);
                    CIC_DEBUG("------------- (5)--------------");
                    fileData = mqttSdCard.getFirstFile("/");
                    CIC_DEBUG("------------- (6)--------------");
                } else {
                    mqttSdCard.deleteFile(fileData);
                    CIC_DEBUG("------------- (7)--------------");
                    fileData = mqttSdCard.getFirstFile("/");
                    CIC_DEBUG("------------- (8)--------------");
                    CIC_DEBUG_(F("Error on sending: "));
                    CIC_DEBUG("------------- (9)--------------");
                    CIC_DEBUG(fileData);
                }
            }
            CIC_DEBUG("------------- (10)--------------");
        }
    } else {
        CIC_DEBUG(F("No messages to send!"))
    }
    nextSlotTimeToSend();
    CIC_DEBUG(F("Finished send MQTT messages!"))
    delay(60000);
}

boolean DCPMQTT::connectMQTTServer() {
    CIC_DEBUG("Connecting to MQTT Server...");
    if (!clientPub->connected()) {
        //Se conecta ao device que definimos
        CIC_DEBUG("------------- (0.1)--------------");
        vTaskDelay(100);
        if (clientPub->connect(DEVICE_ID.c_str(), MQTT_USER.c_str(), MQTT_PWD.c_str())) {
            CIC_DEBUG("------------- (0.2)--------------");
            //Se a conexão foi bem sucedida
            CIC_DEBUG("Connected!");
            CIC_DEBUG("------------- (0.3)--------------");
            return true;
        } else {
            //Se ocorreu algum erro
            CIC_DEBUG_("Error: ");
            CIC_DEBUG(clientPub->state());
            return false;
        }
    } else {//Se a conexão foi bem sucedida
        CIC_DEBUG("Connected!");
        CIC_DEBUG("------------- (0.3)--------------");
        return true;
    }
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
        if (takeCommunicationMutex()) {
            vTaskDelay(100);
            int status = _clientPub->publish(TOPIC.c_str(), pkgMsg.c_str());
            if (status == 1) {
                CIC_DEBUG(F("Published successful")); //Status 1 se sucesso ou 0 se deu erro
                giveCommunicationMutex();
                vTaskDelay(100);
                return true;
            } else {
                CIC_DEBUG_(F("Error status: "));
                CIC_DEBUG(String(status)); //Status 1 se sucesso ou 0 se deu erro
                giveCommunicationMutex();
                vTaskDelay(100);
                return false;
            }
        } else {
            CIC_DEBUG("Waiting modem to publishMessage ...");
        }
        attempts = attempts + 1;
        vTaskDelay(SIM_ATTEMPTS_DELAY);
    }
}

