#include <Arduino.h>
#include "display/Display.h"
#include "display/screens/ScreenStartup.h"
#include "display/screens/ScreenMenu.h"
#include "display/screens/ScreenWifiMenu.h"
#include "display/screens/ScreenWifiScan.h"
#include "input/Input.h"

enum AppState
{
    STATE_STARTUP,
    STATE_MENU,
    STATE_WIFI_MENU,
    STATE_WIFI_SCAN
};
static AppState state = STATE_STARTUP;

void setup()
{
    Serial.begin(115200);
    delay(100);

    Display::init();
    initInput();
}

void loop()
{
    ButtonAction btn = getButtonAction();

    switch (state)
    {
    case STATE_STARTUP:
        if (ScreenStartup_loop() == 1)
        {
            state = STATE_MENU;
            ScreenMenu_init();
        }
        break;

    case STATE_MENU:
        ScreenMenu_draw();
        if (btn == BTN_UP)
            ScreenMenu_prev();
        if (btn == BTN_DOWN)
            ScreenMenu_next();

        if (btn == BTN_SELECT)
        {
            uint8_t idx = ScreenMenu_getSelectedIndex();
            Serial.printf("MAIN MENU SELECT -> idx: %d\n", idx);

            if (idx == 3)
            {
                state = STATE_WIFI_MENU;
                ScreenWifiMenu_init();
            }
            // Добавьте другие пункты по аналогии:
            // if(idx == 2) { state = STATE_BT_MENU; ... }
        }
        break;

    case STATE_WIFI_MENU:
        ScreenWifiMenu_draw();
        if (btn == BTN_LEFT)
        {
            state = STATE_MENU;
            ScreenMenu_init();
        }
        else if (btn == BTN_SELECT)
        {
            uint8_t idx = ScreenWifiMenu_getSelectedIndex();
            Serial.printf("WIFI MENU SELECT -> idx: %d\n", idx);
            if (idx == 0)
            {
                state = STATE_WIFI_SCAN;
                ScreenWifiScan_init();
            }
            // case 1: Graph...
        }
        break;

    case STATE_WIFI_SCAN:
        ScreenWifiScan_draw();
        if (ScreenWifiScan_handleInput())
        {
            state = STATE_WIFI_MENU;
            ScreenWifiMenu_init();
        }
        break;
    }
}