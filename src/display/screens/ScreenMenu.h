#pragma once
#include <Arduino.h>

void ScreenMenu_init();
void ScreenMenu_draw();

void ScreenMenu_prev();
void ScreenMenu_next();

bool ScreenMenu_wasSelected();
bool ScreenMenu_handleInput();
uint8_t ScreenMenu_getSelectedIndex();