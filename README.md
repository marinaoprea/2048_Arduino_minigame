# 2048 on LCD Minigame

## Introduction
- This is a proposed implementation for the classic 2048 game using Arduino Uno and peripheral devices.

- **Purpose:**
  - For developers: Exercising basic concepts of programming microcontrollers.
  - For users: Entertainment.

- **Motivation:**
  - 2048 is my favorite arcade game as it needs the right combination of thinking and luck to make it entertaining enough to be relaxing while developing basic strategies.

- **Game functionalities:**
  - Classical 2048 game with score tracking, high score tracking, and GoBack option.

## Overview
The game will start with a simple hello menu. Then, the grid and score will be displayed on the LCD screen.

**User input:**
- Swipe on the touchscreen.
- Joystick movement.
- Joystick button push.

Any functionality of the game will be implemented both ways, either by interpreting screen touch or joystick input.

**Modules used:**
- Arduino Uno
- 2.8-inch, 320x240 LCD TFT touchscreen
- Dual-axis analog joystick

Components will be connected as shown in the block diagram:

<img src="./images/pm_block_scheme.png" style="height 400px; width: 780px;"/>

## Hardware Design

### Electrical Scheme
<img src="./images/circuit_image.png" style="height 400px; width: 780px;"/>

The screen uses 3.3V for logical high while Arduino UNO uses 5V as pin output. Voltage dividers were added to prevent screen damage.

To obtain a good ratio, two pairs of resistors were used:
- 560Ω - 1kΩ
- 4.1kΩ - 10kΩ

### Pin Configurations

#### TFT Screen - Arduino
The Arduino SPI interface is shared between the display and touchscreen.

- **Power & Control Pins**
  - VCC - 5V
  - GND - GND
  - LED - 3.3V
  - CS - D8
  - RESET - D9

- **SPI Pins**
  - MOSI - D11 (shared)
  - SCK - D13 (shared)
  - MISO - D12 (shared, optional)

- **Touchscreen Pins**
  - T_CS - D7
  - T_CLK - D13 (shared)
  - T_DIN - D11 (shared)
  - T_DO - D12 (shared)
  - T_IRQ - D13

#### Dual-axis Joystick Module - Arduino
Only the left joystick is used, so only a few pins are connected.

- **Connections**
  - VCC - 5V
  - A0 (VRx) - A0
  - A1 (VRy) - A1
  - D2 (SW) - D2

### Bill of Materials
| Component        | Shop Link | Datasheet Link |
|-----------------|-----------|---------------|
| **TFT LCD 2.8"** | [eMAG Product Page](https://www.emag.ro/display-tactil-tft-lcd-2-8-inch-320x240-touchscreen-spi-driver-ili9341-arduino-emg561/pd/DSFJ88YBM/) | [Datasheet & Info](http://www.lcdwiki.com/2.8inch_SPI_Module_ILI9341_SKU:MSP2807) |
| **Arduino Uno** | [eMAG Product Page](https://www.emag.ro/placa-dezvoltare-arduino-uno-r3-compatibil-arduino-ide-atmega328-microcontroler-16-intrari-analogice-distanta-dintre-pini-2-54mm-i7/pd/DNS9R1YBM/) | [Datasheet](https://www.alldatasheet.com/datasheet-pdf/view/1943445/ARDUINO/ARDUINO-UNO.html) |
| **Joystick Module** | [eMAG Product Page](https://www.emag.ro/modul-joystick-compatibil-cu-arduino-uno-3874784221527/pd/D48GB9YBM/?ref=hdr-favorite_products) | [Datasheet](https://components101.com/modules/joystick-module) |
| **Resistors** | [eMAG Product Page](https://www.emag.ro/set-600pcs-rezistente-0-25w-30-tipuri-toleranta-1-20-de-bucati-pentru-fiecare-valoare-ideal-pentru-proiecte-electronice-si-ingineri-include-rezistente-10-100-1k-10k-100k-pana-la-1m-utilizat-pentru-pro/pd/DBV351YBM/) | [Datasheet](https://assets.rs-online.com/v1699613067/Datasheets/7ec977c91977fd4e95a020bd86d6d6c5.pdf) |

## Software Design

## Results

## Conclusions

## Timeline

## Bibliography
