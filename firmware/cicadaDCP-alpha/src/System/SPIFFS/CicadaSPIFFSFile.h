/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */

const String RESET_TIME = "7,0";
const String DIR_STATION_ID = "/stt/id";
const String DIR_STATION_LATITUDE = "/stt/lat";
const String DIR_STATION_LONGITUDE = "/stt/lon";
const String DIR_STATION_SENDTIMEINTERVAL = "/stt/sti";
const String DIR_STATION_TIME_TO_RESET = "/stt/ttr";
const String DIR_STATION_NAME = "/stt/name";
const String DIR_STATION_PASS = "/stt/pass";
const String DIR_STATION_STOREMETADATA = "/stt/smi";
const String DIR_STATION_SEHOST = "/stt/host";
const String FILE_STATION_SEHOST = "hostserver";
const String DIR_STATION_SEPATH = "/stt/path";
const String FILE_STATION_SEPATH = "pathserver";
const String DIR_STATION_SEPORT = "/stt/port";
const String DIR_STATION_SETIME = "/stt/tup";
const String DIR_STATION_OWNAME = "/stt/owname";
const String FILE_STATION_OWNAME = "owname";
const String DIR_STATION_OWEMAIL = "/stt/owemail";
const String FILE_STATION_OWEMAIL = "owemail";
const String DIR_STATION_OWPHONE = "/stt/owphone";
const String FILE_STATION_OWPHONE = "owphone";

const String DIR_SENSOR_CODETEMP = "/sen/codetemp";
const String DIR_SENSOR_CODEHUM = "/sen/codehum";
const String DIR_SENSOR_CODEPLUV = "/sen/codeplu";
const String DIR_SENSOR_CODEVIN = "/sen/codevin";
const String DIR_SENSOR_CODEVBA = "/sen/codevba";

const String DIR_SENSOR_DATATYPETEMP = "/sen/dttemp";
const String DIR_SENSOR_DATATYPEHUM = "/sen/dthum";
const String DIR_SENSOR_DATATYPEPLUV = "/sen/dtplu";
const String DIR_SENSOR_DATATYPEVIN = "/sen/dtvin";
const String DIR_SENSOR_DATATYPEVBA = "/sen/dtvba";

const String DIR_SENSOR_COLLTINTDHT = "/sen/colldht";
const String DIR_SENSOR_COLLTINTPLUV = "/sen/collplu";
const String DIR_SENSOR_COLLTINTVIN = "/sen/collvin";
const String DIR_SENSOR_COLLTINTVBA = "/sen/collvba";

const String DIR_SENSOR_PLUVIO_BUCKET_VOL = "/sen/pluviobucket";
const String DIR_SENSOR_PLUVIO_AREA = "/sen/pluvioarea";

const String DIR_FIRMWARE_VERSION = "/fmwver";
const String DIR_MQTT_SERVER = "/mqtt/host";
const String FILE_MQTT_SERVER = "hostserver";
const String DIR_MQTT_PORT = "/mqtt/port";
const String DIR_MQTT_USER = "/mqtt/user";
const String DIR_MQTT_PWD = "/mqtt/pwd";
const String FILE_MQTT_PWD = "pwdserver";
const String DIR_MQTT_TOPIC = "/mqtt/topic";
const String DIR_SIMCARD_APN = "/simcard/apn";
const String FILE_SIMCARD_APN = "apnhost";
const String DIR_SIMCARD_USER = "/simcard/user";
const String DIR_SIMCARD_PWD = "/simcard/pwd";
const String FILE_SIMCARD_PWD = "pwdapn";
const String DIR_WIFI_SSID = "/wifi/ssid";
const String DIR_WIFI_PWD = "/wifi/pwd";
