/**************************************************************
   CicadaWizard is a library for the ESP32/Arduino platform
   to enable easy configuration and reconfiguration of Cicada DCP using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer/examples/CaptivePortalAdvanced
   https://github.com/tzapu

   Built to CICADA DCP Firmware for the ESP32

     VERSION: 0.0.1-alpha
     DATE   : 2021-01
     AUTHORS: André Ivo <andre.ivo@gmail.com.br>
     LICENSE: CC-BY-4.0
        SITE: https://github.com/andreivo/CicadaProject

 **************************************************************/

#include "CicadaWizard.h"

/**
 * File system directories and variables
 */
SPIFFSManager spiffsMan;

String fmwver = "";
String stationID = "";
String macAddr = "";

CicadaWizardParameter::CicadaWizardParameter(const char *custom) {
    _id = NULL;
    _placeholder = NULL;
    _length = 0;
    _value = NULL;

    _customHTML = custom;
}

CicadaWizardParameter::CicadaWizardParameter(const char *id, const char *placeholder, const char *defaultValue, int length) {
    init(id, placeholder, defaultValue, length, "");
}

CicadaWizardParameter::CicadaWizardParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom) {
    init(id, placeholder, defaultValue, length, custom);
}

void CicadaWizardParameter::init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom) {
    _id = id;
    _placeholder = placeholder;
    _length = length;
    _value = new char[length + 1];
    for (int i = 0; i < length; i++) {
        _value[i] = 0;
    }
    if (defaultValue != NULL) {
        strncpy(_value, defaultValue, length);
    }

    _customHTML = custom;
}

const char* CicadaWizardParameter::getValue() {
    return _value;
}

const char* CicadaWizardParameter::getID() {
    return _id;
}

const char* CicadaWizardParameter::getPlaceholder() {
    return _placeholder;
}

int CicadaWizardParameter::getValueLength() {
    return _length;
}

const char* CicadaWizardParameter::getCustomHTML() {
    return _customHTML;
}

CicadaWizard::CicadaWizard() {
}

void CicadaWizard::addParameter(CicadaWizardParameter *p) {
    if (_paramsCount + 1 > WIFI_MANAGER_MAX_PARAMS) {
        //Max parameters exceeded!
        DEBUG_WM("WIFI_MANAGER_MAX_PARAMS exceeded, increase number (in CicadaWizard.h) before adding more parameters!");
        DEBUG_WM("Skipping parameter with ID:");
        DEBUG_WM(p->getID());
        return;
    }
    _params[_paramsCount] = p;
    _paramsCount++;
    DEBUG_WM("Adding parameter");
    DEBUG_WM(p->getID());
}

void CicadaWizard::setupConfigPortal() {
    dnsServer.reset(new DNSServer());
#ifdef ESP8266
    server.reset(new ESP8266WebServer(80));
#else
    server.reset(new WebServer(80));
#endif

    DEBUG_WM(F(""));
    _configPortalStart = millis();

    DEBUG_WM(F("Configuring access point... "));
    DEBUG_WM(_apName);
    if (_apPassword != NULL) {
        if (strlen(_apPassword) < 8 || strlen(_apPassword) > 63) {
            // fail passphrase to short or long!
            DEBUG_WM(F("Invalid AccessPoint password. Ignoring"));
            _apPassword = NULL;
        }
        DEBUG_WM(_apPassword);
    }

    //optional soft ip config
    if (_ap_static_ip) {
        DEBUG_WM(F("Custom AP IP/GW/Subnet"));
        WiFi.softAPConfig(_ap_static_ip, _ap_static_gw, _ap_static_sn);
    }

    if (_apPassword != NULL) {
        WiFi.softAP(_apName, _apPassword); //password option
    } else {
        WiFi.softAP(_apName);
    }

    delay(500); // Without delay I've seen the IP address blank
    DEBUG_WM(F("AP IP address: "));
    DEBUG_WM(WiFi.softAPIP());

    /* Setup the DNS server redirecting all the domains to the apIP */
    dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());

    /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
    server->on("/", std::bind(&CicadaWizard::handleRoot, this));
    server->on("/wifi", std::bind(&CicadaWizard::handleWifi, this, true));
    server->on("/0wifi", std::bind(&CicadaWizard::handleWifi, this, false));
    server->on("/wifisave", std::bind(&CicadaWizard::handleWifiSave, this));
    server->on("/info", std::bind(&CicadaWizard::handleInfo, this));
    server->on("/reboot", std::bind(&CicadaWizard::handleReboot, this));
    //server->on("/generate_204", std::bind(&CicadaWizard::handle204, this));  //Android/Chrome OS captive portal check.
    server->on("/fwlink", std::bind(&CicadaWizard::handleRoot, this)); //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    server->onNotFound(std::bind(&CicadaWizard::handleNotFound, this));
    server->begin(); // Web server start
    DEBUG_WM(F("HTTP server started"));

    // CICADA
    server->on("/saveCicadaStation", std::bind(&CicadaWizard::handleSaveCicadaStation, this));
    server->on("/sensors", std::bind(&CicadaWizard::handleSensors, this));
    server->on("/saveSensorsConfig", std::bind(&CicadaWizard::handleSaveSensorsConfig, this));
    server->on("/saveCicadaMQTT", std::bind(&CicadaWizard::handleSaveCicadaMQTT, this));
    server->on("/saveCicadaSIM", std::bind(&CicadaWizard::handleSaveCicadaSIMCard, this));
    server->on("/delwifi", std::bind(&CicadaWizard::handleDelWifi, this));
    server->on("/factoryreset", std::bind(&CicadaWizard::handleConfirmFactoryReset, this));
    server->on("/factoryresetYES", std::bind(&CicadaWizard::handleFactoryReset, this));

    // Get the Station ID
    stationID = spiffsMan.getSettings("Station ID", DIR_STATION_ID, false);
    // Get Firmware
    fmwver = spiffsMan.getSettings("Firmware", DIR_FIRMWARE_VERSION, false);
    // Get Station MAC Address
    macAddr = WiFi.softAPmacAddress();
}

