/*   ___  _   _ ___ ___  _   _ 
 *  / _ \| | | |_ _/ _ \| | | |
 * | | | | |_| || | | | | |_| |
 * | |_| |  _  || | |_| |  _  |
 *  \___/|_| |_|___\___/|_| |_|
 * R   E   S   E   A   R   C   H
 */

#include <pthread.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
//#include <BLEAdvertisedDevice.h>
#include <Adafruit_SSD1306.h>

#include "connect.h"
#include "screen.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

const unsigned char ADDR_I2C_SSD1306{0x3C};
const unsigned char ADDR_I2C_GY521{0x68};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);
BLEScan *bleScan{nullptr};

OHIOH::Screen *currentScreen{nullptr};
WiFiClient *client {&OHIOH::ConnectionManager::clientTcp};

#define PIN_STATUS 32

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;

    // Initialize pins
    // analogReadResolution(8);
    pinMode(PIN_STATUS, OUTPUT);
    pinMode(0, INPUT);
    pinMode(33, INPUT);

    // Initalize SSD1306 display
    display.begin(SSD1306_SWITCHCAPVCC, ADDR_I2C_SSD1306);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Initialize GY-521
    Wire.begin();
    Wire.beginTransmission(ADDR_I2C_GY521);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission();

    bool connected{false};

    // Create initialization thread
    pthread_t initThread;

    if (pthread_create(&initThread, NULL, OHIOH::ConnectionManager::setupIpv4, &connected) == 0)
        Serial.println("Thread started!");
    else
        Serial.println("Thread failed to start!");

    u_char nDots{0};

    while (!connected)
    {
        display.clearDisplay();
        display.setCursor(0, 0);

        display.write("\n");

        char str[] = "Initializing:\n";
        writeHCentered(str);
        writeHCentered("WiFi\n");

        snprintf(str, nDots + 1, "...");
        writeHCentered(str);

        digitalWrite(PIN_STATUS, HIGH);
        delay(250);
        digitalWrite(PIN_STATUS, LOW);
        delay(250);

        display.display();

        if (nDots < 3)
            nDots++;
        else
            nDots = 1;
    }

    BLEDevice::init("ESP32-OHIOH");
    bleScan = BLEDevice::getScan();
    bleScan->setActiveScan(true);
    bleScan->setInterval(100);
    bleScan->setWindow(99);

    currentScreen = new OHIOH::StatusScreen();
}

void loop()
{
    if (digitalRead(0) == 0)
        client->connect(OHIOH::ConnectionManager::host, OHIOH::ConnectionManager::port);

    // Set status LED
    if (WiFi.isConnected() && (bool)client->connected() && BLEDevice::getInitialized())
        digitalWrite(PIN_STATUS, HIGH);
    else
        digitalWrite(PIN_STATUS, LOW);

    currentScreen->draw(display);

    BLEScanResults results = bleScan->start(5, false);

    if (results.getCount() > 0)
    {
        Serial.println("Devices found:");

        for (int i{0}; i < results.getCount(); i++)
        {
            char str[1024];
            sprintf(str, "%d %s", i + 1, results.getDevice(i).toString().c_str());
            Serial.println(str);
            //Serial.println(results.getDevice(i).getPayloadLength());
        }
    }
    else
    {
        Serial.println("No devices found...");
    }

    // Netcode
    if (results.getCount() > 0)
    {
        uint8_t *buffer = new uint8_t[1024]();
        size_t bufferSize {2};

        buffer[0] = (uint8_t)0xA; // 0x1: Scanned BLE devices report
        buffer[1] = (uint8_t)results.getCount();

        for (int i {0}; i < results.getCount(); i++)
        {
            BLEAdvertisedDevice device {results.getDevice(i)};
            size_t payloadLength = device.getPayloadLength();

            // Write payload size
            memcpy(buffer + bufferSize, &payloadLength, sizeof(size_t));
            bufferSize += sizeof(size_t);

            memcpy(buffer + bufferSize, device.getPayload(), payloadLength);
            bufferSize += payloadLength;
        }
        
        client->write(buffer, bufferSize);
        client->flush();

        delete [] buffer;
    }

    bleScan->clearResults();
}

void writeHCentered(const char *str)
{
    uint16_t *x = new uint16_t{0};
    uint16_t *y = new uint16_t{0};
    int16_t *w = new int16_t{0};
    int16_t *h = new int16_t{0};

    display.getTextBounds(str, display.getCursorX(), display.getCursorY(), w, h, x, y);

    display.setCursor(display.width() / 2 - ((int16_t)*x / 2), display.getCursorY());
    display.write(str);

    delete x, y, w, h;
}