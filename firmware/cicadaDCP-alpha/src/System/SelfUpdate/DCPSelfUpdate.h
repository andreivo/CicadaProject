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

#define SU_ATTEMPTS 3
#define SU_ATTEMPTS_DELAY 500
#define DOWNLOAD_TIMEOUT 15000

class DCPSelfUpdate {
public:
    DCPSelfUpdate();
    boolean setupSelfUpdate(int timeToCheck, String _host, String _hostpath, int _port, String _firmwareDate, String _stationName);
    boolean updateFirmware(boolean force = false);
    boolean startUpdate(String filename);
private:
    int TIME_TO_CHECK = (10); // hour
    String host;
    String hostpath;
    int port;
    time_t firmwareDate;
    String stationName;
    time_t nexTimeToCheck;
    boolean onTimeToCheck();
    void nextTimeToCheck();
    boolean downloadFile(Client* client, String host, String hostPath, int port, String filename, String saveAs);
    String getHeaderValue(String header, String headerName);
    void printPercent(long readLength, long contentLength, uint32_t timeElapsed);
    boolean prepareUpdate(String subPath);

};

#endif