boolean CicadaWizard::autoConnect() {
    String ssid = "ESP" + String(ESP_getChipId());
    return autoConnect(ssid.c_str(), NULL);
}

boolean CicadaWizard::autoConnect(char const *apName, char const *apPassword) {
    DEBUG_WM(F(""));
    DEBUG_WM(F("AutoConnect"));

    // read eeprom for ssid and pass
    //String ssid = getSSID();
    //String pass = getPassword();

    // attempt to connect; should it fail, fall back to AP
    WiFi.mode(WIFI_STA);

    if (connectWifi("", "") == WL_CONNECTED) {
        DEBUG_WM(F("IP Address:"));
        DEBUG_WM(WiFi.localIP());
        //connected
        return true;
    }

    return startConfigPortal(apName, apPassword);
}

boolean CicadaWizard::configPortalHasTimeout() {
#if defined(ESP8266)
    if (_configPortalTimeout == 0 || wifi_softap_get_station_num() > 0) {
#else
    if (_configPortalTimeout == 0) { // TODO
#endif
        _configPortalStart = millis(); // kludge, bump configportal start time to skew timeouts
        return false;
    }
    return (millis() > _configPortalStart + _configPortalTimeout);
}

boolean CicadaWizard::startConfigPortal() {
    String ssid = "ESP" + String(ESP_getChipId());
    return startConfigPortal(ssid.c_str(), NULL);
}

boolean CicadaWizard::startConfigPortal(char const *apName, char const *apPassword) {
    //setup AP
    WiFi.mode(WIFI_AP_STA);
    DEBUG_WM("SET AP STA");

    _apName = apName;
    _apPassword = apPassword;

    //notify we entered AP mode
    if (_apcallback != NULL) {
        _apcallback(this);
    }

    connect = false;
    setupConfigPortal();

    while (1) {

        // check if timeout
        if (configPortalHasTimeout()) break;

        //DNS
        dnsServer->processNextRequest();
        //HTTP
        server->handleClient();


        if (connect) {
            connect = false;
            delay(2000);
            DEBUG_WM(F("Connecting to new AP"));

            // using user-provided  _ssid, _pass in place of system-stored ssid and pass
            if (connectWifi(_ssid, _pass) != WL_CONNECTED) {
                DEBUG_WM(F("Failed to connect."));
            } else {
                //connected
                WiFi.mode(WIFI_STA);
                //notify that configuration has changed and any optional parameters should be saved
                if (_savecallback != NULL) {
                    //todo: check if any custom parameters actually exist, and check if they really changed maybe
                    _savecallback();
                }
                break;
            }

            if (_shouldBreakAfterConfig) {
                //flag set to exit after config after trying to connect
                //notify that configuration has changed and any optional parameters should be saved
                if (_savecallback != NULL) {
                    //todo: check if any custom parameters actually exist, and check if they really changed maybe
                    _savecallback();
                }
                break;
            }
        }
        yield();
    }

    server.reset();
    dnsServer.reset();

    return WiFi.status() == WL_CONNECTED;
}

int CicadaWizard::connectWifi(String ssid, String pass) {
    DEBUG_WM(F("Connecting as wifi client..."));

    // check if we've got static_ip settings, if we do, use those.
    if (_sta_static_ip) {
        DEBUG_WM(F("Custom STA IP/GW/Subnet"));
        WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn);
        DEBUG_WM(WiFi.localIP());
    }
    //fix for auto connect racing issue
    if (WiFi.status() == WL_CONNECTED) {
        DEBUG_WM("Already connected. Bailing out.");
        return WL_CONNECTED;
    }
    //check if we have ssid and pass and force those, if not, try with last saved values
    if (ssid != "") {
        WiFi.begin(ssid.c_str(), pass.c_str());
    } else {
        if (WiFi.SSID()) {
            DEBUG_WM("Using last saved values, should be faster");
#if defined(ESP8266)
            //trying to fix connection in progress hanging
            ETS_UART_INTR_DISABLE();
            wifi_station_disconnect();
            ETS_UART_INTR_ENABLE();
#else
            esp_wifi_disconnect();
#endif

            WiFi.begin();
        } else {
            DEBUG_WM("No saved credentials");
        }
    }

    int connRes = waitForConnectResult();
    DEBUG_WM("Connection result: ");
    DEBUG_WM(connRes);
    //not connected, WPS enabled, no pass - first attempt
    if (_tryWPS && connRes != WL_CONNECTED && pass == "") {
        startWPS();
        //should be connected at the end of WPS
        connRes = waitForConnectResult();
    }
    return connRes;
}

uint8_t CicadaWizard::waitForConnectResult() {
    if (_connectTimeout == 0) {
        return WiFi.waitForConnectResult();
    } else {
        DEBUG_WM(F("Waiting for connection result with time out"));
        unsigned long start = millis();
        boolean keepConnecting = true;
        uint8_t status;
        while (keepConnecting) {
            status = WiFi.status();
            if (millis() > start + _connectTimeout) {
                keepConnecting = false;
                DEBUG_WM(F("Connection timed out"));
            }
            if (status == WL_CONNECTED || status == WL_CONNECT_FAILED) {
                keepConnecting = false;
            }
            delay(100);
        }
        return status;
    }
}

void CicadaWizard::startWPS() {
#if defined(ESP8266)
    DEBUG_WM("START WPS");
    WiFi.beginWPSConfig();
    DEBUG_WM("END WPS");
#else
    // TODO
    DEBUG_WM("ESP32 WPS TODO");
#endif
}

String CicadaWizard::getSSID() {
    if (_ssid == "") {
        DEBUG_WM(F("Reading SSID"));
        _ssid = WiFi.SSID();
        DEBUG_WM(F("SSID: "));
        DEBUG_WM(_ssid);
    }
    return _ssid;
}

String CicadaWizard::getPassword() {
    if (_pass == "") {
        DEBUG_WM(F("Reading Password"));
        _pass = WiFi.psk();
        DEBUG_WM("Password: " + _pass);
        //DEBUG_WM(_pass);
    }
    return _pass;
}

String CicadaWizard::getConfigPortalSSID() {
    return _apName;
}

