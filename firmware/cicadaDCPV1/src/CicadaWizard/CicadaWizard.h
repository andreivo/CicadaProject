/**************************************************************
   CicadaWizard is a library for the ESP32/Arduino platform
   to enable easy configuration and reconfiguration of Cicada DCP using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer/examples/CaptivePortalAdvanced
   https://github.com/tzapu
   Built by André Ivo https://github.com/tzapu
 **************************************************************/

#ifndef CicadaWizard_h
#define CicadaWizard_h

#include <FS.h> // FS must be the first
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#else
#include <WiFi.h>
#include <WebServer.h>
#endif
#include <DNSServer.h>
#include <memory>

#include <SPIFFS.h>
#include "../SPIFFS/SPIFFSManager.h"
#include "../SPIFFS/CicadaSPIFFSFile.h"

#if defined(ESP8266)
extern "C" {
#include "user_interface.h"
}
#define ESP_getChipId()   (ESP.getChipId())
#else
#include <esp_wifi.h>
#define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())
#endif

const char HTTP_HEAD_HTML[] PROGMEM = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM = "<style>.c{text-align:center}.q{float:right;width:64px;text-align:right}div,input{padding:5px;font-size:1em}input{width:95%}body{text-align:center;font-family:verdana}button{border:0;border-radius:.3rem;background-color:#00055B;color:#fff;line-height:2.4rem;font-size:1.0rem;width:100%;margin:3px 0 3px 0}p{text-align:left;margin:0;padding:0}h1{margin:0 0 5px 0;padding:0;font-size:1.2rem;text-align:center}h2{margin:0 0 5px 0;padding:0;font-size:1rem;text-align:center}h3{margin:0 0 5px 0;padding:0;font-size:0.8rem;text-align:center}h4{margin:0 0 5px 0;padding:0;font-size:1.0rem;text-align:left;text-decoration: underline;}h6{margin:0 0 5px 0;padding:0}</style>";
const char HTTP_SCRIPT[] PROGMEM = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_END[] PROGMEM = "</div><br/><br/><pre style='text-align:right; font-size:.6em;'><b>ID:</b>&nbsp;{sttid}<br/><b>MAC:</b>&nbsp;{wifimac}<br/><b>Firmware:</b>&nbsp;v{fmwver}</pre><i style='text-align:right; font-size:.6em; display:block'>Cicada DCP &copy; 2021. All Rights Reserved.</i></body></html>";

const char HTTP_SAVED[] PROGMEM = "<div>Credentials Saved<br />Trying to connect Weread to network.<br />If it fails reconnect to AP to try again</div>";

/***
 * CICADA HTML
 */

