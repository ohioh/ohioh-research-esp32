<div align="center">
<pre>
  ___  _   _ ___ ___  _   _ 
 / _ \| | | |_ _/ _ \| | | |
| | | | |_| || | | | | |_| |
| |_| |  _  || | |_| |  _  |
 \___/|_| |_|___\___/|_| |_|
R   E   S   E   A   R   C   H
</pre>
<hr>
</div>

This repository holds the source code for the ESP32 micro-controller dedicated to the OHIOH research project.

## Concept
An ESP32 module executing this code will continously exchange Bluetooth data with nearby ESP32's of the same kind and will transmit this information host running the [OHIOH Research server](https://github.com/ohioh/ohioh-research-server).\
The collected data is then used to determine the optimal conditions for Bluetooth data exchange.

## Prerequisites & layout
#### Libraries
- [Ethernet](https://github.com/arduino-libraries/Ethernet)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)

#### Required hardware
##### Recommended:
- Heltec ESP32 LoRa board
- W5500 Ethernet module
##### Alternative:
- ESP32 devboard
- W5500 Ethernet module (*WiFi as fallback available*)
- SSD1306 OLED display

#### Layout
(*insert breadport schematic here...*)

For more information on how to set up a development environment as well as compiling and uploading the code, please see the [getting started guide](). A documentation of the network packet protocol can be examined [here]().