/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.fibase.mqtt;

import static com.fibase.QueueIdentity.MQTTBROKERURL;
import static com.fibase.QueueIdentity.MQTTTOKEN;
import static com.fibase.QueueIdentity.MQTTTOPIC;
import com.fibase.datapackage.DataPackage;
import com.fibase.datapackage.MeasuresDtp;
import com.fibase.datapackage.MetadataDtp;
import java.io.UnsupportedEncodingException;
import java.time.format.DateTimeFormatter;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

/**
 *
 * @author andre
 */
public class MQTTProcessorThread {

    private MqttClient client;
    private MqttConnectOptions conOpt;

    public MQTTProcessorThread() throws MqttException, UnsupportedEncodingException {
        conOpt = new MqttConnectOptions();
        conOpt.setCleanSession(true);
        conOpt.setUserName(MQTTTOKEN);

        client = new MqttClient(MQTTBROKERURL, MqttClient.generateClientId());
    }

    public void publish(DataPackage dp) throws MqttException, UnsupportedEncodingException, InterruptedException, ExecutionException {
        ExecutorService executor = Executors.newSingleThreadExecutor();
        TaskMQTT taskMQTT = new TaskMQTT();
        taskMQTT.setDataPackage(dp, client, conOpt);
        Future<String> future = executor.submit(taskMQTT);

        try {
            System.out.println("Started Publishing THREAD..");
            System.out.println(future.get(20, TimeUnit.SECONDS));
            System.out.println("Finished Publishing THREAD!");
        } catch (TimeoutException e) {
            future.cancel(true);
            System.out.println("Timeout Publishing THREAD!");
        }

        taskMQTT = null;
        dp = null;
        executor.shutdownNow();
        executor = null;
    }

    public void free() {
        try {
            super.finalize();
        } catch (Throwable ex) {
        }
    }
}

class TaskMQTT implements Callable<String> {

    private DataPackage dp;
    private MqttClient client;
    private MqttConnectOptions conOpt;

    public void setDataPackage(DataPackage dp, MqttClient client, MqttConnectOptions conOpt) {
        this.dp = dp;
        this.client = client;
        this.conOpt = conOpt;
    }

    @Override
    public String call() throws Exception {
        if (dp != null) {
            publish(dp);
        }

        return "Ready!";
    }

    public void publish(DataPackage dp) throws MqttException, UnsupportedEncodingException {

        System.out.println("Publishing data to MQTTServer....");

        // Data setup
        String payload = formatPayload(dp);
        int qos = 0;

        if (!client.isConnected()) {
            System.out.println("Connecting to MQTT broker ...");
            client.connect(conOpt);
        }

        if (client.isConnected()) {
            System.out.println("Send MQTT message of Station: " + dp.getTokenStation().toLowerCase());

            // Sends Data
            MqttMessage message = new MqttMessage(payload.getBytes("UTF-8"));
            message.setQos(qos);

            client.publish(MQTTTOPIC + dp.getTokenStation().toLowerCase(), message);
            client.disconnect();
            System.out.println("Finish send message of Station: " + dp.getTokenStation().toLowerCase());
        }

    }

    private String formatPayload(DataPackage dp) {
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
