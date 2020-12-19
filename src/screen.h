#pragma once

#include <WiFi.h>
#include <Adafruit_SSD1306.h>

namespace Ohioh
{
    class Screen
    {
    public:
        virtual ~Screen() {}
        virtual void draw(Adafruit_SSD1306 &display) = 0;
    };

    class StatusScreen : public Screen
    {
    public:
        ~StatusScreen() {}
        void draw(Adafruit_SSD1306 &display) override;
    };

    class InitScreen : public Screen
    {
    public:
        ~InitScreen() {}
        void draw(Adafruit_SSD1306 &display) override;
    };
} // namespace OHIOH