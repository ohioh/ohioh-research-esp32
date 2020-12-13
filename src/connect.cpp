#include "connect.h"

namespace OHIOH
{
    const char *ConnectionManager::SSID = "";
    const char *ConnectionManager::PASSWORD = "";

    IPAddress ConnectionManager::host(0, 0, 0, 0);

    WiFiClient ConnectionManager::clientTcp;

    void *ConnectionManager::setupIpv4(void *a)
    {
#if defined(WIFI)
        WiFi.mode(WIFI_MODE_STA);
        WiFi.begin(SSID, PASSWORD);

        while (WiFi.status() != WL_CONNECTED)
            delay(500);
#else
        EthernetClass::init();
#endif

        clientTcp.setNoDelay(true);
        clientTcp.connect(host, port);

        // delay(100);
        *(bool *)a = true;
    }

    void *ConnectionManager::setupBluetooth(void *a)
    {
    }
} // namespace OHIOH