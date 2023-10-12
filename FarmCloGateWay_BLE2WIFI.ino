#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_system.h"
#include <sstream>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "version.h"

#define SCAN_TIME  3 // seconds

String uuid ="0";

boolean METRIC = true; //Set true for metric system; false for imperial

BLEScan *pBLEScan;

void IRAM_ATTR resetModule(){
    ets_printf("reboot\n");
    esp_restart();
}

float current_moisture = -100;
float previous_moisture = -100;
float current_temperature = -100;
float previous_temperature = -100;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        if (advertisedDevice.haveName() && advertisedDevice.haveServiceData() && !advertisedDevice.getName().compare("MJ_HT_V1")) {

            int serviceDataCount = advertisedDevice.getServiceDataCount();
            std::string strServiceData = advertisedDevice.getServiceData(0);

            uint8_t cServiceData[100];
            char charServiceData[100];

            strServiceData.copy((char *)cServiceData, strServiceData.length(), 0);

            //Serial.printf("\n\nAdvertised Device: %s\n", advertisedDevice.toString().c_str());
            Serial.printf("\n\nAdvertised Name exists: %u\n", advertisedDevice.haveName());
            Serial.printf("Device has ServiceData: %u\n", advertisedDevice.haveServiceData());
            // 장치가 여러 개의 serviceUUID를 가지고 있다면 모두 출력
            if(advertisedDevice.haveServiceUUID()) {
                int serviceUUIDCount = advertisedDevice.getServiceUUIDCount();
                for(int i = 0; i < serviceUUIDCount; i++) {
                    Serial.printf("Service UUID %d: %s\n", i+1, advertisedDevice.getServiceUUID(i).toString().c_str());
                }
            }
                
            for (int i=0;i<strServiceData.length();i++) {
                sprintf(&charServiceData[i*2], "%02x", cServiceData[i]);
            }

            std::stringstream ss;
            ss << "fe95" << charServiceData;
            
            Serial.print("Payload:");
            Serial.println(ss.str().c_str());

            char eventLog[256];
            unsigned long value, value2;
            char charValue[5] = {0,};

            Serial.print("cServiceData: [");
            for(int i = 0; i < strServiceData.length(); i++) {
                Serial.printf("%02X", cServiceData[i]);
                if (i != strServiceData.length() - 1) {
                    Serial.print(", ");
                }
            }
            Serial.println("]");

            Serial.printf("===>>>>> %02X\r\n", cServiceData[11]);
            
            switch (cServiceData[11]) {
                case 0x04:
                    sprintf(charValue, "%02X%02X", cServiceData[15], cServiceData[14]);
                    value = strtol(charValue, 0, 16);
                    if(METRIC)
                    {
                      current_temperature = (float)value/10;
                    }else
                    {
                      current_temperature = CelciusToFahrenheit((float)value/10);
                    }
 
                    break;
                case 0x06:
                    sprintf(charValue, "%02X%02X", cServiceData[15], cServiceData[14]);
                    value = strtol(charValue, 0, 16);  
                    current_moisture = (float)value/10;
                     
                    Serial.printf("MOISTURE_EVENT: %s, %d\n", charValue, value);
                    break;
                case 0x0A:
                    sprintf(charValue, "%02X", cServiceData[14]);
                    value = strtol(charValue, 0, 16);                    
                    Serial.printf("BATTERY_EVENT: %s, %d\n", charValue, value);
                    break;
                case 0x0D:
                    sprintf(charValue, "%02X%02X", cServiceData[15], cServiceData[14]);
                    value = strtol(charValue, 0, 16);      
                    if(METRIC)
                    {
                      current_temperature = (float)value/10;
                    }else
                    {
                      current_temperature = CelciusToFahrenheit((float)value/10);
                    }
        
                    Serial.printf("TEMPERATURE_EVENT: %s, %d\n", charValue, value);                    
                    sprintf(charValue, "%02X%02X", cServiceData[17], cServiceData[16]);
                    value2 = strtol(charValue, 0, 16);
                    current_moisture = (float)value2/10;
                                  
                    Serial.printf("MOISTURE_EVENT: %s, %d\n", charValue, value2);
                    break;
            }
        }
    }
};


String convertFloatToString(float f)
{
  String s = String(f,1);
  return s;
}

float CelciusToFahrenheit(float Celsius)
{
 float Fahrenheit=0;
 Fahrenheit = Celsius * 9/5 + 32;
 return Fahrenheit;
}

void initBluetooth()
{
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->setInterval(0x50);
    pBLEScan->setWindow(0x30);
}

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);

    sprintf(build_version, "%d%02d%02d%d", YEAR, MONTH + 1, DAY, DATE_AS_INT);

    Serial.println("");

    Serial.printf("            ♪~ ♬ ♪♬~♪ ♪~ ♬ \r\n");
    Serial.printf("──────▄▀▄─────▄▀▄ ♪♬~♪ ♪~ ♬\r\n");
    Serial.printf("─────▄█░░▀▀▀▀▀░░█▄  ♪♬~♪ ♪~ ♬ ♪\r\n");
    Serial.printf("─▄▄──█░░░░░░░░░░░█──▄▄ ID: %s\r\n", uuid);
    Serial.printf("█▄▄█─█░░▀░░┬░░▀░░█─█▄▄█ GateWay B2W: %s\r\n", build_version);
    Serial.printf("███████████████████████████████████████████████████████████\r\n");
    Serial.printf("███████╗░█████╗░██████╗░███╗░░░███╗░█████╗░██╗░░░░░░█████╗░\r\n");
    Serial.printf("██╔════╝██╔══██╗██╔══██╗████╗░████║██╔══██╗██║░░░░░██╔══██╗\r\n");
    Serial.printf("█████╗░░███████║██████╔╝██╔████╔██║██║░░╚═╝██║░░░░░██║░░██║\r\n");
    Serial.printf("██╔══╝░░██╔══██║██╔══██╗██║╚██╔╝██║██║░░██╗██║░░░░░██║░░██║\r\n");
    Serial.printf("██║░░░░░██║░░██║██║░░██║██║░╚═╝░██║╚█████╔╝███████╗╚█████╔╝\r\n");
    Serial.printf("╚═╝░░░░░╚═╝░░╚═╝╚═╝░░╚═╝╚═╝░░░░░╚═╝░╚════╝░╚══════╝░╚════╝░\r\n");
    Serial.printf("███████████████████████████████████████████████████████████\r\n");
    Serial.println("♪~ ♬ ♪♬~♪ ♪~ ♬ ♪♬~♪ ♪~ ♬ ♪♬~♪ ♪~ ♬ ♪ ♪~ ♬ ♪♬~♪ ♪~ ♬ ♪♬~♪ ♪~ ♬ ♪♬~♪ ♪~ ♬ ♪ ♪♬~♪ ♪~ ♬ ♪♪♬~\r\n");
    Serial.printf("==================================================================================================\r\n");

    initBluetooth();
}

void loop() {
    char printLog[256];
    Serial.printf("Start BLE scan for %d seconds...\n", SCAN_TIME);
    BLEScan* pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
    int count = foundDevices.getCount();
    printf("Found device count : %d\n", count);

    delay(100);
}
