/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

#ifndef DCPVoltage_h
#define DCPVoltage_h

#include "../../System/DCPSystem.h"

#define R1 33.0
#define R2 10.0
#define RESISTOR_DIVIDER (R2/(R1+R2))
#define MAX_VOLTAGE_ADC 3.33
#define MAX_VOLTAGE_READ (MAX_VOLTAGE_ADC / RESISTOR_DIVIDER)
#define MAX_ADC_VALUE 4096
#define ADC2VOLTS(adc) ((adc * MAX_VOLTAGE_READ)/MAX_ADC_VALUE)

class DCPVoltage {
public:
    DCPVoltage();
    void setupVccInSensor();
    void setupVccBatSensor();
    void initVccSensor(String _codeVccIn, String _typeVccIn, int timeSlotVccIn, String _codeVccBat, String _typeVccBat, int timeSlotVccBat);
    void readVccIn();
    void readVccBat();
    String printVccIn();
    String printVccBat();
    void updateNextSlotIn();
    void updateNextSlotBat();
    void printNextVccInSlot();
    void printNextVccBatSlot();
private:
    int nextSlotTimeToRead(int TIME_TO_READ);
    String codeVccIn;
    String typeVccIn;
    int TIME_TO_READ_VCCIN = (10);
    void nextVccInSlotTimeToRead();
    int nextSlotVccIn;
    boolean timeToReadVccIn();
    String codeVccBat;
    String typeVccBat;
    int TIME_TO_READ_VCCSOL = (10);
    void nextVccBatSlotTimeToRead();
    int nextSlotVccBat;
    boolean timeToReadVccBat();
    void initVccInSensor(String _codeVccIn, String _typeVccIn, int timeSlotVccIn);
    void initVccBatarCellSensor(String _codeVccBat, String _typeVccBat, int timeSlotVccBat);
};

#endif