void CicadaWizard::deleteWifiCredentials() {
    DEBUG_WM(F("settings invalidated"));
    DEBUG_WM(F("THIS MAY CAUSE AP NOT TO START UP PROPERLY. YOU NEED TO COMMENT IT OUT AFTER ERASING THE DATA."));
    // TODO On ESP32 this does not erase the SSID and password. See
    // https://github.com/espressif/arduino-esp32/issues/400
    // For now, use "make erase_flash".
    //WiFi.disconnect(true);
    //delay(200);

    spiffsMan.FSDeleteFiles(DIR_WIFI_SSID);
    spiffsMan.FSDeleteFiles(DIR_WIFI_PWD);

    //Hack André ivo
    WiFi.disconnect(true); // still not erasing the ssid/pw. Will happily reconnect on next start
    WiFi.begin("0", "0"); // adding this effectively seems to erase the previous stored SSID/PW
    delay(1000);
    DEBUG_WM(F("Hack to deleteWifiCredentials (André Ivo)"));
    ////////////////

}

void CicadaWizard::setTimeout(unsigned long seconds) {
    setConfigPortalTimeout(seconds);
}

void CicadaWizard::setConfigPortalTimeout(unsigned long seconds) {
    _configPortalTimeout = seconds * 1000;
}

void CicadaWizard::setConnectTimeout(unsigned long seconds) {
    _connectTimeout = seconds * 1000;
}

void CicadaWizard::setDebugOutput(boolean debug) {
    _debug = debug;
}

void CicadaWizard::setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn) {
    _ap_static_ip = ip;
    _ap_static_gw = gw;
    _ap_static_sn = sn;
}

void CicadaWizard::setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn) {
    _sta_static_ip = ip;
    _sta_static_gw = gw;
    _sta_static_sn = sn;
}

void CicadaWizard::setMinimumSignalQuality(int quality) {
    _minimumQuality = quality;
}

void CicadaWizard::setBreakAfterConfig(boolean shouldBreak) {
    _shouldBreakAfterConfig = shouldBreak;
}

/*******************************************************************************
 * CICADA CODES - BEGIN
 *******************************************************************************/
boolean CicadaWizard::startWizardPortal(char const *apName, char const *apPassword) {
    //setup AP
    WiFi.mode(WIFI_AP_STA);
    DEBUG_WM("SET AP STA");

    _apName = apName;
    _apPassword = apPassword;

    //notify we entered AP mode
    if (_apcallback != NULL) {
        _apcallback(this);
    }

    connect = false;
    setupConfigPortal();

    while (1) {

        // check if timeout
        if (configPortalHasTimeout()) break;

        //DNS
        dnsServer->processNextRequest();
        //HTTP
        server->handleClient();

        yield();
    }

    server.reset();
    dnsServer.reset();

    return WiFi.status() == WL_CONNECTED;
}

/** Handle root or redirect to captive portal */
void CicadaWizard::handleRoot() {
    DEBUG_WM(F("Handle root"));
    if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
        return;
    }

    String page = FPSTR(HTTP_HEAD_HTML);
    page.replace("{v}", "Cicada DCP Wizard");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += _customHeadElement;
    page += FPSTR(HTTP_HEAD_END);

    // Get the previous bucket configuration
    String vol = spiffsMan.getSettings("Bucket calibration", DIR_STATION_BUCKET_VOL, false);

    // Get the previous Station Name
    String name = spiffsMan.getSettings("Station Name", DIR_STATION_NAME, false);

    // Get the previous Station Name
    String pass = spiffsMan.getSettings("Station password", DIR_STATION_PASS, false);

    // Get the previous Latitude
    String lat = spiffsMan.getSettings("Latitude", DIR_STATION_LATITUDE, false);

    // Get the previous Longitude
    String lon = spiffsMan.getSettings("Longitude", DIR_STATION_LONGITUDE, false);

    // Get the previous Send Time Interval
    String sti = spiffsMan.getSettings("Send Time Interval", DIR_STATION_SENDTIMEINTERVAL, false);

    // Get the previous Store metadata interval
    String smi = spiffsMan.getSettings("Store metadata interval", DIR_STATION_STOREMETADATA, false);

    // Setup the form
    String form = FPSTR(HTTP_FORM_CONFIG_STATION);
    form.replace("{vol}", vol);
    form.replace("{name}", name);
    form.replace("{spass}", pass);
    form.replace("{lat}", lat);
    form.replace("{lon}", lon);
    form.replace("{sti}", sti);
    form.replace("{smi}", smi);

    form.replace("{sttid}", stationID);
    form.replace("{cicadalogo}", HTTP_CICADALOGO);

    page += form;

    // Setup the script
    String script = FPSTR(HTTP_SCRIPT_FORM_CONFIG_STATION);
    script.replace("{RESET_TIME}", RESET_TIME);
    page += script;

    String footer = FPSTR(HTTP_END);
    footer.replace("{fmwver}", fmwver);
    footer.replace("{sttid}", stationID);
    footer.replace("{wifimac}", macAddr);
    page += footer;

    //DEBUG_WM(page);

    server->sendHeader("Content-Length", String(page.length()));
    server->send(200, "text/html", page);

}

/**
 * Save CICADA Station
 */
void CicadaWizard::handleSaveCicadaStation() {

    DEBUG_WM(F("\n\nSAVE CICADA DCP STATION"));
    DEBUG_WM(F("===========================================\n"));

    // Save location on file system
    String lat = server->arg("lat");
    spiffsMan.saveSettings("Latitude", DIR_STATION_LATITUDE, lat);
    String lon = server->arg("lon");
    spiffsMan.saveSettings("Longitude", DIR_STATION_LONGITUDE, lon);

    // Save bucket volume on file system
    String bucketVolume = server->arg("vol");
    bucketVolume.replace(",", ".");
    spiffsMan.saveSettings("Bucket Calibration", DIR_STATION_BUCKET_VOL, bucketVolume);

    // Save time to next reset
    String ttr = server->arg("ttr");
    spiffsMan.saveSettings("Time to Reset", DIR_STATION_TIME_TO_RESET, ttr);

    // Save station name
    String stationName = server->arg("name");
    spiffsMan.saveSettings("Station Name", DIR_STATION_NAME, stationName);

    // Save station name
    String pass = server->arg("spass");
    spiffsMan.saveSettings("Station Pass", DIR_STATION_PASS, pass);

    // Send Time Interval
    String sti = server->arg("sti");
    spiffsMan.saveSettings("Send Time Interval", DIR_STATION_SENDTIMEINTERVAL, sti);

    // Send Time Interval
    String smi = server->arg("smi");
    spiffsMan.saveSettings("Store metadata Interval", DIR_STATION_STOREMETADATA, smi);

    //Redirect Step 2
    handleMQTTSERVER();
}

