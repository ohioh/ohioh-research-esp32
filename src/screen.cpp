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
    uint8_t val = analogRead(33);
    display.write("Op-mode: "); display.write(_device->getOpMode() == OpMode::SEND ? "SEND" : "RECEIVE");
    display.write("\nStatus: ");
    display.write((ConnectionManager::clientTcp.connected() ? "Connected" : "Disconnected"));
    display.write("\nIPv4:   ");

    // Print IPv4 address
    for (int i{0}; i < 4; i++)
    {
        char buf[3]{0};
        itoa(WiFi.localIP()[i], buf, 10);
        display.write(buf);

        if (i < 3)
            display.write('.');
    }

    bool t = (BLEDevice::getInitialized() ? true : false);

    //char buf [1024];
    //size_t len = sprintf(buf, "Bluetooth: %s", (BLEDevice::getInitialized() ? "on" : "off"));
    //display.write(buf, len);

    display.write((std::string("\nBluetooth: " ) + (BLEDevice::getInitialized() ? "on" : "off")).c_str());

    display.display();
}
