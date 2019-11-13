#pragma once

void init_wifi();
void change_opmode(bool station, const char* ssid, const char* pass);
void change_ota_pass(char* old, char* pass);