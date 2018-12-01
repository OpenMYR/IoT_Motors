#include <Arduino.h>
#include <ESP8266mDNS.h>
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
#include "include/wifi.h"

AsyncWebServer websrv(80);
void handlePost(AsyncWebServerRequest* req);

udp_srv* UDP_server;

void (*motor_task_ptr)(os_event_t*);
void (*ack_task_ptr)(os_event_t*);
void (*estop_task_ptr)(os_event_t*);

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
  estop_task_ptr = &(command_layer::endstop_ack);
  system_os_task(motor_driver_os_task, MOTOR_DRIVER_TASK_PRIO, task_queue::queue, TASK_QUEUE_LENGTH);
  system_os_task(ack_task_ptr, ACK_TASK_PRIO, task_queue::queue, TASK_QUEUE_LENGTH);
  system_os_task(estop_task_ptr, ESTOP_TASK_PRIO, task_queue::queue, TASK_QUEUE_LENGTH);
      Serial.begin(115200);
  Serial.println("Booting");
  command_layer::init_motor_driver();

  init_wifi();

  ArduinoOTA.onStart([=]() {
    if(UDP_server)
      UDP_server->end();
    websrv.reset();
    command_layer::stop_motor();
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

  MDNS.begin("openmyr");

   if (!SPIFFS.begin())
  {
    // Serious problem
    Serial.println("SPIFFS Mount failed");
    ArduinoOTA.setPasswordHash("acc3a26f06f0d71c9d06380b14139aaa");
  } else {
    Serial.println("SPIFFS Mount successful");
    File f = SPIFFS.open("/ota_pass.txt", "r");
    if(f)
      ArduinoOTA.setPasswordHash(f.readString().c_str());
    else
      ArduinoOTA.setPasswordHash("acc3a26f06f0d71c9d06380b14139aaa");
    f.close();
  }

  websrv.serveStatic("/", SPIFFS, MOTOR_TYPE == 0 ? "/web_srv/" : "/web_step/");
  websrv.serveStatic("/", SPIFFS,  MOTOR_TYPE == 0 ? "/web_srv/" : "/web_step/").setDefaultFile("index.html");
  websrv.on("/", HTTP_POST, handlePost);
  websrv.begin();

  Serial.println("HTTP server active");

  UDP_server = new udp_srv();
  UDP_server->begin();

  ArduinoOTA.begin();
}

void loop() {
    ArduinoOTA.handle();
    UDP_server->prompt_broadcast();
    delay(1000);
}

void handlePost(AsyncWebServerRequest* req)
{
  Serial.println("POST DETECTED");
  Serial.printf("%d params\n", req->params());
  for(int i = 0; i < req->params(); i++)
  {
    command_layer::json_process_command(req->getParam(i)->value().c_str());
  }
  req->send(202);
}