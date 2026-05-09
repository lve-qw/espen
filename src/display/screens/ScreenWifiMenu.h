#pragma once
#include <Arduino.h>

void ScreenWifiMenu_init();
void ScreenWifiMenu_draw();
void ScreenWifiMenu_prev();
void ScreenWifiMenu_next();
bool ScreenWifiMenu_wasSelected();

uint8_t ScreenWifiMenu_getSelectedIndex();