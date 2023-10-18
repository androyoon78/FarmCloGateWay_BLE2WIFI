#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_system.h"
#include <sstream>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// Load Wi-Fi library
#include <WiFi.h>
#include "version.h"

String uuid ="0";


// Replace with your network credentials
const char* ssid = "JJHouse";
const char* password = "01068015475";

String farmCloSensor = "MJ_HT_V1";

// Set web server port number to 80
WiFiServer server(80);
WiFiClient client;

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// for BLE and Bluetooth
#define SCAN_TIME  3 // seconds
boolean METRIC = true; //Set true for metric system; false for imperial
BLEScan *pBLEScan;

float current_moisture = -100;
float previous_moisture = -100;
float current_temperature = -100;
float previous_temperature = -100;
float current_battery = -100;

String convertFloatToString(float f);
float CelciusToFahrenheit(float Celsius);

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        //Serial.print(advertisedDevice.getName().c_str());
        //Serial.print(" ");
        //Serial.println(advertisedDevice.getAddress().toString().c_str());
        
        if (advertisedDevice.haveName() && advertisedDevice.haveServiceData() && !advertisedDevice.getName().compare("MJ_HT_V1")) {

            int serviceDataCount = advertisedDevice.getServiceDataCount();
            std::string strServiceData = advertisedDevice.getServiceData(0);

            uint8_t cServiceData[100];
            char charServiceData[100];

            strServiceData.copy((char *)cServiceData, strServiceData.length(), 0);

            Serial.printf("\n\nAdvertised Device: %s\n", advertisedDevice.getName().c_str());
            //Serial.printf("\n\nAdvertised Name exists: %u\n", advertisedDevice.haveName());
            //Serial.printf("Device has ServiceData: %u\n", advertisedDevice.haveServiceData());
            
            // 장치가 여러 개의 serviceUUID를 가지고 있다면 모두 출력
            if(advertisedDevice.haveServiceUUID()) {
                int serviceUUIDCount = advertisedDevice.getServiceUUIDCount();
                Serial.printf("Device has %d Service \n", advertisedDevice.getServiceUUIDCount());
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

                    current_battery = ((float)value / 255.0) * 100; // value를 100분율로 변환
                    Serial.printf("Current Battery: %.2f%%\n", current_battery);
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
    pBLEScan->setActiveScan(false); //active scan uses more power, but get results faster
    pBLEScan->setInterval(0xA0);
    pBLEScan->setWindow(0x60);
}

void setup() {
    Serial.begin(115200);

    sprintf(build_version, "%d%02d%02d%d", YEAR, MONTH + 1, DAY, DATE_AS_INT);

    Serial.printf("            ♪~ ♬ ♪♬~♪ ♪~ ♬ \r\n");
    Serial.printf("──────▄▀▄─────▄▀▄ ♪♬~♪ ♪~ ♬\r\n");
    Serial.printf("─────▄█░░▀▀▀▀▀░░█▄  ♪♬~♪ ♪~ ♬ ♪\r\n");
    Serial.printf("─▄▄──█░░░░░░░░░░░█──▄▄ ID: %s\r\n", uuid);
    Serial.printf("█▄▄█─█░░▀░░┬░░▀░░█─█▄▄█ \r\n");
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
    Serial.printf("\t << Product: FarmCLo Gateway B2W (GBW) >>\r\n");
    Serial.printf("\t << Version: %s >>\r\n", build_version);
    Serial.printf("\t Current CPU frq = %d Mhz\r\n", getCpuFrequencyMhz());
    Serial.printf("\t Date : %s %s\r\n", __DATE__, __TIME__);

    // Connect to Wi-Fi network with SSID and password
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  
    
    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
  
    initBluetooth();
}


void loop()
{
    client = server.available(); // Listen for incoming clients
/*
    Serial.printf("Start BLE scan for %d seconds...\n", SCAN_TIME);
    BLEScan *pBLEScan = BLEDevice::getScan(); // create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
*/
    pBLEScan->setActiveScan(true); // active scan uses more power, but get results faster

    BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
    int count = foundDevices.getCount();
    printf("Found device count : %d\n", count);

    if (client)
    {
      // Check if the client has sent a request
      if (client.available())
      {
        // Read the HTTP request line by line
        while (client.available())
        {
          char c = client.read();
          header += c;
          if (c == '\n')
          {
            // End of the HTTP request
            break;
          }
        }
  
        // Clear the header variable
        header = "";
      }
  
      // Send the HTTP response to the client
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Connection: close");
      //client.println("Refresh: 5"); // 5초마다 페이지 자동 새로 고침
      client.println("<meta http-equiv='refresh' content='5'>");
      client.println();
      
      // HTML 코드
      client.println("<!DOCTYPE html><html>");
      client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
      client.println("<link rel=\"icon\" href=\"data:,\">");
      client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
      client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
      client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
      client.println(".button2 {background-color: #555555;}</style></head>");
      client.println("<body><h1>Farmclo Web Server</h1>");
      
      // Display Bluetooth Devices for farmCloSensor
      client.println("<h2>Bluetooth Devices for farmCloSensor:</h2>");
      client.println("<div style='text-align:center;'>"); // 텍스트를 중앙 정렬하는 div 요소 시작
      client.println("<table border='1' style='margin:0 auto;'>"); // 테이블을 가운데 정렬하고자 하는 스타일 추가
      client.println("<tr><th>Name</th><th>Address</th></tr>");
      for (int i = 0; i < count; i++) {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);
        bool nameReceived = device.haveName();
    
        // Check if the device's name matches farmCloSensor
        if (nameReceived && device.getName() == farmCloSensor.c_str()) {
          client.println("<tr>");
          client.print("<td>");
          client.print(device.getName().c_str()); 
          client.print("</td>");
          client.print("<td>");
          client.print(device.getAddress().toString().c_str());
          client.print("</td>");
          client.println("</tr>");
        }
      }
      client.println("</table>");
      client.println("</div>");

           client.println("<h2>Device Data:</h2>");
      client.println("<div style='text-align:center;'>");
      client.println("<table border='1' style='margin:0 auto;'>");
      client.println("<tr><th>Temperature</th><th>Moisture</th><th>Battery</th></tr>");

      client.println("<tr>");
      client.printf("<td>%.2f°C</td>", current_temperature);
      client.printf("<td>%.2f%%</td>", current_moisture);
      client.printf("<td>%.2f%%</td>", current_battery);
      client.println("</tr>");

      client.println("</table>");
      client.println("</div>"); // div 요소 끝

      // Close the HTML page
      client.println("</body></html>");
      
      // Close the client connection
      client.stop();
  }

  if ((current_temperature != -100) && (current_moisture != -100))
  {
    Serial.println("-------------------------------------------------");
    Serial.printf("Temperature: %.2f\n", current_temperature);
    Serial.printf("Moisture: %.2f\n", current_moisture);
    if (current_battery > 0)
    {
      Serial.printf("Battery: %f\n", current_battery);
      SendDataToServer();
    }

    Serial.println("-------------------------------------------------");
    
  }
  

  /*
  for(int i=0; i<60; i++)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println(".");
  */

  
}

