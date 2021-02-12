package com.fibase.datapackage;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.TypeAdapter;
import com.google.gson.stream.JsonReader;
import com.google.gson.stream.JsonToken;
import com.google.gson.stream.JsonWriter;
import java.io.IOException;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;

public class DataPackage {

    private String tokenStation;
    private String passwdStation;
    private LocalDateTime sentDateTime;
    private List<MeasuresDtp> measures;
    private List<MetadataDtp> metadata;

    public DataPackage() {
        this.measures = new ArrayList<>();
    }

    public String getTokenStation() {
        return tokenStation;
    }

    public void setTokenStation(String tokenStation) {
        this.tokenStation = tokenStation;
    }

    public String getPasswdStation() {
        return passwdStation;
    }

    public void setPasswdStation(String passwdStation) {
        this.passwdStation = passwdStation;
    }

    public LocalDateTime getSentDateTime() {
        return sentDateTime;
    }

    public void setSentDateTime(LocalDateTime sentDateTime) {
        this.sentDateTime = sentDateTime;
    }

    public List<MeasuresDtp> getMeasures() {
        return measures;
    }

    public void setMeasures(List<MeasuresDtp> measures) {
        this.measures = measures;
    }

    public void addMeasures(LocalDateTime dateTime, String value, Long sensorExternalCode, String dateType) {
        this.measures.add(new MeasuresDtp(dateTime, value, sensorExternalCode, dateType));
    }

    public List<MetadataDtp> getMetadata() {
        return metadata;
    }

    public void setMetadata(List<MetadataDtp> metadata) {
        this.metadata = metadata;
    }

    public String toJson() {
        GsonBuilder builder = new GsonBuilder();
        builder.registerTypeAdapter(LocalDateTime.class, new LocalDateTimeAdapter());
        Gson gson = builder.create();
        return gson.toJson(this);
    }

    public static DataPackage fromJson(String json) {
        GsonBuilder builder = new GsonBuilder();
        builder.registerTypeAdapter(LocalDateTime.class, new LocalDateTimeAdapter());
        Gson gson = builder.create();
        return gson.fromJson(json, DataPackage.class);

    }

    public static class LocalDateTimeAdapter extends TypeAdapter<LocalDateTime> {

        @Override
        public void write(JsonWriter writer, LocalDateTime value) throws IOException {
            if (value == null) {
                writer.nullValue();
                return;
            }
            DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss'Z'");
            writer.value(formatter.format(value));
        }

        @Override
        public LocalDateTime read(JsonReader reader) throws IOException {
            if (reader.peek() == JsonToken.NULL) {
                reader.nextNull();
                return null;
            }
            String str = reader.nextString();
            DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss'Z'");
            LocalDateTime dateTime = LocalDateTime.parse(str, formatter);
            return dateTime;
        }
    }

}
