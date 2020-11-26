<div align="center">
    <h2>OHIOH Research</h2>
    <h4>ESP32 micro-controller code</h4>
    <hr>
</div>

This repository holds the source code for the ESP32 micro-controller dedicated to the OHIOH research project.

## Concept
An ESP32 module executing this code will continously exchange Bluetooth data with nearby devices (preferably ESP32's) and will transmit this information alongside it's position using the GY-521 3-axis gyroscope sensor to a host running the [OHIOH Research server](https://github.com/ohioh/ohioh-research-server).\
The collected data is then used to determine the optimal conditions for Bluetooth data exchange.

## Prerequisites & layout
#### Libraries
- [Ethernet](https://github.com/arduino-libraries/Ethernet)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)

#### Required hardware
- ESP32 module
- GY-521 3-axis gyroscope sensor
- *Optional:* USR-S1 Ethernet module

#### Layout
(*insert breadport schematic here...*)