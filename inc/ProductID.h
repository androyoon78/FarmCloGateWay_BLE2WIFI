#ifndef _PRODUCTID_H_
#define _PRODUCTID_H_

//#define _ANDRO_TEST_
//#define _NOWIFI_TEST_
#define _CONFIG_TUNNING_SENSOR_
#define _CONFIG_DEBUG_RAW_

//#define _CONFIG_LONG_
#define _CONFIG_SMALL_BAT_

#define AP_IPADDRESS 10,0,1,10
#define _CONFIG_SENSORS_
#define _CONFIG_SOIL_SENSOR_
#define _CONFIG_FARMCLO_

#ifdef _CONFIG_FARMCLO_
String VENDOR_NAME = "FarmCLO_";
//#define _CONFIG_JSON_
#define _CONFIG_HTTP_
//#define _CONFIG_TME_          // CP Model
#define _CONFIG_TMB_            // Battery Model
#endif

String hostname = "Damoatech Soil Sensor";
const char* serverHttp = "http://";
char* serverMainName = "farmclo.com";
const char* serverSubName = "/api/v2/sensing/";
char *serverPort = "4040";
long rssi = 0;

unsigned int measureCycle = 1;

String categorySensor_sms = "/sms";
String categorySensor_cms = "/cms";
String categorySensor_mms = "/mms";
/////
/////            <input style="width: 40px;" type="text" name="port" placeholder="1880">/api/v1/wifi-sensor
#endif
