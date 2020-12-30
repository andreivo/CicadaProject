/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 1.0.0
 *    AUTHORS:
 *             André Ivo <andre.ivo@gmail.com.br>
 *
 *       SITE: https://github.com/andreivo/cicada
 */

#ifndef DCPVoltage_h
#define DCPVoltage_h

#include "../system/DCPSystem.h"

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
    void setupVccSolSensor();
    void initVccSensor(String _codeVccIn, String _typeVccIn, int timeSlotVccIn, String _codeVccSol, String _typeVccSol, int timeSlotVccSol);
    void readVccIn();
    void readVccSol();
private:

    int nextSlotTimeToRead(int TIME_TO_READ);
    String codeVccIn;
    String typeVccIn;
    int TIME_TO_READ_VCCIN = (10);
    void nextVccInSlotTimeToRead();
    int nextSlotVccIn;
    boolean timeToReadVccIn();
    String codeVccSol;
    String typeVccSol;
    int TIME_TO_READ_VCCSOL = (10);
    void nextVccSolSlotTimeToRead();
    int nextSlotVccSol;
    boolean timeToReadVccSol();
    void initVccInSensor(String _codeVccIn, String _typeVccIn, int timeSlotVccIn);
    void initVccSolarCellSensor(String _codeVccSol, String _typeVccSol, int timeSlotVccSol);
};

#endif