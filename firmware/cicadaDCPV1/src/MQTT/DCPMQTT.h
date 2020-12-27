/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#ifndef DCPMQTT_h
#define DCPMQTT_h
#include "../system/DCPSystem.h"
#define TINY_GSM_MODEM_SIM800 //Tipo de modem que estamos usando
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include "../SDCard/DCPSDCard.h"

String prepareMessage(String payload, String tknDCP, String pwdDCP);
boolean publishMessage(String sendMessage, PubSubClient* _clientPub, String tknDCP, String pwdDCP, String TOPIC);

class DCPMQTT {
public:
    DCPMQTT();
    void sendMessagesData();
    void sendAllMessagesData();
    void sendAllMessagesData(TinyGsmSim800 modem);
    boolean connectMQTTServer();
    boolean setupMQTTModule(int timeToSend, String _DEVICE_ID, String _MQTT_SERVER, String _MQTT_PORT, String _MQTT_USER, String _MQTT_PWD, String _TOPIC, String _tknDCP, String _pwdDCP, String _LA, String _LO);
    boolean onTimeToSend();

private:
    int32_t lastEp;
    PubSubClient* clientPub;
    int TIME_TO_SEND = (60 * 10);
    String DEVICE_ID;
    String MQTT_SERVER;
    String MQTT_PORT;
    String MQTT_USER;
    String MQTT_PWD;
    String TOPIC;
    String tknDCP;
    String pwdDCP;
    String LA;
    String LO;

};

#endif