const char HTTP_CICADALOGO[] PROGMEM = "<div style='text-align:center;min-width:260px'><img alt='' src='data:image/png;base64,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'></div>";
//const char HTTP_CICADALOGO[] PROGMEM = "";
//Station
const char HTTP_FORM_CONFIG_STATION[] PROGMEM = "<form name='frmConfig' method='post' action='/saveCicadaStation' onsubmit='return validate();'>{cicadalogo}<h1>Cicada DCP Wizard</h1><h2>1-4: Station Configuration</h2><p id='msg' style='color: red'></p><p>Station Name</p><input type='text' name='name' value='{name}' maxlength='15'/><p>Station Password</p><input type='text' name='spass' value='{spass}'/><p>Latitude</p><input type='text' name='lat' value='{lat}' /><p>Longitute</p><input type='text' name='lon' value='{lon}' /><p>Bucket Calibration</p><input type='text' name='vol' value='{vol}' /><p>Time slot to send (minutes)</p><input type='text' name='sti' value='{sti}' /><p>Time slot store metadata (hours)</p><input type='text' name='smi' value='{smi}' /><br><input type='hidden' name='ttr' /><button type='submit'>Save and Next</button></form><br/><div class='c'><a href='/sensors'>Sensors Configuration</a></div>";
const char HTTP_SCRIPT_FORM_CONFIG_STATION[] PROGMEM = "<script type='text/javascript'>var msg = document.getElementById('msg');function validate() {msg.innerHTML = '';var result = true;var f = document.forms['frmConfig'];var vol = parseFloat(f['vol'].value);var sti = parseFloat(f['sti'].value);var smi = parseFloat(f['smi'].value);var lat = parseFloat(f['lat'].value);var lon = parseFloat(f['lon'].value);var ttr = f['ttr'];var name = f['name'].value;var pass = f['spass'].value;if(!lat || isNaN(lat)) {msg.innerHTML += '- Latitude is required!<br>';result = false;}if(!lon || isNaN(lon)) {msg.innerHTML += '- Longitude is required!<br>';result = false;}if(!vol || isNaN(vol) || (vol < 2.00 || vol > 5.00)) {msg.innerHTML += '- The calibration value must be between 2.00 and 5.00 (e.g.: 2.47)<br>';result = false;}if(!sti || isNaN(sti) || (sti < 5.00 || sti > 30.00)) {msg.innerHTML += '- The time slot to send must be between 5 and 30 min (e.g.: 10)<br>';result = false;}if(!smi || isNaN(smi) || (smi < 6.00 || smi > 12.00)) {msg.innerHTML += '- The time slot store metadata must be between 6 and 12 hours (e.g.: 12)<br>';result = false;}if(!name|| !(/^([a-z0-9\-\_]+)$/ig.test(name))) {msg.innerHTML += ' - Invalid name. Must contain only (aA-zZ), (0-9), (-) or (_)';result = false;}if(!pass) {msg.innerHTML += ' - Password is required!';result = false;}var now = new Date();var nextReset = new Date();nextReset.setHours({RESET_TIME}, 0);if(now > nextReset) {nextReset.setDate(nextReset.getDate() + 1);}var nowSecs = parseInt(now.getTime() / 1000);var nRs = parseInt(nextReset.getTime() / 1000);var ttrInSecs = parseInt(nRs - nowSecs);ttr.value = ttrInSecs;return result;}</script>";

