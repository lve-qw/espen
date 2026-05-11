#include "display/Display.h"
#include <WiFi.h>
#include "input/Input.h"
#include "ScreenWifiScan.h"

// Простые состояния: ждём → готово → ошибка
enum ScanState
{
    SC_SCANNING,
    SC_READY,
    SC_ERROR
};
static ScanState state = SC_SCANNING;

// Навигация
static int16_t selected = 0;
static int16_t scroll = 0;

// Настройки отображения
static const uint8_t ROWS = 4;   // 4 строки на экран 128×64
static const uint8_t ROW_H = 14; // Высота строки
static const uint8_t TOP_Y = 12; // Отступ сверху

// Шифрование → текст
static const char *encStr(int auth)
{
    switch (auth)
    {
    case 0:
        return "OPEN";
    case 1:
        return "WEP";
    case 2:
        return "WPA";
    case 3:
        return "WPA2";
    case 4:
        return "WPA/2";
    case 5:
        return "WPA3";
    default:
        return "UNK";
    }
}

// Инициализация: запускаем асинхронный скан
void ScreenWifiScan_init()
{
    state = SC_SCANNING;
    selected = 0;
    scroll = 0;

    WiFi.mode(WIFI_STA);
    delay(50);
    WiFi.scanNetworks(true, true); // async=true, hidden=true
}

void ScreenWifiScan_draw()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setFontMode(1);

    if (state == SC_SCANNING)
    {

        u8g2.drawStr(35, 30, "Scanning");
        static uint32_t t = millis();
        static uint8_t d = 0;
        if (millis() - t > 250)
        {
            d = (d + 1) % 4;
            t = millis();
        }
        if (d > 0)
            u8g2.drawStr(85, 30, ".");
        if (d > 1)
            u8g2.drawStr(90, 30, ".");
        if (d > 2)
            u8g2.drawStr(95, 30, ".");
        u8g2.sendBuffer();
        return;
    }

    if (state == SC_ERROR)
    {
        u8g2.drawStr(30, 30, "Scan Failed");
        u8g2.sendBuffer();
        return;
    }

    int count = WiFi.scanComplete();
    if (count <= 0)
    {
        u8g2.drawStr(40, 30, count == 0 ? "No APs" : "Error");
        u8g2.sendBuffer();
        return;
    }

    for (uint8_t i = 0; i < ROWS; i++)
    {
        int idx = scroll + i;
        if (idx >= count)
            break;

        int y = TOP_Y + i * ROW_H;
        int x = 2;

        // Курсор
        if (idx == selected)
        {
            u8g2.drawStr(x, y, ">");
            x += 6;
        }

        //"SSID | ENC | xx"
        String ssid = WiFi.SSID(idx);
        if (ssid.length() > 11)
            ssid = ssid.substring(0, 10) + "~";

        char line[32];
        snprintf(line, sizeof(line), "%-11s|%-5s|%02d",
                 ssid.c_str(),
                 encStr(WiFi.encryptionType(idx)),
                 WiFi.channel(idx));

        u8g2.drawStr(x, y, line);
    }
    u8g2.sendBuffer();
}

bool ScreenWifiScan_handleInput()
{
    if (state == SC_SCANNING)
    {
        int res = WiFi.scanComplete();
        if (res >= 0)
        {
            state = SC_READY;
            selected = 0;
            scroll = 0;
        }
        else if (res == WIFI_SCAN_FAILED)
        {
            state = SC_ERROR;
        }
        return false;
    }

    ButtonAction btn = getButtonAction();
    if (btn == BTN_NONE)
        return false;

    int count = WiFi.scanComplete();

    switch (btn)
    {
    case BTN_UP:
        if (selected > 0)
        {
            selected--;
            if (selected < scroll)
                scroll--;
        }
        break;
    case BTN_DOWN:
        if (selected < count - 1)
        {
            selected++;
            if (selected >= scroll + ROWS)
                scroll++;
        }
        break;
    case BTN_LEFT:
        WiFi.scanDelete();
        return true;
    default:
        break;
    }
    return false;
}