void CicadaWizard::handleSensors() {

    String page = FPSTR(HTTP_HEAD_HTML);
    page.replace("{v}", "Cicada DCP Wizard");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += _customHeadElement;
    page += FPSTR(HTTP_HEAD_END);

    // Get Temperature Sensor Code
    String codetemp = spiffsMan.getSettings("Code temp", DIR_SENSOR_CODETEMP, false);
    String codehum = spiffsMan.getSettings("Code hum", DIR_SENSOR_CODEHUM, false);
    String codeplu = spiffsMan.getSettings("Code plu", DIR_SENSOR_CODEPLUV, false);
    String codevin = spiffsMan.getSettings("Code vcc in", DIR_SENSOR_CODEVIN, false);
    String codevso = spiffsMan.getSettings("Code vcc so", DIR_SENSOR_CODEVSO, false);

    String dttemp = spiffsMan.getSettings("Data Type temp", DIR_SENSOR_DATATYPETEMP, false);
    String dthum = spiffsMan.getSettings("Data Type hum", DIR_SENSOR_DATATYPEHUM, false);
    String dtplu = spiffsMan.getSettings("Data Type plu", DIR_SENSOR_DATATYPEPLUV, false);
    String dtvin = spiffsMan.getSettings("Data Type vcc in", DIR_SENSOR_DATATYPEVIN, false);
    String dtvso = spiffsMan.getSettings("Data Type vcc so", DIR_SENSOR_DATATYPEVSO, false);

    String collDHT = spiffsMan.getSettings("Coll. time interval DHT", DIR_SENSOR_COLLTINTDHT, false);
    String collplu = spiffsMan.getSettings("Coll. time interval plu", DIR_SENSOR_COLLTINTPLUV, false);
    String collvin = spiffsMan.getSettings("Coll. time interval vcc in", DIR_SENSOR_COLLTINTVIN, false);
    String collvso = spiffsMan.getSettings("Coll. time interval vcc so", DIR_SENSOR_COLLTINTVSO, false);

    // Setup the form
    String form = FPSTR(HTTP_FORM_CONFIG_SENSORS);
    form.replace("{codetemp}", codetemp);
    form.replace("{codehum}", codehum);
    form.replace("{codeplu}", codeplu);
    form.replace("{codevin}", codevin);
    form.replace("{codevso}", codevso);

    form.replace("{dttemp}", dttemp);
    form.replace("{dthum}", dthum);
    form.replace("{dtplu}", dtplu);
    form.replace("{dtvin}", dtvin);
    form.replace("{dtvso}", dtvso);

    form.replace("{collDHT}", collDHT);
    form.replace("{collplu}", collplu);
    form.replace("{collvin}", collvin);
    form.replace("{collvso}", collvso);

    form.replace("{cicadalogo}", HTTP_CICADALOGO);

    page += form;

    // Setup the script
    String script = FPSTR(HTTP_SCRIPT_FORM_CONFIG_SENSORS);
    page += script;

    String footer = FPSTR(HTTP_END);
    footer.replace("{fmwver}", fmwver);
    footer.replace("{sttid}", stationID);
    footer.replace("{wifimac}", macAddr);
    page += footer;

    server->sendHeader("Content-Length", String(page.length()));
    server->send(200, "text/html", page);
}

/**
 * Save CICADA Sensors
 */
void CicadaWizard::handleSaveSensorsConfig() {
    DEBUG_WM(F("\n\nSAVE CICADA DCP Sensors"));
    DEBUG_WM(F("===========================================\n"));

    String codetemp = server->arg("codetemp");
    spiffsMan.saveSettings("code temp", DIR_SENSOR_CODETEMP, codetemp);
    String codehum = server->arg("codehum");
    spiffsMan.saveSettings("code hum", DIR_SENSOR_CODEHUM, codehum);
    String codeplu = server->arg("codeplu");
    spiffsMan.saveSettings("code plu", DIR_SENSOR_CODEPLUV, codeplu);
    String codevin = server->arg("codevin");
    spiffsMan.saveSettings("code vcc in", DIR_SENSOR_CODEVIN, codevin);
    String codevso = server->arg("codevso");
    spiffsMan.saveSettings("code vcc sol", DIR_SENSOR_CODEVSO, codevso);

    String dttemp = server->arg("dttemp");
    spiffsMan.saveSettings("dt temp", DIR_SENSOR_DATATYPETEMP, dttemp);
    String dthum = server->arg("dthum");
    spiffsMan.saveSettings("dt hum", DIR_SENSOR_DATATYPEHUM, dthum);
    String dtplu = server->arg("dtplu");
    spiffsMan.saveSettings("dt plu", DIR_SENSOR_DATATYPEPLUV, dtplu);
    String dtvin = server->arg("dtvin");
    spiffsMan.saveSettings("dt vcc in", DIR_SENSOR_DATATYPEVIN, dtvin);
    String dtvso = server->arg("dtvso");
    spiffsMan.saveSettings("dt vcc so", DIR_SENSOR_DATATYPEVSO, dtvso);

    String collDHT = server->arg("collDHT");
    spiffsMan.saveSettings("coll DHT", DIR_SENSOR_COLLTINTDHT, collDHT);
    String collplu = server->arg("collplu");
    spiffsMan.saveSettings("coll plu", DIR_SENSOR_COLLTINTPLUV, collplu);
    String collvin = server->arg("collvin");
    spiffsMan.saveSettings("coll vcc in", DIR_SENSOR_COLLTINTVIN, collvin);
    String collvso = server->arg("collvso");
    spiffsMan.saveSettings("coll vcc so", DIR_SENSOR_COLLTINTVSO, collvso);

    //Redirect Step 2
    handleMQTTSERVER();
}