//Sensors
const char HTTP_FORM_CONFIG_SENSORS[] PROGMEM = "<form name='frmSensors' method='post' action='/saveSensorsConfig' onsubmit='return validate()'>{cicadalogo}<h1>Cicada DCP Wizard</h1><h2>1.1-4: Sensors Configuration</h2><p id='msg' style='color:red'></p><br><h4>Temperature</h4><p>Code</p><input name='codetemp' value='{codetemp}'><p>DataType</p><input name='dttemp' value='{dttemp}'><p>Time slot to collection (minutes)</p><input name='colltemp' value='{colltemp}'><br><h4>Humidity</h4><p>Code</p><input name='codehum' value='{codehum}'><p>DataType</p><input name='dthum' value='{dthum}'><p>Time slot to collection (minutes)</p><input name='collhum' value='{collhum}'><br><h4>Pluviometer</h4><p>Code</p><input name='codeplu' value='{codeplu}'><p>DataType</p><input name='dtplu' value='{dtplu}'><p>Time slot to collection (minutes)</p><input name='collplu' value='{collplu}'><br><h4>Battery Voltage</h4><p>Code</p><input name='codebtv' value='{codebtv}'><p>DataType</p><input name='dtbtv' value='{dtbtv}'><p>Time slot to collection (minutes)</p><input name='collbtv' value='{collbtv}'><br><h4>Battery Current</h4><p>Code</p><input name='codebtc' value='{codebtc}'><p>DataType</p><input name='dtbtc' value='{dtbtc}'><p>Time slot to collection (minutes)</p><input name='collbtc' value='{collbtc}'><br><button type='submit'>Save and Next</button></form>";
const char HTTP_SCRIPT_FORM_CONFIG_SENSORS[] PROGMEM = "<script type='text/javascript'>function validate(){msg.innerHTML='';var e=!0,r=document.forms.frmSensors,n=parseInt(r.codetemp.value),i=r.dttemp.value,t=parseInt(r.colltemp.value),a=parseInt(r.codehum.value),l=r.dthum.value,u=parseInt(r.collhum.value),s=parseInt(r.codeplu.value),m=r.dtplu.value,d=parseInt(r.collplu.value),o=parseInt(r.codebtv.value),v=r.dtbtv.value,b=parseInt(r.collbtv.value),T=parseInt(r.codebtc.value),c=r.dtbtc.value,g=parseInt(r.collbtc.value);return(!n||isNaN(n))&&(msg.innerHTML+='- Code Temp. is required or inválid!<br>',e=!1),i||(msg.innerHTML+='- DataType Temp. is required!<br>',e=!1),(!t||isNaN(t)||5>t||t>60)&&(msg.innerHTML+='- The Temp. collection time interval value must be between 5 and 60 min (e.g.: 10)<br>',e=!1),(!a||isNaN(a))&&(msg.innerHTML+='- Code Hum. is required or inválid!<br>',e=!1),l||(msg.innerHTML+='- DataType Hum. is required!<br>',e=!1),(!u||isNaN(u)||5>u||u>60)&&(msg.innerHTML+='- The Hum. collection time interval value must be between 5 and 60 min (e.g.: 10)<br>',e=!1),(!s||isNaN(s))&&(msg.innerHTML+='- Code Plu. is required or inválid!<br>',e=!1),m||(msg.innerHTML+='- DataType Plu. is required!<br>',e=!1),(!d||isNaN(d)||5>d||d>60)&&(msg.innerHTML+='- The Pluv. collection time interval value must be between 5 and 60 min (e.g.: 10)<br>',e=!1),(!o||isNaN(o))&&(msg.innerHTML+='- Code Bat. Volt. is required or inválid!<br>',e=!1),v||(msg.innerHTML+='- DataType Bat. Volt. is required!<br>',e=!1),(!b||isNaN(b)||5>b||b>60)&&(msg.innerHTML+='- The Bat. Volt. collection time interval value must be between 5 and 60 min (e.g.: 10)<br>',e=!1),(!T||isNaN(T))&&(msg.innerHTML+='- Code Bat. Curr. is required or inválid!<br>',e=!1),c||(msg.innerHTML+='- DataType Bat. Curr. is required!<br>',e=!1),(!g||isNaN(g)||5>g||g>60)&&(msg.innerHTML+='- The Bat. Curr. collection time interval value must be between 5 and 60 min (e.g.: 10)<br>',e=!1),e}var msg=document.getElementById('msg');</script>";

//MQTT Server
const char HTTP_FORM_CONFIG_MQTT[] PROGMEM = "<form name='frmMQTT' method='post' action='/saveCicadaMQTT' onsubmit='return validate()'>{cicadalogo}<h1>Cicada DCP Wizard</h1><h2>2-4: MQTT Server Configuration</h2><p id='msg' style='color:red'></p><p>MQTT Host Server</p><input name='host' value='{host}'><p>MQTT Port</p><input name='port' value='{port}'><p>MQTT User</p><input name='user' value='{user}'><p>MQTT Password</p><input name='pass' value='{pass}'><p>MQTT Topic</p><input name='topic' value='{topic}'><br><button type='submit'>Save and Next</button></form>";
const char HTTP_SCRIPT_FORM_CONFIG_MQTT[] PROGMEM = "<script type='text/javascript'>var msg = document.getElementById('msg');function validate() {msg.innerHTML = '';var result = true;var f = document.forms['frmMQTT'];var host = f['host'].value;var port = parseInt(f['port'].value);var user = f['user'].value;var pass = f['pass'].value;var topic = f['topic'].value;if(!host) {msg.innerHTML += '- Host is required<br>';result = false;}if(!port || isNaN(port)) {msg.innerHTML += '- The Port value must be integer (e.g.: 1883)';result = false;}if(!user) {msg.innerHTML += '- User is required<br>'; result = false;}if(!pass) {msg.innerHTML += '- Password is required<br>';result = false;}if(!topic) {msg.innerHTML += '- Topic is required<br>';result = false;}return result;}</script>";

