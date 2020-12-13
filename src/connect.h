#pragma once

#define WIFI

#ifdef WIFI
#include <WiFi.h>
#else
#include <Ethernet.h>
#endif

namespace OHIOH
{
    class ConnectionManager
    {
    public:
        static const char *SSID;
        static const char *PASSWORD;

        static IPAddress host;
        const static uint16_t port{5527};

        static WiFiClient clientTcp;

        static void *setupIpv4(void *a);
        static void *setupBluetooth(void *a);
    };
} // namespace OHIOH