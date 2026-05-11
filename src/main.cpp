#include <Arduino.h>
#include <WiFi.h>
#include <esp_netif.h>
#include <nvs_flash.h>

#include "display/Display.h"
#include "display/screens/ScreenStartup.h"
#include "display/screens/mainMenu/Menu.h"
#include "input/Input.h"

#include "display/screens/wifiMenu/menu/Menu.h"
#include "display/screens/wifiMenu/scan/Scan.h"
#include "display/screens/wifiMenu/graph/Graph.h"

enum AppState
{
    STATE_STARTUP,
    STATE_MENU,
    STATE_WIFI_MENU,
    STATE_WIFI_SCAN,
    STATE_WIFI_GRAPH,
};
static AppState state = STATE_STARTUP;

void setup()
{
    Serial.begin(115200);
    delay(1500);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    WiFi.mode(WIFI_STA);
    delay(50);
    WiFi.begin("init", "dummy_password");
    delay(100);
    WiFi.disconnect(true);
    delay(200);
    WiFi.mode(WIFI_STA);
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

        if (btn == BTN_UP)
            ScreenWifiMenu_prev();
        if (btn == BTN_DOWN)
            ScreenWifiMenu_next();

        if (btn == BTN_LEFT)
        {
            state = STATE_MENU;
            ScreenMenu_init();
        }
        else if (btn == BTN_SELECT)
        {
            uint8_t idx = ScreenWifiMenu_getSelectedIndex();
            Serial.printf("WIFI MENU SELECT -> idx: %d\n", idx);

            switch (idx)
            {
            case 0:
                state = STATE_WIFI_SCAN;
                ScreenWifiScan_init();
                break;
            case 1:
                state = STATE_WIFI_GRAPH;
                ScreenWifiGraph_init();
                break;
            case 2: /* state = STATE_WIFI_DEAUTH; ... */
                break;
            case 3: /* state = STATE_WIFI_BEFLOOD; ... */
                break;
            case 4: /* state = STATE_WIFI_PROBEFLOOD; ... */
                break;
            case 5: /* state = STATE_WIFI_PMKID; ... */
                break;
            default:
                break;
            }
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
    case STATE_WIFI_GRAPH:
        ScreenWifiGraph_draw();
        if (ScreenWifiGraph_handleInput(btn))
        {
            state = STATE_WIFI_MENU;
        }
        break;
    }
}