//Sim Card Telecom
const char HTTP_FORM_CONFIG_SIM[] PROGMEM = "<form name='frmSIM' method='post' action='/saveCicadaSIM'>{cicadalogo}<h1>Cicada DCP Wizard</h1><h2>3-4: SIM Card Configuration</h2><h3>(If you don't use, leave in blank!)</h3><h3>(SIM card OR WIFI is required!)</h3><p id='msg' style='color:red'></p><p>Carrier APN</p><input name='apn' value='{apn}'><p>Carrier APN User</p><input name='userapn' value='{userapn}'><p>Carrier APN Password</p><input name='pwdapn' value='{pwdapn}'><br><button type='submit'>Save and Next</button></form>";
const char HTTP_SCRIPT_FORM_CONFIG_SIM[] PROGMEM = "";

//Wifi
const char HTTP_HEAD_CONFIG_WIFI[] PROGMEM = "{cicadalogo}<h1>Cicada DCP Wizard</h1><h2>4-4: WIFI Configuration</h2><h3>(If you don't use, leave in blank!)</h3><h3>(SIM card OR WIFI is required!)</h3><p id='msg' style='color:red'>{msg}</p>";
const char HTTP_FORM_CONFIG_WIFI[] PROGMEM = "<form action='/wifi'><button>Configure WiFi</button></form><form action='/0wifi'><button>Configure WiFi (No Scan)</button></form><form action='/delwifi'><button>Delete WIFI credentials</button></form><form action='/info'><br><button type='submit'>Next</button></form>";
const char HTTP_SCRIPT_FORM_CONFIG_WIFI[] PROGMEM = "";
const char HTTP_FORM_CONFIG_WIFI_END[] PROGMEM = "<br/><button type='submit'>Save and Next</button></form>";
const char HTTP_SCAN_LINK_CONFIG_WIFI[] PROGMEM = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char HTTP_ITEM_CONFIG_WIFI[] PROGMEM = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_PARAM_CONFIG_WIFI[] PROGMEM = "<br/><input id='{i}' name='{n}' length={l} placeholder='{p}' value='{v}' {c}>";
const char HTTP_FORM_START_CONFIG_WIFI[] PROGMEM = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID' value='{ssid}'><br/><input id='p' name='p' length=64 placeholder='password' value='{pass}'><br/>";


/***************/

#define WIFI_MANAGER_MAX_PARAMS 10

class CicadaWizardParameter {
public:
    CicadaWizardParameter(const char *custom);
    CicadaWizardParameter(const char *id, const char *placeholder, const char *defaultValue, int length);
    CicadaWizardParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    const char *getID();
    const char *getValue();
    const char *getPlaceholder();
    int getValueLength();
    const char *getCustomHTML();
private:
    const char *_id;
    const char *_placeholder;
    char *_value;
    int _length;
    const char *_customHTML;

    void init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    friend class CicadaWizard;
};

class CicadaWizard {
public:
    CicadaWizard();

    /***
     * CICADA
     */
    boolean startWizardPortal(char const *apName, char const *apPassword = NULL);

    boolean autoConnect();
    boolean autoConnect(char const *apName, char const *apPassword = NULL);

    //if you want to always start the config portal, without trying to connect first
    boolean startConfigPortal();
    boolean startConfigPortal(char const *apName, char const *apPassword = NULL);

    // get the AP name of the config portal, so it can be used in the callback
    String getConfigPortalSSID();
    String getSSID();
    String getPassword();
    void deleteWifiCredentials();

    //sets timeout before webserver loop ends and exits even if there has been no setup.
    //useful for devices that failed to connect at some point and got stuck in a webserver loop
    //in seconds setConfigPortalTimeout is a new name for setTimeout
    void setConfigPortalTimeout(unsigned long seconds);
    void setTimeout(unsigned long seconds);

