/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.fibase.mediator;

import com.fibase.amqp.DataProcessor;
import com.fibase.datapackage.MeasuresDtp;
import com.fibase.datapackage.MetadataDtp;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import java.lang.reflect.Field;

/**
 *
 * @author andre
 */
public class DataPackageNormalizer {

    private final Ontology ontology;

    public DataPackageNormalizer(String ontologyPath) {
        this.ontology = new Ontology(ontologyPath);
    }

    public String normalizeByOntology(String original, Class clazz) throws Exception {
        JsonObject jsonObj = new JsonParser().parse(original).getAsJsonObject();

        if (jsonObj.entrySet().size() > clazz.getDeclaredFields().length) { // if the json-string has more entries than class has fields, we're already out
            throw new Exception("Invalid Datapackage!");
        }

        String result = original;

        for (String keyField : jsonObj.keySet()) {
            Field declaredField = null;
            try {
                declaredField = clazz.getDeclaredField(keyField);
            } catch (Exception e) {
            }

            if (declaredField == null) {
                String equivalentKey = ontology.getEquivalent(keyField);
                if (!equivalentKey.equals("")) {
                    result = result.replace(keyField, equivalentKey);
                }
            }

            if ("measures".equals(keyField)) {
                JsonElement get = jsonObj.get("measures");
                JsonArray asJsonArray = get.getAsJsonArray();

                if (asJsonArray != null) {
                    int plen = asJsonArray.size();
                    for (int i = 0; i < plen; i++) {
                        String normalizeByOntology = normalizeByOntology(asJsonArray.get(i).toString(), MeasuresDtp.class);
                        result = result.replace(asJsonArray.get(i).toString(), normalizeByOntology);
                    }
                }
            }

            if ("metadata".equals(keyField)) {
                JsonElement get = jsonObj.get("metadata");
                JsonArray asJsonArray = get.getAsJsonArray();

                if (asJsonArray != null) {
                    int plen = asJsonArray.size();
                    for (int i = 0; i < plen; i++) {
                        String normalizeByOntology = normalizeByOntology(asJsonArray.get(i).toString(), MetadataDtp.class);
                        result = result.replace(asJsonArray.get(i).toString(), normalizeByOntology);
                    }
                }
            }
        }
        return result;

    }

    public void free() {
        try {
            super.finalize();
        } catch (Throwable ex) {
        }
    }

//        /**
//     * @param args the command line arguments
//     */
    public static void main(String[] args) throws Exception {
        // TODO code application logic here
        String data = "{\"stationCode\":\"3335685A\", \n"
                + " \"serverPasswd\":\"PASSWD-DCP\", \n"
                + " \"transmissionDateTime\":\"2020-12-09 18:01:56Z\", \n"
                + " \"measures\":[{\"snsEC\":20,\"dtT\":\"pluv\",\"colDT\":\"2020-12-09 18:01:56Z\",\"val\":\"8\"}, \n"
                + "               {\"snsEC\":21,\"dtT\":\"vccin\",\"colDT\":\"2020-12-09 18:01:56Z\",\"val\":\"12.58\"}, \n"
                + "               {\"snsEC\":21,\"dtT\":\"temp\",\"colDT\":\"2020-12-09 18:01:56Z\",\"val\":\"24\"}, \n"
                + "               {\"snsEC\":21,\"dtT\":\"hum\",\"colDT\":\"2020-12-09 18:01:56Z\",\"val\":\"70\"}]}";

        System.out.println(data);
        DataProcessor dp = new DataProcessor();
        dp.process(data);

        data = "{\"stationCode\":\"3335685A\", \n"
                + " \"serverPasswd\":\"PASSWD-DCP\", \n"
                + " \"transmissionDateTime\":\"2020-12-09 18:01:56Z\", \n"
                + " \"metadata\":[{\"dtT\":\"la\",\"colDT\":\"2020-12-09 18:01:56Z\",\"val\":\"-23.7\"}, \n"
                + "              {\"dtT\":\"lo\",\"colDT\":\"2020-12-09 18:01:56Z\",\"val\":\"-44.7\"}, \n"
                + "              {\"dtT\":\"bkt\",\"colDT\":\"2020-12-09 18:01:56Z\",\"val\":\"3.14\"}]}";

        System.out.println(data);
        dp = new DataProcessor();
        dp.process(data);

        data = "{\"stationCode\":\"3335685A\", \n"
                + " \"serverPasswd\":\"PASSWD-DCP\", \n"
                + " \"transmissionDateTime\":\"2020-12-09 18:01:56Z\", \n"
                + " \"metadata\":[{\"dtT\":\"csq\",\"colDT\":\"2020-12-09 18:01:56Z\",\"val\":\"14\",\"context\":\"{'cty':'SIM','car':'Claro','icc':'89550532180055763051','lip':'100.70.166.103'}\"}]}";

        System.out.println(data);
        dp = new DataProcessor();
        dp.process(data);

//        DataPackageNormalizer dpn = new DataPackageNormalizer(path);
//        String normalizeDP = dpn.normalizeByOntology(data, DataPackage.class);
//        System.out.println(normalizeDP);
//        DataPackage dp = DataPackage.fromJson(normalizeDP);
//        System.out.println(dp.toJson());
//
//        Ontology ont = new Ontology("/dados/projetos/projetos.pessoais/bitbucket/doutorado-2/Tese/06_Artigo/Estudo-de-Caso/github/FIBASE-Case-Study/ontologies/OR_OBSNetwork_NewArchitecture.rdf");
//        ont.printAllEquivalents();
    }
}
