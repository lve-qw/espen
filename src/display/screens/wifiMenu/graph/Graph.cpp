#include "display/Display.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include "input/Input.h"
#include "Graph.h"

static const uint8_t CHANNEL_MIN = 1;
static const uint8_t CHANNEL_MAX = 13;    // 2.4 ГГц, 13 каналов в РФ/Европе
static const uint8_t GRAPH_TOP_Y = 12;    // Верхняя граница графика
static const uint8_t GRAPH_BOTTOM_Y = 58; // Нижняя граница
static const uint8_t GRAPH_HEIGHT = GRAPH_BOTTOM_Y - GRAPH_TOP_Y;
static const int16_t RSSI_MIN = -100; // Мин. уровень сигнала (низ графика)
static const int16_t RSSI_MAX = -30;  // Макс. уровень (верх графика)

static uint8_t currentChannel = CHANNEL_MIN;
static int16_t rssiBuffer[14] = {0}; // Индекс 1..13, 0 не используется
static uint32_t lastHopTime = 0;
static const uint16_t HOP_INTERVAL_MS = 150; // Время на канале перед переключением
static bool scanning = true;

static int16_t rssiToY(int16_t rssi)
{
    if (rssi <= RSSI_MIN)
        return GRAPH_BOTTOM_Y;
    if (rssi >= RSSI_MAX)
        return GRAPH_TOP_Y;
    // Линейная интерполяция
    return GRAPH_BOTTOM_Y - (int16_t)((float)(rssi - RSSI_MIN) / (RSSI_MAX - RSSI_MIN) * GRAPH_HEIGHT);
}

// Обновление буфера: сглаживание (экспоненциальное)
static void updateRssi(uint8_t ch, int16_t newRssi)
{
    if (ch < CHANNEL_MIN || ch > CHANNEL_MAX)
        return;
    // Смешиваем новое значение с предыдущим (коэффициент 0.7)
    rssiBuffer[ch] = (int16_t)(rssiBuffer[ch] * 0.7 + newRssi * 0.3);
}

// ── API ───────────────────────────────────────────
void ScreenWifiGraph_init()
{

    // Сброс состояния
    for (uint8_t i = 0; i < 14; i++)
        rssiBuffer[i] = RSSI_MIN;
    currentChannel = CHANNEL_MIN;
    lastHopTime = millis();
    scanning = true;

    // Настройка Wi-Fi для пассивного сканирования
    WiFi.mode(WIFI_STA);
    delay(50);

    // Включаем promiscuous mode для приёма всех кадров
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(nullptr); // Callback не нужен, используем scan
}

void ScreenWifiGraph_draw()
{
    u8g2.clearBuffer();
    u8g2.setFontMode(1);

    // ── Ось каналов (подпись) ─────────────────────
    u8g2.setFont(u8g2_font_4x6_tr);
    for (uint8_t ch = CHANNEL_MIN; ch <= CHANNEL_MAX; ch += 3)
    {
        int x = 2 + (ch - CHANNEL_MIN) * 9; // ~9px на канал
        char buf[3];
        snprintf(buf, sizeof(buf), "%d", ch);
        u8g2.drawStr(x, 63, buf);
    }

    // ── График: столбцы ───────────────────────────
    u8g2.setDrawColor(1);
    for (uint8_t ch = CHANNEL_MIN; ch <= CHANNEL_MAX; ch++)
    {
        int16_t rssi = rssiBuffer[ch];
        if (rssi <= RSSI_MIN)
            continue; // Нет данных — не рисуем

        int x = 2 + (ch - CHANNEL_MIN) * 9;
        int y_top = rssiToY(rssi);
        int y_bottom = GRAPH_BOTTOM_Y;
        int h = y_bottom - y_top;

        // Подсветка текущего канала
        if (ch == currentChannel)
        {
            u8g2.setDrawColor(2); // XOR режим для инверсии
            u8g2.drawBox(x, y_top, 7, h);
            u8g2.setDrawColor(1);
        }
        else
        {
            u8g2.drawBox(x, y_top, 7, h);
        }
    }

    u8g2.setFont(u8g2_font_5x7_tr);
    char legend[20];

    // Найти канал с лучшим сигналом
    int16_t bestRssi = RSSI_MIN;
    uint8_t bestCh = 0;
    for (uint8_t ch = CHANNEL_MIN; ch <= CHANNEL_MAX; ch++)
    {
        if (rssiBuffer[ch] > bestRssi)
        {
            bestRssi = rssiBuffer[ch];
            bestCh = ch;
        }
    }

    if (bestCh > 0)
    {
        snprintf(legend, sizeof(legend), "CH%d:%ddBm", bestCh, bestRssi);
        u8g2.drawStr(2, 6, legend);
    }

    // Индикатор сканирования
    if (scanning)
    {
        static uint32_t t = millis();
        static uint8_t d = 0;
        if (millis() - t > 200)
        {
            d = (d + 1) % 4;
            t = millis();
        }
        u8g2.drawStr(110, 2, d > 0 ? "." : " ");
        u8g2.drawStr(115, 2, d > 1 ? "." : " ");
        u8g2.drawStr(120, 2, d > 2 ? "." : " ");
    }

    u8g2.sendBuffer();
}

bool ScreenWifiGraph_handleInput(ButtonAction btn)
{
    if (btn == BTN_LEFT)
    {
        esp_wifi_set_promiscuous(false);
        WiFi.mode(WIFI_STA);
        return true;
    }

    if (btn == BTN_SELECT)
    {
        scanning = !scanning;
        return false;
    }

    if (!scanning)
        return false;

    if (millis() - lastHopTime >= HOP_INTERVAL_MS)
    {
        lastHopTime = millis();

        // Параметры: async=false, show_hidden=true, passive=true, time=80ms, channel=X
        int n = WiFi.scanNetworks(false, true, false, 80, currentChannel);

        if (n > 0)
        {
            int16_t sum = 0;
            for (int i = 0; i < n; i++)
                sum += WiFi.RSSI(i);
            updateRssi(currentChannel, sum / n);
        }

        // Следующий канал
        currentChannel++;
        if (currentChannel > CHANNEL_MAX)
            currentChannel = CHANNEL_MIN;

        esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    }

    return false;
}