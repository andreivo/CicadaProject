/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

#ifndef DCPMQTT_h
#define DCPMQTT_h
#include "../DCPSystem.h"
#define TINY_GSM_MODEM_SIM800 //Tipo de modem que estamos usando
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include "../SDCard/DCPSDCard.h"
#include "../SIM800/DCPSIM800.h"

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

private:
    PubSubClient* clientPub;
    int TIME_TO_SEND = (10);
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
    int nextSlotToSend;
    boolean onTimeToSend();
    void nextSlotTimeToSend();

};

#endif