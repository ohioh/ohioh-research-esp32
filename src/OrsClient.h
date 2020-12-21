#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <WiFi.h>

#include "connect.h"

namespace Ohioh
{
    enum OpMode
    {
        SEND,
        RECEIVE
    };

    class OrsClient
    {
    public:
        void init();
        uint8_t connect(IPAddress host, uint16_t port);
        void loop();
        void listen();
        void setOpMode(OpMode opMode);
        OpMode getOpMode();
        void getMTU();

    private:
        uint16_t _mtu;
        OpMode _opMode;
        BLEScan *bleScan{nullptr};
        WiFiClient *client{&ConnectionManager::clientTcp};
    };
} // namespace Ohioh