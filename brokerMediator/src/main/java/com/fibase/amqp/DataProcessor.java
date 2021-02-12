/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.fibase.amqp;

import com.fibase.Constants;
import com.fibase.database.WriteDataPackage;
import com.fibase.datapackage.DataPackage;
import com.fibase.mediator.DataPackageNormalizer;
import com.fibase.mqtt.MQTTProcessorThread;
import java.io.UnsupportedEncodingException;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.eclipse.paho.client.mqttv3.MqttException;

/**
 *
 * @author andre
 */
public class DataProcessor {

    private DataPackageNormalizer dpn;
    private MQTTProcessorThread mqttpt;

    public DataProcessor() throws MqttException, UnsupportedEncodingException {
        this.dpn = new DataPackageNormalizer(Constants.ONTOLOGYPATH);
        this.mqttpt = new MQTTProcessorThread();
    }

    public void process(String message) {
        try {
            String normalizeDP = dpn.normalizeByOntology(message, DataPackage.class);
            DataPackage dp = DataPackage.fromJson(normalizeDP);
            if (dp != null) {
                if (dp.getTokenStation() != null) {
                    WriteDataPackage.csv(dp);
                    mqttpt.publish(dp);
                }
            }
        } catch (Exception ex) {
            System.out.println("Error normalize DataPackage! Erro: " + ex.getMessage());
            this.mqttpt.free();
            try {
                this.mqttpt = new MQTTProcessorThread();
            } catch (Exception ex1) {
                Logger.getLogger(DataProcessor.class.getName()).log(Level.SEVERE, null, ex1);
            }

        }
    }

}
