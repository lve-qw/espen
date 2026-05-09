#include "Display.h"
#include <Wire.h>
#include "pins.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

namespace Display
{
    void init()
    {
        Wire.begin(OLED_SDA, OLED_SCL);
        u8g2.begin();
        u8g2.setFontMode(1);
        u8g2.setBitmapMode(1);
        u8g2.setFont(u8g2_font_ncenB08_tr);
    }

    void clear()
    {
        u8g2.clearBuffer();
        u8g2.sendBuffer();
    }
}