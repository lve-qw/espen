#include "display/Display.h"
#include <WiFi.h>
#include "input/Input.h"
#include "ScreenWifiScan.h"

enum ScanState
{
    SC_IDLE,
    SC_SCANNING,
    SC_DONE,
    SC_ERROR
};
static ScanState state = SC_IDLE;
static int16_t selected = 0;
static int16_t scroll = 0;
static const uint8_t VISIBLE_ITEMS = 2; // Влезает на 128x64 с комфортным отступом
static const uint8_t LINE_H = 10;       // Высота строки шрифта
static const uint8_t START_Y = 14;

// Преобразование типа шифрования в строку
static const char *getEncStr(int auth)
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
    case 6:
        return "W2/W3";
    default:
        return "UNK";
    }
}

void ScreenWifiScan_init()
{
    state = SC_SCANNING;
    selected = 0;
    scroll = 0;
    WiFi.mode(WIFI_STA);
    WiFi.scanNetworks(true, true);
}

void ScreenWifiScan_draw()
{
    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setFont(u8g2_font_6x10_tr);

    if (state == SC_SCANNING)
    {
        u8g2.drawStr(30, 30, "Scanning");

        static uint32_t last_t = millis();
        static uint8_t dots = 0;
        if (millis() - last_t > 300)
        {
            dots = (dots + 1) % 4;
            last_t = millis();
        }
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.drawStr(80, 30, dots > 0 ? "." : " ");
        u8g2.drawStr(85, 30, dots > 1 ? "." : " ");
        u8g2.drawStr(90, 30, dots > 2 ? "." : " ");
    }
    else if (state == SC_DONE)
    {
        int count = WiFi.scanComplete();
        if (count <= 0)
        {
            u8g2.drawStr(35, 30, "No APs found");
        }
        else
        {
            u8g2.setFont(u8g2_font_5x7_tr); // Ещё плотнее для данных

            for (uint8_t i = 0; i < VISIBLE_ITEMS; i++)
            {
                int idx = scroll + i;
                if (idx >= count)
                    break;

                int y = START_Y + i * (LINE_H * 3 + 2);
                int x = 2;
                if (idx == selected)
                {
                    u8g2.drawStr(x, y, ">");
                    x += 6;
                }

                String ssid = WiFi.SSID(idx);
                if (ssid.length() > 15)
                    ssid = ssid.substring(0, 14) + "~";

                int rssi = WiFi.RSSI(idx);
                int ch = WiFi.channel(idx);
                int auth = WiFi.encryptionType(idx);
                const uint8_t *bssid = WiFi.BSSID(idx);

                char l1[24], l2[24], l3[24];
                snprintf(l1, sizeof(l1), "%s", ssid.c_str());
                snprintf(l2, sizeof(l2), "%3d dBm CH%02d %s", rssi, ch, getEncStr(auth));
                snprintf(l3, sizeof(l3), "MAC:..:%02X:%02X:%02X", bssid[3], bssid[4], bssid[5]);

                u8g2.drawStr(x, y, l1);
                u8g2.drawStr(x, y + LINE_H, l2);
                u8g2.drawStr(x, y + LINE_H * 2, l3);
            }
        }
    }
    else if (state == SC_ERROR)
    {
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(25, 30, "Scan Failed");
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
            state = SC_DONE;
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

    if (state == SC_DONE)
    {
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
                if (selected >= scroll + VISIBLE_ITEMS)
                    scroll++;
            }
            break;
        case BTN_LEFT:
            WiFi.scanDelete(); // Освобождаем буфер сканирования
            return true;       // Сигнал "Назад"
        default:
            break;
        }
    }
    else
    {
        if (btn == BTN_LEFT)
        {
            WiFi.scanDelete();
            return true;
        }
    }
    return false;
}