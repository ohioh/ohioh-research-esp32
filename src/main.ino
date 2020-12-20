/*   ___  _   _ ___ ___  _   _ 
 *  / _ \| | | |_ _/ _ \| | | |
 * | | | | |_| || | | | | |_| |
 * | |_| |  _  || | |_| |  _  |
 *  \___/|_| |_|___\___/|_| |_|
 * R   E   S   E   A   R   C   H
 */

#include <pthread.h>
#include <Wire.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
//#include <BLEAdvertisedDevice.h>
#include <Adafruit_SSD1306.h>

#include "net/packets.h"
#include "connect.h"
#include "screen.h"

#define SCREEN_WIDTH 128

#if defined(WIFI_LoRa_32_V2)
#define PIN_STATUS 34
#define PIN_OLED_RESET    16
#define SCREEN_HEIGHT 64
#else
#define PIN_STATUS 32
#define PIN_OLED_RESET    -1
#define SCREEN_HEIGHT 32
#endif


using namespace Ohioh;

const unsigned char ADDR_I2C_SSD1306{0x3C};
const unsigned char ADDR_I2C_GY521{0x68};

const uint16_t MTU{1500};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, PIN_OLED_RESET);
BLEScan *bleScan{nullptr};

Screen *currentScreen{nullptr};
WiFiClient *client{&ConnectionManager::clientTcp};

enum OpMode
{
    SEND,
    RECEIVE
} opMode;

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

    // Initialization steps specific to Heltec board
#if defined(WIFI_LoRa_32_V2)
    Wire.begin(4, 15);
#endif

    // Initalize SSD1306 display
    display.begin(SSD1306_SWITCHCAPVCC, ADDR_I2C_SSD1306);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Init operating mode
    uint8_t opModeCode = EEPROM.readByte(0x0);

    if (opModeCode != (uint8_t)OpMode::SEND || opModeCode != (uint8_t)OpMode::RECEIVE)
        opMode = OpMode::SEND;
    else
        opMode = (OpMode)opModeCode;

    char str[32];
    sprintf(str, "Operation-mode: %d\n", (uint8_t)opMode);
    Serial.write(str);

    bool connected{false};

    // Create initialization thread
    pthread_t initThread;

    if (pthread_create(&initThread, NULL, ConnectionManager::setupIpv4, &connected) == 0)
        Serial.println("Thread started!\n");
    else
        Serial.println("Thread failed to start!\n");

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

    currentScreen = new StatusScreen();

    // Create network loop thread
    TaskHandle_t *networkTask;
    xTaskCreate(networkLoop, "listen", 10000, NULL, 0, networkTask);
}

void loop()
{
    // Force reconnect when on-board button is pressed
    if (digitalRead(0) == 0)
        client->connect(ConnectionManager::host, ConnectionManager::port);

    // Set status LED
    //if (WiFi.isConnected() && (bool)client->connected() && BLEDevice::getInitialized())
    //digitalWrite(PIN_STATUS, HIGH);
    //else
    //digitalWrite(PIN_STATUS, LOW);

    currentScreen->draw(display);

    switch (opMode)
    {
    case OpMode::SEND:
        break;

    case OpMode::RECEIVE:
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
            uint8_t buffer[MTU] = {0};
            size_t bufferSize{2};

            buffer[0] = MsgId::SCAN_REPORT;
            buffer[1] = (uint8_t)results.getCount();

            for (int i{0}; i < results.getCount(); i++)
            {
                BLEAdvertisedDevice device{results.getDevice(i)};
                size_t payloadLength = device.getPayloadLength();

                // Write payload size
                memcpy(buffer + bufferSize, &payloadLength, sizeof(size_t));
                bufferSize += sizeof(size_t);

                memcpy(buffer + bufferSize, device.getPayload(), payloadLength);
                bufferSize += payloadLength;
            }

            client->write(buffer, bufferSize);
            client->flush();
        }

        bleScan->clearResults();
        break;
    }
}

void networkLoop(void *)
{
    // Receive packets from the ORS
    while (true)
    {
        if (client->available())
        {
            digitalWrite(PIN_STATUS, HIGH);

            uint8_t buffer[MTU] = {0};

            uint16_t bufSize = client->read(buffer, bufSize);

            char str[256];
            sprintf(str, "Received packet with ID \"%d\" with size of %d bytes\n", buffer[0], bufSize);
            Serial.write(str);

            switch ((MsgId)buffer[0])
            {
            case MsgId::SET_OPMODE:
                uint8_t opmo = buffer[1];

                char str[1024];
                sprintf(str, "Setting op-mode to \"%d\"...\n", opmo);
                Serial.write(str);

                if (opmo == (uint8_t)OpMode::SEND || opmo == (uint8_t)OpMode::RECEIVE)
                {
                    // Write new op-mode to flash
                    EEPROM.writeByte(0x0, opmo);

                    opMode == (OpMode)opmo;

                    // Respond with status report packet
                    // TO-DO

                    Serial.write("Success!\n");
                }
                else
                    Serial.write("Invalid op-mode!\n");
                break;
            }
        }
        else
        {
            digitalWrite(PIN_STATUS, LOW);
        }
    }
    vTaskDelete(NULL);
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