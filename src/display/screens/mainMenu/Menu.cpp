#include "display/Display.h"
#include "display/assets/icons.h"
#include "input/Input.h"
#include "Menu.h"

struct MenuItem
{
    const char *label;
    const unsigned char *icon;
    uint8_t w, h;
};

static const MenuItem items[] = {
    {"Settings", image_settings_bits, MENU_WIDTH, MENU_HEIGHT},             // 0
    {"Files", image_files_bits, FILES_WIDTH, FILES_HEIGHT},                 // 1
    {"Bluetooth", image_bluetooth_bits, BLUETOOTH_WIDTH, BLUETOOTH_HEIGHT}, // 2
    {"Wi-Fi", image_wifi_bits, WIFI_WIDTH, WIFI_HEIGHT}                     // 3
};
static const uint8_t ITEM_COUNT = sizeof(items) / sizeof(items[0]);
static uint8_t currentIndex = 0;
static bool selectTriggered = false;

void ScreenMenu_init()
{
    currentIndex = 0;
    selectTriggered = false;
}

void ScreenMenu_draw()
{

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_timR12_tr);
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);

    const uint8_t LINE_HEIGHT = 32;
    const uint8_t START_Y = 13;
    const uint8_t TEXT_BASE_X = 2;
    const uint8_t CURSOR_SHIFT = 9;

    for (uint8_t i = 0; i < ITEM_COUNT; i++)
    {
        bool selected = (i == currentIndex);
        int y = START_Y + i * LINE_HEIGHT;
        int textX = TEXT_BASE_X;

        if (selected)
        {
            u8g2.drawStr(textX, y, ">");
            textX += CURSOR_SHIFT;
        }
        u8g2.drawStr(textX, y, items[i].label);

        uint16_t textWidth = u8g2.getStrWidth(items[i].label);
        int iconX = textX + textWidth + 4;
        int iconY = y - items[i].h + 2;
        u8g2.drawXBMP(iconX, iconY, items[i].w, items[i].h, items[i].icon);
    }
    u8g2.sendBuffer();
}

void ScreenMenu_prev()
{
    currentIndex = (currentIndex == 0) ? ITEM_COUNT - 1 : currentIndex - 1;
}

void ScreenMenu_next()
{
    currentIndex = (currentIndex == ITEM_COUNT - 1) ? 0 : currentIndex + 1;
}

bool ScreenMenu_wasSelected()
{
    if (selectTriggered)
    {
        selectTriggered = false;
        return true;
    }
    return false;
}

uint8_t ScreenMenu_getSelectedIndex()
{
    return currentIndex;
}