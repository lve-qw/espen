#include <Arduino.h>
#include "Display.h"
#include "ScreenStartup.h"

void setup()
{
    Serial.begin(115200);
    Display::init();

    // Сбрасываем логику старта
    // (тут нужен вызов функции сброса, если мы вынесли состояние)
}

void loop()
{
    // Простой конечный автомат (FSM)
    static enum { STARTUP,
                  MENU,
                  ACTIVE } state = STARTUP;

    switch (state)
    {
    case STARTUP:
        // Вызываем логику старта. Если вернул 1 -> переходим дальше
        if (ScreenStartup_loop() == 1)
        {
            state = MENU;
            // Здесь инициализация следующего экрана
        }
        break;

    case MENU:
        // Логика меню...
        Display::clear();
        u8g2.drawStr(0, 20, "Main Menu");
        u8g2.sendBuffer();
        delay(2000);
        state = ACTIVE;
        break;

    case ACTIVE:
        // Основной цикл пентест-инструмента
        break;
    }
}