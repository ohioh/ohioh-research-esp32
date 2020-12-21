#pragma once

#include "OrsClient.h"
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
        StatusScreen(OrsClient *client);
        ~StatusScreen() {}
        void draw(Adafruit_SSD1306 &display) override;

    private:
        OrsClient *_device;
    };

    class InitScreen : public Screen
    {
    public:
        ~InitScreen() {}
        void draw(Adafruit_SSD1306 &display) override;
    };
} // namespace Ohioh