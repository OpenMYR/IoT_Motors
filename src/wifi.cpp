#include "include/wifi.h"
#include <Arduino.h>
#include <FS.h>
#include "md5.h"
#include "ArduinoOTA.h"
#include <ESP8266WiFi.h>

static uint32_t num_retries = 0;

WiFiEventHandler stationDisconnectHandler;
WiFiEventHandler stationGotIPHandler;

void wifiStationDisconnectHandler(const WiFiEventStationModeDisconnected& evt)
{
    if(num_retries <= 3)
    {
        Serial.print("Connection fail! Retrying...");
        num_retries++;
    }
    else
    {
        num_retries = 0;
        change_opmode(false, "", "");
    }
}

void wifiStationGotIPHandler(const WiFiEventStationModeGotIP& evt)
{
  Serial.println("Got Local IP");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void change_opmode(bool station, char* ssid, char* pass)
{
    if(station)
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, pass);
    }
    else
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("OpenMYR-Motor");
    }
}

void init_wifi()
{
    stationDisconnectHandler = WiFi.onStationModeDisconnected(&wifiStationDisconnectHandler);
    stationGotIPHandler = WiFi.onStationModeGotIP(&wifiStationGotIPHandler);
    switch(WiFi.getMode())
    {
        case WIFI_OFF:
        case WIFI_AP:
        case WIFI_AP_STA:
            WiFi.softAP("OpenMYR-Motor");
            break;
        case WIFI_STA:
            WiFi.begin();
            break;
    }
}

void change_ota_pass(char* old, char* pass)
{
    MD5Builder md5;
    md5.begin();
    md5.add(pass);
    md5.calculate();

    FSInfo info;
    if(SPIFFS.info(info))
    {
        File f = SPIFFS.open("/ota_pass.txt", "r");
        if(f)
        {
            const char* old_pass = f.readString().c_str();
            if(strcmp(old_pass, md5.toString().c_str()) == 0)
            {
                f.close();
                f = SPIFFS.open("/ota_pass.txt", "w");
                if(f)
                {
                    f.print(md5.toString());
                    f.close();
                }
                else
                {
                    Serial.println("OTA Password change failed: SPIFFS error");
                }
            }
            else
            {
                Serial.println("Wrong OTA Password!");
            }
        }
        else
        {
            if(strcmp(old, "openmyr") == 0)
            {
                File f = SPIFFS.open("/ota_pass.txt", "w");
                if(f)
                {
                    f.print(md5.toString());
                    f.close();
                }
                else
                {
                    Serial.println("OTA Password change failed: SPIFFS error");
                }
            }
            else
            {
                Serial.println("Wrong OTA password!");
            }
        }
    }
}