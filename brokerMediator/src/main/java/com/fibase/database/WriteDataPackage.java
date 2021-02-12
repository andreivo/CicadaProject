/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.fibase.database;

import com.fibase.Constants;
import com.fibase.datapackage.DataPackage;
import com.fibase.datapackage.MeasuresDtp;
import com.fibase.datapackage.MetadataDtp;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.time.format.DateTimeFormatter;

/**
 *
 * @author andre
 */
public class WriteDataPackage {

    public static void csv(DataPackage dp) {

        BufferedWriter buffWrite = null;
        try {
            String fileName = dp.getTokenStation() + ".csv";
            FileWriter fileWriter = null;
            buffWrite = null;

            if (dp.getMeasures() != null) {
                fileWriter = new FileWriter(new File(Constants.DATAPATH + fileName), true);
                buffWrite = new BufferedWriter(fileWriter);
                for (MeasuresDtp measure : dp.getMeasures()) {
                    String data = formatData(dp, measure);
                    buffWrite.append(data + "\n");
                }
                buffWrite.close();
            }

            if (dp.getMetadata() != null) {
                fileName = dp.getTokenStation() + "_mtd.csv";
                fileWriter = new FileWriter(new File(Constants.DATAPATH + fileName), true);
                buffWrite = new BufferedWriter(fileWriter);
                for (MetadataDtp metadata : dp.getMetadata()) {
                    String data = formatData(dp, metadata);
                    buffWrite.append(data + "\n");
                }
                buffWrite.close();
            }

        } catch (IOException ex) {
            System.out.println("Write CSV Error: " + ex.getMessage());
        }
    }

    private static String formatData(DataPackage dp, MeasuresDtp measure) {
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
        String result = dp.getTokenStation() + "; "
                + dp.getSentDateTime().format(formatter) + "; "
                + measure.getDataType() + "; "
                + measure.getSensorExternalCode().toString() + "; "
                + measure.getCollectDateTime().format(formatter) + "; "
                + measure.getDataValue();
        return result;
    }

    private static String formatData(DataPackage dp, MetadataDtp metadata) {
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
        String result = dp.getTokenStation() + "; "
                + dp.getSentDateTime().format(formatter) + "; "
                + metadata.getDataType() + "; "
                + metadata.getCollectDateTime().format(formatter) + "; "
                + metadata.getDataValue() + "; "
                + metadata.getContext();
        return result;
    }

}