void CicadaWizard::handleMQTTSERVER() {

    String page = FPSTR(HTTP_HEAD_HTML);
    page.replace("{v}", "Cicada DCP Wizard");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += _customHeadElement;
    page += FPSTR(HTTP_HEAD_END);

    // Get MQTT Host
    String host = spiffsMan.getSettings("MQTT Host", DIR_MQTT_SERVER, true);
    // Get MQTT Port
    String port = spiffsMan.getSettings("MQTT Port", DIR_MQTT_PORT, false);
    // Get MQTT User
    String user = spiffsMan.getSettings("MQTT User", DIR_MQTT_USER, false);
    // Get MQTT Password
    String pass = spiffsMan.getSettings("MQTT Password", DIR_MQTT_PWD, true);
    // Get MQTT Topic
    String topic = spiffsMan.getSettings("MQTT Topic", DIR_MQTT_TOPIC, false);

    // Setup the form
    String form = FPSTR(HTTP_FORM_CONFIG_MQTT);
    form.replace("{host}", host);
    form.replace("{port}", port);
    form.replace("{user}", user);
    form.replace("{pass}", pass);
    form.replace("{topic}", topic);

    form.replace("{cicadalogo}", HTTP_CICADALOGO);

    page += form;

    // Setup the script
    String script = FPSTR(HTTP_SCRIPT_FORM_CONFIG_MQTT);
    page += script;

    String footer = FPSTR(HTTP_END);
    footer.replace("{fmwver}", fmwver);
    footer.replace("{sttid}", stationID);
    footer.replace("{wifimac}", macAddr);
    page += footer;

    server->sendHeader("Content-Length", String(page.length()));
    server->send(200, "text/html", page);
}

/**
 * Save CICADA MQTT
 */
void CicadaWizard::handleSaveCicadaMQTT() {
    DEBUG_WM(F("\n\nSAVE CICADA DCP MQTT"));
    DEBUG_WM(F("===========================================\n"));

    // Get MQTT Host
    String host = server->arg("host");
    spiffsMan.saveSettings("MQTT Host", DIR_MQTT_SERVER, FILE_MQTT_SERVER, host);
    // Get MQTT Port
    String port = server->arg("port");
    spiffsMan.saveSettings("MQTT Port", DIR_MQTT_PORT, port);
    // Get MQTT User
    String user = server->arg("user");
    spiffsMan.saveSettings("MQTT User", DIR_MQTT_USER, user);
    // Get MQTT Password
    String pass = server->arg("pass");
    spiffsMan.saveSettings("MQTT Password", DIR_MQTT_PWD, FILE_MQTT_PWD, pass);
    // Get MQTT Topic
    String topic = server->arg("topic");
    spiffsMan.saveSettings("MQTT Topic", DIR_MQTT_TOPIC, topic);

    handleSIMCard();

}

void CicadaWizard::handleSIMCard() {
    String page = FPSTR(HTTP_HEAD_HTML);
    page.replace("{v}", "Cicada DCP Wizard");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += _customHeadElement;
    page += FPSTR(HTTP_HEAD_END);

    // Get MQTT Host
    String apn = spiffsMan.getSettings("SIM Carrier APN", DIR_SIMCARD_APN, true);
    // Get MQTT User
    String user = spiffsMan.getSettings("SIM Carrier APN User", DIR_SIMCARD_USER, false);
    // Get MQTT Password
    String pwd = spiffsMan.getSettings("SIM  Carrier APN Pwd", DIR_SIMCARD_PWD, true);

    // Setup the form
    String form = FPSTR(HTTP_FORM_CONFIG_SIM);
    form.replace("{apn}", apn);
    form.replace("{userapn}", user);
    form.replace("{pwdapn}", pwd);

    form.replace("{cicadalogo}", HTTP_CICADALOGO);

    page += form;

    // Setup the script
    String script = FPSTR(HTTP_SCRIPT_FORM_CONFIG_SIM);
    page += script;

    String footer = FPSTR(HTTP_END);
    footer.replace("{fmwver}", fmwver);
    footer.replace("{sttid}", stationID);
    footer.replace("{wifimac}", macAddr);
    page += footer;

    server->sendHeader("Content-Length", String(page.length()));
    server->send(200, "text/html", page);
}

/**
 * Save CICADA SIM Card
 */
void CicadaWizard::handleSaveCicadaSIMCard() {
    DEBUG_WM(F("\n\nSAVE CICADA SIM CARD"));
    DEBUG_WM(F("===========================================\n"));

    // Get SIM APN
    String apn = server->arg("apn");
    spiffsMan.saveSettings("SIM Carrier APN", DIR_SIMCARD_APN, FILE_SIMCARD_APN, apn);
    // Get SIM User
    String user = server->arg("userapn");
    spiffsMan.saveSettings("SIM Carrier APN User", DIR_SIMCARD_USER, user);
    // Get MQTT Password
    String pwd = server->arg("pwdapn");
    spiffsMan.saveSettings("SIM  Carrier APN Pwd", DIR_SIMCARD_PWD, FILE_SIMCARD_PWD, pwd);

    handleWIFIConfig("");
}

void CicadaWizard::handleWIFIConfig(String msg) {
    String page = FPSTR(HTTP_HEAD_HTML);
    page.replace("{v}", "Cicada DCP Wizard");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += _customHeadElement;
    page += FPSTR(HTTP_HEAD_END);
    page += FPSTR(HTTP_HEAD_CONFIG_WIFI);
    page.replace("{cicadalogo}", HTTP_CICADALOGO);
    page.replace("{msg}", msg);

    // Setup the form
    String form = FPSTR(HTTP_FORM_CONFIG_WIFI);
    page += form;

    // Setup the script
    String script = FPSTR(HTTP_SCRIPT_FORM_CONFIG_WIFI);
    page += script;

    String footer = FPSTR(HTTP_END);
    footer.replace("{fmwver}", fmwver);
    footer.replace("{sttid}", stationID);
    footer.replace("{wifimac}", macAddr);
    page += footer;

    server->sendHeader("Content-Length", String(page.length()));
    server->send(200, "text/html", page);
}

