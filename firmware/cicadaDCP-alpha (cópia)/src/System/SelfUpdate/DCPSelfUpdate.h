/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

#ifndef DCPSelfUpdate_h
#define DCPSelfUpdate_h
#include "../DCPSystem.h"
#include <Update.h>

class DCPSelfUpdate {
public:
    DCPSelfUpdate();
    boolean setupSelfUpdate(int timeToCheck, String _host, String _hostpath, int _port, String _firmwareDate, String _stationName, xTaskHandle _coreTask);
    boolean updateFirmware(boolean force = false);
private:
    int TIME_TO_CHECK = (10); // hour
    String host;
    String hostpath;
    int port;
    time_t firmwareDate;
    String stationName;
    xTaskHandle coreTask;
    time_t nexTimeToCheck;
    boolean onTimeToCheck();
    void nextTimeToCheck();
    boolean startUpdate(String filename);
    boolean downloadFile(Client* client, String host, String hostPath, int port, String filename, String saveAs);
    String getHeaderValue(String header, String headerName);
    void printPercent(long readLength, long contentLength, uint32_t timeElapsed);

};

#endif