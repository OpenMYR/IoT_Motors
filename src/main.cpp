#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <cstdlib>
#include <cstring>
#include "ESPAsyncWebServer.h"
#include "include/udp_srv.h"
#include "include/command_layer.h"
#include "include/task_queue.h"
#include "include/task_prio.h"

const char* ssid = "";
const char* password = "";

AsyncWebServer websrv(80);

udp_srv* UDP_server;

void (*motor_task_ptr)(os_event_t*);
void (*ack_task_ptr)(os_event_t*);

extern "C"
{
  void motor_driver_os_task(os_event_t *event)
  {
    motor_task_ptr(event);
  }
}

void setup() {
  motor_task_ptr = &(command_layer::motor_driver_task_passthrough);
  ack_task_ptr = &(command_layer::acknowledge_command);
  system_os_task(motor_driver_os_task, MOTOR_DRIVER_TASK_PRIO, task_queue::queue, TASK_QUEUE_LENGTH);
  system_os_task(ack_task_ptr, ACK_TASK_PRIO, task_queue::queue, TASK_QUEUE_LENGTH);
  command_layer::init_motor_driver();
      Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

   if (!SPIFFS.begin())
  {
    // Serious problem
    Serial.println("SPIFFS Mount failed");
  } else {
    Serial.println("SPIFFS Mount successful");
    File f = SPIFFS.open("/test.txt", "r");
    Serial.printf(f.readString().c_str());
    f.close();
  }

  websrv.serveStatic("/", SPIFFS, "/web/");
  websrv.serveStatic("/", SPIFFS, "/web/").setDefaultFile("index.html");
  websrv.begin();

  Serial.println("HTTP server active");

  UDP_server = new udp_srv();
  UDP_server->begin();
}

void loop() {
    ArduinoOTA.handle();
    UDP_server->prompt_broadcast();
    delay(1000);
}