void CicadaWizard::handleDelWifi() {
    deleteWifiCredentials();
    handleWIFIConfig("The credentials have been deleted!");
}

void CicadaWizard::handleConfirmFactoryReset() {
    String page = FPSTR(HTTP_HEAD_HTML);
    page.replace("{v}", "Cicada DCP Wizard");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += _customHeadElement;
    page += FPSTR(HTTP_HEAD_END);
    page += F(HTTP_CICADALOGO);
    page += F("<h1>Cicada DCP Wizard</h1>");
    page += F("<h2>Factory Reset!</h2>");
    page += F("<dl>");
    page += F("<h1 style='color:red'>Warning!</h1>");
    page += F("<h1 style='color:red; text-align:center}'><b>DELETE ALL</b> saved settings?</h1>");

    page += F("<div>");
    page += F("<form action='/factoryresetYES' method='post'><button style='background-color:#208514'>Yes</button></form>");
    page += F("<form action='/info' method='post'><button style='background-color:#890D0D'>No</button></form>");
    page += F("</div>");

    String footer = FPSTR(HTTP_END);
    footer.replace("{fmwver}", fmwver);
    footer.replace("{sttid}", stationID);
    footer.replace("{wifimac}", macAddr);
    page += footer;

    server->sendHeader("Content-Length", String(page.length()));
    server->send(200, "text/html", page);
}

void CicadaWizard::handleFactoryReset() {
    spiffsMan.FSDeleteFiles("/");
    handleRoot();
}


/******************************************************************************
 * END
 ******************************************************************************/

/** Wifi config page handler */
void CicadaWizard::handleWifi(boolean scan) {

    String page = FPSTR(HTTP_HEAD_HTML);
    page.replace("{v}", "Cicada DCP Wizard");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += _customHeadElement;
    page += FPSTR(HTTP_HEAD_END);
    page += FPSTR(HTTP_HEAD_CONFIG_WIFI);
    page.replace("{cicadalogo}", HTTP_CICADALOGO);
    page.replace("{msg}", "");

    if (scan) {
        //Hack André Ivo
        DEBUG_WM(F("Hack to scan (André Ivo)"));
        WiFi.disconnect();
        ///////////////

        int n = WiFi.scanNetworks();
        DEBUG_WM(F("Scan done"));
        if (n == 0) {
            DEBUG_WM(F("No networks found"));
            page += F("No networks found. Refresh to scan again.");
        } else {

            //sort networks
            int indices[n];
            for (int i = 0; i < n; i++) {
                indices[i] = i;
            }

            // RSSI SORT

            // old sort
            for (int i = 0; i < n; i++) {
                for (int j = i + 1; j < n; j++) {
                    if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
                        std::swap(indices[i], indices[j]);
                    }
                }
            }

            /*std::sort(indices, indices + n, [](const int & a, const int & b) -> bool
              {
              return WiFi.RSSI(a) > WiFi.RSSI(b);
              });*/

            // remove duplicates ( must be RSSI sorted )
            if (_removeDuplicateAPs) {
                String cssid;
                for (int i = 0; i < n; i++) {
                    if (indices[i] == -1) continue;
                    cssid = WiFi.SSID(indices[i]);
                    for (int j = i + 1; j < n; j++) {
                        if (cssid == WiFi.SSID(indices[j])) {
                            DEBUG_WM("DUP AP: " + WiFi.SSID(indices[j]));
                            indices[j] = -1; // set dup aps to index -1
                        }
                    }
                }
            }

            //display networks in page
            for (int i = 0; i < n; i++) {
                if (indices[i] == -1) continue; // skip dups
                DEBUG_WM(WiFi.SSID(indices[i]));
                DEBUG_WM(WiFi.RSSI(indices[i]));
                int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));

                if (_minimumQuality == -1 || _minimumQuality < quality) {
                    String item = FPSTR(HTTP_ITEM_CONFIG_WIFI);
                    String rssiQ;
                    rssiQ += quality;
                    item.replace("{v}", WiFi.SSID(indices[i]));
                    item.replace("{r}", rssiQ);
#if defined(ESP8266)
                    if (WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE) {
#else
                    if (WiFi.encryptionType(indices[i]) != WIFI_AUTH_OPEN) {
#endif
                        item.replace("{i}", "l");
                    } else {
                        item.replace("{i}", "");
                    }
                    //DEBUG_WM(item);
                    page += item;
                    delay(0);
                } else {
                    DEBUG_WM(F("Skipping due to quality"));
                }

            }
            page += "<br/>";
        }
    }

    // Get MQTT Host
    String tmpssid = spiffsMan.getSettings("SSID", DIR_WIFI_SSID, false);
    // Get MQTT User
    String tmppwd = spiffsMan.getSettings("Password", DIR_WIFI_PWD, false);


    String formWifi = FPSTR(HTTP_FORM_START_CONFIG_WIFI);
    formWifi.replace("{ssid}", tmpssid);
    formWifi.replace("{pass}", tmppwd);
    page += formWifi;

    char parLength[2];
    // add the extra parameters to the form
    for (int i = 0; i < _paramsCount; i++) {
        if (_params[i] == NULL) {
            break;
        }

        String pitem = FPSTR(HTTP_FORM_PARAM_CONFIG_WIFI);
        if (_params[i]->getID() != NULL) {
            pitem.replace("{i}", _params[i]->getID());
            pitem.replace("{n}", _params[i]->getID());
            pitem.replace("{p}", _params[i]->getPlaceholder());
            snprintf(parLength, 2, "%d", _params[i]->getValueLength());
            pitem.replace("{l}", parLength);
            pitem.replace("{v}", _params[i]->getValue());
            pitem.replace("{c}", _params[i]->getCustomHTML());
        } else {
            pitem = _params[i]->getCustomHTML();
        }

        page += pitem;
    }
    if (_params[0] != NULL) {
        page += "<br/>";
    }

    if (_sta_static_ip) {

        String item = FPSTR(HTTP_FORM_PARAM_CONFIG_WIFI);
        item.replace("{i}", "ip");
        item.replace("{n}", "ip");
        item.replace("{p}", "Static IP");
        item.replace("{l}", "15");
        item.replace("{v}", _sta_static_ip.toString());

        page += item;

        item = FPSTR(HTTP_FORM_PARAM_CONFIG_WIFI);
        item.replace("{i}", "gw");
        item.replace("{n}", "gw");
        item.replace("{p}", "Static Gateway");
        item.replace("{l}", "15");
        item.replace("{v}", _sta_static_gw.toString());

        page += item;

        item = FPSTR(HTTP_FORM_PARAM_CONFIG_WIFI);
        item.replace("{i}", "sn");
        item.replace("{n}", "sn");
        item.replace("{p}", "Subnet");
        item.replace("{l}", "15");
        item.replace("{v}", _sta_static_sn.toString());

        page += item;

        page += "<br/>";
    }

    page += FPSTR(HTTP_FORM_CONFIG_WIFI_END);
    page += FPSTR(HTTP_SCAN_LINK_CONFIG_WIFI);

    String footer = FPSTR(HTTP_END);
    footer.replace("{fmwver}", fmwver);
    footer.replace("{sttid}", stationID);
    footer.replace("{wifimac}", macAddr);
    page += footer;

    server->sendHeader("Content-Length", String(page.length()));
    server->send(200, "text/html", page);

    DEBUG_WM(F("Sent config page"));
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void CicadaWizard::handleWifiSave() {
    DEBUG_WM(F("WiFi save"));

    //SAVE/connect here
    _ssid = server->arg("s").c_str();
    _pass = server->arg("p").c_str();

    //parameters
    for (int i = 0; i < _paramsCount; i++) {
        if (_params[i] == NULL) {
            break;
        }
        //read parameter
        String value = server->arg(_params[i]->getID()).c_str();
        //store it in array
        value.toCharArray(_params[i]->_value, _params[i]->_length);
        DEBUG_WM(F("Parameter"));
        DEBUG_WM(_params[i]->getID());
        DEBUG_WM(value);
    }

    if (server->arg("ip") != "") {
        DEBUG_WM(F("static ip"));
        DEBUG_WM(server->arg("ip"));
        //_sta_static_ip.fromString(server->arg("ip"));
        String ip = server->arg("ip");
        optionalIPFromString(&_sta_static_ip, ip.c_str());
    }
    if (server->arg("gw") != "") {
        DEBUG_WM(F("static gateway"));
        DEBUG_WM(server->arg("gw"));
        String gw = server->arg("gw");
        optionalIPFromString(&_sta_static_gw, gw.c_str());
    }
    if (server->arg("sn") != "") {

        DEBUG_WM(F("static netmask"));
        DEBUG_WM(server->arg("sn"));
        String sn = server->arg("sn");
        optionalIPFromString(&_sta_static_sn, sn.c_str());
    }

    // SSID
    spiffsMan.saveSettings("SSID", DIR_WIFI_SSID, _ssid);
    // Password
    spiffsMan.saveSettings("Password", DIR_WIFI_PWD, _pass);

    connect = true; //signal ready to connect/reset
    handleInfo();
}

