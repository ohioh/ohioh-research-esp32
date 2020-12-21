#include "OrsClient.h"
#include "pins.h"
#include "net/packets.h"

using namespace Ohioh;

void OrsClient::init()
{
    BLEDevice::init("ESP32-OHIOH");
    bleScan = BLEDevice::getScan();
    bleScan->setActiveScan(true);
    bleScan->setInterval(100);
    bleScan->setWindow(99);
}

void OrsClient::setOpMode(OpMode opMode)
{
    _opMode = opMode;
}

OpMode OrsClient::getOpMode()
{
    return _opMode;
}

uint8_t OrsClient::connect(IPAddress host, uint16_t port)
{
    client->connect(ConnectionManager::host, ConnectionManager::port);
}

void OrsClient::loop()
{
    // Set status LED
    //if (WiFi.isConnected() && (bool)client->connected() && BLEDevice::getInitialized())
    //digitalWrite(PIN_STATUS, HIGH);
    //else
    //digitalWrite(PIN_STATUS, LOW);

    switch (_opMode)
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
            uint8_t buffer[_mtu] = {0};
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

void OrsClient::listen()
{
    // Receive packets from the ORS
    while (true)
    {
        if (client->available())
        {
            // digitalWrite(PIN_STATUS, HIGH);

            uint8_t buffer[_mtu] = {0};

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

                    _opMode = ((OpMode)opmo);

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