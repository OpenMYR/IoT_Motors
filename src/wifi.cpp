#include "include/wifi.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

static uint32_t num_retries = 0;

WiFiEventHandler stationDisconnectHandler;

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