/** Handle the info page */
void CicadaWizard::handleInfo() {

    DEBUG_WM(F("Info"));

    String page = FPSTR(HTTP_HEAD_HTML);
    page.replace("{v}", "Cicada DCP Wizard");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += _customHeadElement;
    page += FPSTR(HTTP_HEAD_END);
    page += F(HTTP_CICADALOGO);
    page += F("<h1>Cicada DCP Wizard</h1>");
    page += F("<h2>Configurations is done!</h2>");
    page += F("<dl>");

    page += F("<h4>Station</h4>");
    // Get the previous Station Name
    String name = spiffsMan.getSettings("Station Name", DIR_STATION_NAME, false);
    // Get the previous Station pass
    String spass = spiffsMan.getSettings("Station password", DIR_STATION_PASS, false);
    // Get the previous Latitude
    String lat = spiffsMan.getSettings("Latitude", DIR_STATION_LATITUDE, false);
    // Get the previous Longitude
    String lon = spiffsMan.getSettings("Longitude", DIR_STATION_LONGITUDE, false);
    // Get the previous bucket configuration
    String vol = spiffsMan.getSettings("Bucket calibration", DIR_STATION_BUCKET_VOL, false);
    // Get the previous Send time interval
    String sti = spiffsMan.getSettings("Send time interval", DIR_STATION_SENDTIMEINTERVAL, false);
    page += F("<p style='font-size:0.8rem;'><b>Station Name:</b> ");
    page += name;
    page += F("</p>");
    page += F("<p style='font-size:0.8rem;'><b>Station Password:</b> ");
    page += spass;
    page += F("</p>");
    page += F("<p style='font-size:0.8rem;'><b>Latitude:</b> ");
    page += lat;
    page += F("</p>");
    page += F("<p style='font-size:0.8rem;'><b>Longitude:</b> ");
    page += lon;
    page += F("</p>");
    page += F("<p style='font-size:0.8rem;'><b>Bucket calibration:</b> ");
    page += vol;
    page += F("</p>");
    page += F("<p style='font-size:0.8rem;'><b>Send time interval:</b> ");
    page += sti;
    page += F("</p><br>");

    page += F("<h4>MQTT Server</h4>");
    // Get MQTT Host
    String host = spiffsMan.getSettings("MQTT Host", DIR_MQTT_SERVER, true);
    // Get MQTT Port
    String port = spiffsMan.getSettings("MQTT Port", DIR_MQTT_PORT, false);
    // Get MQTT User
    String user = spiffsMan.getSettings("MQTT User", DIR_MQTT_USER, false);
    // Get MQTT Password
    String pass = spiffsMan.getSettings("MQTT Password", DIR_MQTT_PWD, true);
    // Get MQTT Topic
    String topic = spiffsMan.getSettings("MQTT Topic", DIR_MQTT_TOPIC, false);
    page += F("<p style='font-size:0.8rem;'><b>Host Server:</b> ");
    page += host;
    page += F("</p>");
    page += F("<p style='font-size:0.8rem;'><b>Port:</b> ");
    page += port;
    page += F("</p>");
    page += F("<p style='font-size:0.8rem;'><b>User:</b> ");
    page += user;
    page += F("</p>");
    page += F("<p style='font-size:0.8rem;'><b>Password:</b> ");
    page += pass;
    page += F("</p>");
    page += F("<p style='font-size:0.8rem;'><b>Topic:</b> ");
    page += topic;
    page += F("</p><br>");


    page += F("<h4>SIM Card Carrier APN</h4>");
    // Get MQTT Host
    String apn = spiffsMan.getSettings("SIM Carrier APN", DIR_SIMCARD_APN, true);
    // Get MQTT User
    String userapn = spiffsMan.getSettings("SIM Carrier APN User", DIR_SIMCARD_USER, false);
    // Get MQTT Password
    String pwdapn = spiffsMan.getSettings("SIM  Carrier APN Pwd", DIR_SIMCARD_PWD, true);

    page += F("<p style='font-size:0.8rem;'><b>APN</b> ");
    page += apn;
    page += F("</p>");
    page += F("<p style='font-size:0.8rem;'><b>User:</b> ");
    page += userapn;
    page += F("</p>");
    page += F("<p style='font-size:0.8rem;'><b>Password:</b> ");
    page += pwdapn;
    page += F("</p><br>");

    page += F("<h4>WIFI</h4>");

    // Get MQTT Host
    String tmpssid = spiffsMan.getSettings("SSID", DIR_WIFI_SSID, false);
    // Get MQTT User
    String tmppwd = spiffsMan.getSettings("Password", DIR_WIFI_PWD, false);

    page += F("<p style='font-size:0.8rem;'><b>SSID:</b> ");
    page += tmpssid;
    page += F("</p>");
    page += F("<p style='font-size:0.8rem;'><b>Password:</b> ");
    page += tmppwd;
    page += F("</p><br>");

    page += F("<form action='/reboot' method='post'><button>Finish and Reboot Module</button></form>");
    page += F("<form action='/factoryreset' method='post'><button>Factory Reset</button></form>");

    String footer = FPSTR(HTTP_END);
    footer.replace("{fmwver}", fmwver);
    footer.replace("{sttid}", stationID);
    footer.replace("{wifimac}", macAddr);
    page += footer;

    server->sendHeader("Content-Length", String(page.length()));
    server->send(200, "text/html", page);

    DEBUG_WM(F("Sent info page"));
}

