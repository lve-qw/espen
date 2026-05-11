#include "display/Display.h"
#include "input/Input.h"
#include "ScreenWifiMenu.h"

static const char *items[] = {"Scan AP", "Graph", "Deauth", "BeFlood", "ProbeFlood", "PMKID"};
static const uint8_t ITEM_COUNT = 6;
static uint8_t currentIndex = 0;
static bool selectTriggered = false;

void ScreenWifiMenu_init()
{
    currentIndex = 0;
    selectTriggered = false;
}

void ScreenWifiMenu_draw()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setFontMode(1);

    const uint8_t LINE_H = 14, START_Y = 13, TEXT_X = 2, CUR = 9;
    for (uint8_t i = 0; i < ITEM_COUNT; i++)
    {
        int y = START_Y + i * LINE_H;
        int x = TEXT_X;
        bool sel = (i == currentIndex);
        if (sel)
        {
            u8g2.drawStr(x, y, ">");
            x += CUR;
        }
        u8g2.drawStr(x, y, items[i]);
    }
    u8g2.sendBuffer();
}

void ScreenWifiMenu_prev() { currentIndex = (currentIndex == 0) ? ITEM_COUNT - 1 : currentIndex - 1; }
void ScreenWifiMenu_next() { currentIndex = (currentIndex == ITEM_COUNT - 1) ? 0 : currentIndex + 1; }

bool ScreenWifiMenu_wasSelected()
{
    if (selectTriggered)
    {
        selectTriggered = false;
        return true;
    }
    return false;
}
uint8_t ScreenWifiMenu_getSelectedIndex() { return currentIndex; }

bool ScreenWifiMenu_handleInput(ButtonAction btn)
{
    if (btn == BTN_UP)
        ScreenWifiMenu_prev();
    if (btn == BTN_DOWN)
        ScreenWifiMenu_next();
    if (btn == BTN_SELECT)
        selectTriggered = true;
    if (btn == BTN_LEFT)
        return true;
    return false;
}