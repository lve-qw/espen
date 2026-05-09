#include "Input.h"
#include "config/pins.h"

struct BtnState
{
    uint8_t pin;
    bool lastState;
    unsigned long lastDebounce;
};

// Массив состояний кнопок
static BtnState btns[5] = {
    {PIN_BTN_UP, true, 0},
    {PIN_BTN_DOWN, true, 0},
    {PIN_BTN_LEFT, true, 0},
    {PIN_BTN_RIGHT, true, 0},
    {PIN_BTN_SEL, true, 0}};

void initInput()
{
    for (auto &b : btns)
    {
        pinMode(b.pin, INPUT_PULLUP); // Кнопки замыкают на GND
        b.lastState = HIGH;
    }
}

ButtonAction getButtonAction()
{
    unsigned long now = millis();
    for (int i = 0; i < 5; i++)
    {
        bool reading = digitalRead(btns[i].pin);
        if (reading != btns[i].lastState)
        {
            btns[i].lastDebounce = now;
            btns[i].lastState = reading;
            // Срабатываем только при нажатии (HIGH -> LOW)
            if (!reading)
            {
                return (ButtonAction)(BTN_UP + i);
            }
        }
    }
    return BTN_NONE;
}