/** Handle the reset page */
void CicadaWizard::handleReboot() {

    DEBUG_WM(F("Reset"));

    String page = FPSTR(HTTP_HEAD_HTML);
    page.replace("{v}", "Cicada DCP Wizard");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += _customHeadElement;
    page += FPSTR(HTTP_HEAD_END);
    page += F(HTTP_CICADALOGO);
    page += F("<h1>Cicada DCP Wizard</h1>");
    page += F("<h2>Reboot System!</h2>");
    page += F("<dl>");
    page += F("<h2>Module will reset in a few seconds.</h2>");
    page += F("<h2>Close this page!</h2>");

    String footer = FPSTR(HTTP_END);
    footer.replace("{fmwver}", fmwver);
    footer.replace("{sttid}", stationID);
    footer.replace("{wifimac}", macAddr);
    page += footer;

    server->sendHeader("Content-Length", String(page.length()));
    server->send(200, "text/html", page);

    DEBUG_WM(F("Sent reset page"));
    delay(5000);
#if defined(ESP8266)
    ESP.reset();
#else
    ESP.restart();
#endif
    delay(2000);
}

void CicadaWizard::handleNotFound() {
    if (captivePortal()) { // If captive portal redirect instead of displaying the error page.
        return;
    }
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server->uri();
    message += "\nMethod: ";
    message += (server->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server->args();
    message += "\n";

    for (uint8_t i = 0; i < server->args(); i++) {

        message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
    }
    server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server->sendHeader("Pragma", "no-cache");
    server->sendHeader("Expires", "-1");
    server->sendHeader("Content-Length", String(message.length()));
    server->send(404, "text/plain", message);
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean CicadaWizard::captivePortal() {
    if (!isIp(server->hostHeader())) {
        DEBUG_WM(F("Request redirected to captive portal"));
        server->sendHeader("Location", String("http://") + toStringIp(server->client().localIP()), true);
        server->send(302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        server->client().stop(); // Stop is needed because we sent no content length

        return true;
    }
    return false;
}

//start up config portal callback

void CicadaWizard::setAPCallback(void (*func)(CicadaWizard* myCicadaWizard)) {

    _apcallback = func;
}

//start up save config callback

void CicadaWizard::setSaveConfigCallback(void (*func)(void)) {

    _savecallback = func;
}

//sets a custom element to add to head, like a new style tag

void CicadaWizard::setCustomHeadElement(const char* element) {

    _customHeadElement = element;
}

//if this is true, remove duplicated Access Points - defaut true

void CicadaWizard::setRemoveDuplicateAPs(boolean removeDuplicates) {

    _removeDuplicateAPs = removeDuplicates;
}

template <typename Generic>
void CicadaWizard::DEBUG_WM(Generic text) {
    if (_debug) {

        Serial.print("*WM: ");
        Serial.println(text);
    }
}

int CicadaWizard::getRSSIasQuality(int RSSI) {
    int quality = 0;

    if (RSSI <= -100) {
        quality = 0;
    } else if (RSSI >= -50) {
        quality = 100;
    } else {

        quality = 2 * (RSSI + 100);
    }
    return quality;
}

/** Is this an IP? */
boolean CicadaWizard::isIp(String str) {
    for (int i = 0; i < str.length(); i++) {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9')) {

            return false;
        }
    }
    return true;
}

/** IP to String? */
String CicadaWizard::toStringIp(IPAddress ip) {
    String res = "";
    for (int i = 0; i < 3; i++) {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}
