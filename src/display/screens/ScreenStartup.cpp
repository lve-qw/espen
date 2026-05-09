#include "../Display.h"
#include "../assets/logo_xbm.h"

void ScreenStartup_draw()
{
    u8g2.clearBuffer();
    // centring: (128 - 93) / 2 = 17
    u8g2.drawXBMP(0, 0, LOGO_WIDTH, LOGO_HEIGHT, logo_start_screen_bits);
    u8g2.sendBuffer();
}

// 0 - active,
// 1 - non-active
uint8_t ScreenStartup_loop()
{
    static uint32_t start_time = 0;

    // timer init
    if (start_time == 0)
    {
        start_time = millis();
        ScreenStartup_draw();
    }

    // Проверка: прошло ли 3 секунды?
    if (millis() - start_time >= 10000)
    {
        start_time = 0;
        return 1;
    }

    return 0;
}

// Сброс состояния (если нужно показать экран повторно)
void ScreenStartup_reset()
{
    // Сброс статики через флаг или вынос состояния в структуру,
    // но для простого сплеш-скрина достаточно перезагрузки или игнорирования.
}