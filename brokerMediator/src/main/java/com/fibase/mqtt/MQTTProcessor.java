/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.fibase.mqtt;

import com.fibase.QueueIdentity;
import com.fibase.datapackage.DataPackage;
import com.fibase.datapackage.MeasuresDtp;
import com.fibase.datapackage.MetadataDtp;
import java.io.UnsupportedEncodingException;
import java.time.format.DateTimeFormatter;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

/**
 *
 * @author andre
 */
public class MQTTProcessor {

    public static void publish(DataPackage dp) throws MqttException, UnsupportedEncodingException {

        System.out.println("Publishing data to MQTTServer....");

        // Data setup
        String payload = formatPayload(dp);
        int qos = 0;

        // Creates the MQTT client
        MqttClient client;
        MqttConnectOptions conOpt;

        conOpt = new MqttConnectOptions();
        conOpt.setCleanSession(true);
        conOpt.setUserName(QueueIdentity.MQTTTOKEN);

        client = new MqttClient(QueueIdentity.MQTTBROKERURL, MqttClient.generateClientId());

        if (!client.isConnected()) {
            System.out.println("Connecting to MQTT broker ...");
            client.connect(conOpt);
        }

        if (client.isConnected()) {
            System.out.println("Send MQTT message of Station: " + dp.getTokenStation().toLowerCase());

            // Sends Data
            MqttMessage message = new MqttMessage(payload.getBytes("UTF-8"));
            message.setQos(qos);

            client.publish(QueueIdentity.MQTTTOPIC + dp.getTokenStation().toLowerCase(), message);
            client.disconnect();
            System.out.println("Finish send message of Station: " + dp.getTokenStation().toLowerCase());
        }
    }

    private static String formatPayload(DataPackage dp) {
        String result = "";

        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss'Z'");

        if (dp.getMeasures() != null) {
            for (MeasuresDtp measure : dp.getMeasures()) {
                if (result != "") {
                    result = result + ",";
                }
                result = result + "\"" + measure.getDataType() + "\":";
                result = result + "{\"value\":" + measure.getDataValue();
                result = result + ",\"context\":{\"collect\":\"" + measure.getCollectDateTime().format(formatter) + "\"}}";
            }
        }

        if (dp.getMetadata() != null) {
            for (MetadataDtp metadata : dp.getMetadata()) {
                if (result != "") {
                    result = result + ",";
                }
                result = result + "\"" + metadata.getDataType() + "\":";
                result = result + "{\"value\":" + metadata.getDataValue();
                result = result + ",\"context\":{\"collect\":\"" + metadata.getCollectDateTime().format(formatter) + "\"";

                if (metadata.getContext() != null) {
                    result = result + ",\"context\":\"" + metadata.getContext() + "\"";
                }

                result = result + "}}";
            }
        }
        System.out.println("{" + result + "}");
        return "{" + result + "}";
    }

}
