#include <BLEDevice.h>
#include <string>

#include "OrsClient.h"
#include "screen.h"
#include "connect.h"

using namespace Ohioh;

void InitScreen::draw(Adafruit_SSD1306 &display)
{
    display.clearDisplay();

    display.display();
}

StatusScreen::StatusScreen(OrsClient *client)
{
    _device = client;
}

void StatusScreen::draw(Adafruit_SSD1306 &display)
{
    display.clearDisplay();
    display.setCursor(0, 0);

    char str[1024];
    size_t length = sprintf(str, "Op-mode: %s\nStatus: %s\nIPv4: %s\nBluetooth: %s",
                            _device->getOpMode() == OpMode::SEND ? "SEND" : "RECEIVE",
                            ConnectionManager::clientTcp.connected() ? "Connected" : "Disconnected",
                            WiFi.localIP().toString().c_str(),
                            BLEDevice::getInitialized() ? "on" : "off");

    display.write(str, length);
    display.display();
}
