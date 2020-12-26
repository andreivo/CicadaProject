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
    lastEp = mqttRTC.nowEpoch();
}

/**
 * Setup WiFi module
 */
boolean DCPMQTT::setupMQTTModule(int timeToSend, String _DEVICE_ID, String _MQTT_SERVER, String _MQTT_PORT, String _MQTT_USER, String _MQTT_PWD, String _TOPIC, String _tknDCP, String _pwdDCP, String _LA, String _LO) {
    CIC_DEBUG_HEADER(F("SETUP MQTT MODULE"));

    TIME_TO_SEND = (60 * timeToSend);
    //TIME_TO_SEND = (120);
    CIC_DEBUG_(F("Send time interval: "));
    CIC_DEBUG_(TIME_TO_SEND);
    CIC_DEBUG(F(" sec."));

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

void DCPMQTT::sendAllMessagesData(TinyGsmSim800 modem) {

    if (onTimeToSend()) {
        //if (clientPub == NULL) {
        TinyGsmClient _clientTransport(modem);
        PubSubClient _clientPub(MQTT_SERVER.c_str(), MQTT_PORT.toInt(), _clientTransport);
        clientPub = &_clientPub;
        //}
        sendMessagesData();
    }
}

void DCPMQTT::sendAllMessagesData() {
    if (onTimeToSend()) {
        //if (clientPub == NULL) {
        WiFiClient _clientTransport;
        PubSubClient _clientPub(MQTT_SERVER.c_str(), MQTT_PORT.toInt(), _clientTransport);
        clientPub = &_clientPub;
        //}
        sendMessagesData();
    }
}

boolean DCPMQTT::onTimeToSend() {
    int32_t actualEpoch = mqttRTC.nowEpoch();
    int32_t periodEpoch = actualEpoch - lastEp;
    return periodEpoch >= TIME_TO_SEND;
}

void DCPMQTT::sendMessagesData() {
    CIC_DEBUG_HEADER(F("SEND MQTT MESSAGE"));
    CIC_DEBUG_("Time: ");
    CIC_DEBUG(mqttRTC.now());
    CIC_DEBUG_("On core: ");
    CIC_DEBUG(xPortGetCoreID());

    if (connectMQTTServer()) {

        String fileData = mqttSdCard.getFirstFile("/");
        while (fileData != "") {
            String sFileContent = mqttSdCard.readFile(fileData);
            char fileContent[sFileContent.length() + 1];
            sFileContent.toCharArray(fileContent, sFileContent.length() + 1);

            String sendMessage = "";
            for (int i = 0; i < sizeof (fileContent) - 1; i++) {
                if (fileContent[i] == '\n') {
                    String pkgMsg = prepareMessage(sendMessage);
                    CIC_DEBUG(pkgMsg);
                    int status = clientPub->publish(TOPIC.c_str(), pkgMsg.c_str());
                    if (status == 1) {
                        CIC_DEBUG(F("Published successful")); //Status 1 se sucesso ou 0 se deu erro
                    } else {
                        CIC_DEBUG_(F("Error status: "));
                        CIC_DEBUG(String(status)); //Status 1 se sucesso ou 0 se deu erro
                    }
                    sendMessage = "";
                } else {
                    sendMessage = sendMessage + fileContent[i];
                }
            }
            mqttSdCard.deleteFile(fileData);
            fileData = mqttSdCard.getFirstFile("/");
        }
        lastEp = mqttRTC.nowEpoch();
    }
    CIC_DEBUG(F("Finished send MQTT messages!"))
    delay(60000);
}

String DCPMQTT::prepareMessage(String payload) {
    String sntDT = mqttRTC.now("%Y-%m-%d %H:%M:%SZ");
    String result = "{\"tknDCP\":\"" + tknDCP + "\",\"pwdDCP\":\"" + pwdDCP + "\",\"sntDT\":\"" + sntDT + "\",";
    result = result + payload + "}";
    return result;
}

boolean DCPMQTT::connectMQTTServer() {
    CIC_DEBUG("Connecting to MQTT Server...");
    //Se conecta ao device que definimos
    if (clientPub->connect(DEVICE_ID.c_str(), MQTT_USER.c_str(), MQTT_PWD.c_str())) {
        //Se a conexão foi bem sucedida
        CIC_DEBUG("Connected!");
        return true;
    } else {
        //Se ocorreu algum erro
        CIC_DEBUG_("Error: ");
        CIC_DEBUG(clientPub->state());
        return false;
    }
}