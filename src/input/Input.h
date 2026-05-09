#pragma once
#include <Arduino.h>

enum ButtonAction
{
    BTN_NONE,
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_SELECT
};

void initInput();
ButtonAction getButtonAction();