char serverip[100];
const char* serverHttp = "http://";
char* serverMainName = "farmclo.com";
const char* serverSubName = "/api/v2/sensing/";
char *serverPort = "4040";
int portNumber = 0;
long rssi = 0;

void SendDataToServer()
{
    String postUrl;
    String postStr;

    Serial.println("FarmClo GateWay B2W");
    postUrl = serverSubName + uuid;
    postStr ="temperature=";
    postStr += String(current_temperature);
    postStr +="&moisture=";
    postStr += String(current_moisture);
    postStr +="&battery=";
    postStr +=String(current_battery);
    postStr +="&fwversion=";
    postStr += build_version;
    postStr +="&rssi=";
    postStr += String(WiFi.RSSI());

    if(client.connect(serverip, portNumber))
    {
        Serial.println("[client.connect]FarmClo GateWay B2W");
            
        client.println("POST " + postUrl + " HTTP/1.1");
        client.println("Host: " + String(serverip) + ":" + portNumber);
        client.println("Content-Type: application/x-www-form-urlencoded");
        client.print("Content-Length: ");
        client.println(postStr.length());
        client.println();
        client.print(postStr);
    } else {
        Serial.println("[client.connect] failed");
    }
    Serial.println(String(serverip) + ":" + String(portNumber) + postUrl + "?" + postStr);

    if (client.connected()) {
        client.stop();
    }
}
