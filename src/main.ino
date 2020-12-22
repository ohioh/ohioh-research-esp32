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
#include <Adafruit_SSD1306.h>

#include "pins.h"
#include "OrsClient.h"
#include "connect.h"
#include "screen.h"

#define SCREEN_WIDTH 128
#if defined(WIFI_LoRa_32_V2)
#define SCREEN_HEIGHT 64
#else
#define SCREEN_HEIGHT 32
#endif

using namespace Ohioh;

const unsigned char ADDR_I2C_SSD1306{0x3C};
const unsigned char ADDR_I2C_GY521{0x68};

const uint16_t MTU{1500};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, PIN_OLED_RESET);

Screen *currentScreen{nullptr};

static OrsClient orsClient;

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

    EEPROM.begin(1);

    // Initialization steps specific to Heltec board
#if defined(WIFI_LoRa_32_V2)
    Wire.begin(4, 15);
#endif

    // Initalize SSD1306 display
    display.begin(SSD1306_SWITCHCAPVCC, ADDR_I2C_SSD1306);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    orsClient.init();

    // Read operating mode
    uint8_t opModeCode = EEPROM.readByte(0x0);

    if (opModeCode == (uint8_t)OpMode::SEND || opModeCode == (uint8_t)OpMode::RECEIVE)
    {
        orsClient.setOpMode((OpMode)opModeCode);
    }
    else
    {
        EEPROM.writeByte(0x0, (uint8_t)OpMode::SEND);
        EEPROM.commit();

        orsClient.setOpMode(OpMode::SEND);
    }

    char str[32];
    sprintf(str, "Operation-mode: %d\n", opModeCode);
    Serial.write(str);

    bool connected{false};

    // Create initialization thread
    //xTaskCreate([](void *param) { ConnectionManager::setupIpv4(NULL); }, "init_ipv4", 10000, NULL, 0, NULL);

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

    currentScreen = new StatusScreen(&orsClient);

    // Create thread for client-side listening
    xTaskCreate([](void *client) { ((OrsClient *)client)->listen(); }, "listen", 10000, &orsClient, 0, NULL);
}

void loop()
{
    // Force reconnect when on-board button is pressed
    if (digitalRead(0) == 0)
        orsClient.connect(ConnectionManager::host, ConnectionManager::port);

    orsClient.loop();

    currentScreen->draw(display);
}