    //sets timeout for which to attempt connecting, useful if you get a lot of failed connects
    void setConnectTimeout(unsigned long seconds);


    void setDebugOutput(boolean debug);
    //defaults to not showing anything under 8% signal quality if called
    void setMinimumSignalQuality(int quality = 8);
    //sets a custom ip /gateway /subnet configuration
    void setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //sets config for a static IP
    void setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //called when AP mode and config portal is started
    void setAPCallback(void (*func)(CicadaWizard*));
    //called when settings have been changed and connection was successful
    void setSaveConfigCallback(void (*func)(void));
    //adds a custom parameter
    void addParameter(CicadaWizardParameter *p);
    //if this is set, it will exit after config, even if connection is unsuccessful.
    void setBreakAfterConfig(boolean shouldBreak);
    //if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)
    //TODO
    //if this is set, customise style
    void setCustomHeadElement(const char* element);
    //if this is true, remove duplicated Access Points - defaut true
    void setRemoveDuplicateAPs(boolean removeDuplicates);

private:
    std::unique_ptr<DNSServer> dnsServer;
#ifdef ESP8266
    std::unique_ptr<ESP8266WebServer> server;
#else
    std::unique_ptr<WebServer> server;
#endif

    //const int     WM_DONE                 = 0;
    //const int     WM_WAIT                 = 10;

    //const String  HTTP_HEAD_HTML = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

    void setupConfigPortal();
    void startWPS();

    const char* _apName = "no-net";
    const char* _apPassword = NULL;
    String _ssid = "";
    String _pass = "";
    unsigned long _configPortalTimeout = 0;
    unsigned long _connectTimeout = 0;
    unsigned long _configPortalStart = 0;

    IPAddress _ap_static_ip;
    IPAddress _ap_static_gw;
    IPAddress _ap_static_sn;
    IPAddress _sta_static_ip;
    IPAddress _sta_static_gw;
    IPAddress _sta_static_sn;

    int _paramsCount = 0;
    int _minimumQuality = -1;
    boolean _removeDuplicateAPs = true;
    boolean _shouldBreakAfterConfig = false;
    boolean _tryWPS = false;

    const char* _customHeadElement = "";

    //String        getEEPROMString(int start, int len);
    //void          setEEPROMString(int start, int len, String string);

    void handleSaveCicadaStation();
    void handleMQTTSERVER();
    void handleSaveCicadaMQTT();
    void handleSIMCard();
    void handleSaveCicadaSIMCard();
    void handleWIFIConfig(String msg);
    void handleDelWifi();
    void handleConfirmFactoryReset();
    void handleFactoryReset();
    void handleSensors();
    void handleSaveSensorsConfig();

    /***
     * END
     */


    int status = WL_IDLE_STATUS;
    int connectWifi(String ssid, String pass);
    uint8_t waitForConnectResult();

    void handleRoot();

    void handleWifi(boolean scan);
    void handleWifiSave();
    void handleInfo();
    void handleReboot();
    void handleNotFound();
    void handle204();
    boolean captivePortal();
    boolean configPortalHasTimeout();

    // DNS server
    const byte DNS_PORT = 53;

    //helpers
    int getRSSIasQuality(int RSSI);
    boolean isIp(String str);
    String toStringIp(IPAddress ip);

    boolean connect;
    boolean _debug = true;

    void (*_apcallback)(CicadaWizard*) = NULL;
    void (*_savecallback)(void) = NULL;

    CicadaWizardParameter * _params[WIFI_MANAGER_MAX_PARAMS];

    template <typename Generic>
    void DEBUG_WM(Generic text);

    template <class T>
    auto optionalIPFromString(T *obj, const char *s) -> decltype( obj->fromString(s)) {
        return obj->fromString(s);
    }
    auto optionalIPFromString(...) -> bool {
        DEBUG_WM("NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work.");
        return false;
    }
};

#endif
