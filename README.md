
# Grove Gas Telemetry

STM32MP157-DK1 + Grove Multichannel Gas Sensor V2 + XIAO ESP32-C3 + ESP-NOW

## Overview

This repository contains the implementation and engineering documentation for a gas-sensor telemetry milestone.

The system reads gas-sensor data from a Grove Multichannel Gas Sensor V2 using the Linux side of an STM32MP157-DK1, transmits the readings over UART7 to an XIAO ESP32-C3, forwards the data using ESP-NOW, and displays the decoded readings on a second XIAO ESP32-C3.

## System Architecture

```text
Grove Multichannel Gas Sensor V2
              |
              | I2C
              v
      STM32MP157-DK1
              |
              | UART7
              | 115200 8N1
              v
          ESP32-A
       UART -> ESP-NOW
              |
              | ESP-NOW
              v
          ESP32-B
       Parser + Display
```

## Hardware

### STM32

* STM32MP157D-DK1 Discovery Board
* OpenSTLinux
* Linux I2C5
* Linux device: `/dev/i2c-1`
* UART7
* Linux UART device: `/dev/ttySTM2`

### Gas Sensor

* Seeed Grove Multichannel Gas Sensor V2
* I2C address: `0x08`

### ESP32-A

* Seeed Studio XIAO ESP32-C3
* Role: UART → ESP-NOW bridge
* RX: GPIO20
* TX: GPIO21
* MAC: `94:A9:90:6A:22:60`

### ESP32-B

* Seeed Studio XIAO ESP32-C3
* Role: ESP-NOW receiver and gas-data display
* MAC: `58:8C:81:A9:E2:18`

## Gas Channels

The current milestone transmits three sensor channels:

| Sensor  | Gas     | Command | Telemetry field |
| ------- | ------- | ------: | --------------- |
| GM-102B | NO₂     |  `0x01` | `NO2`           |
| GM-302B | Ethanol |  `0x03` | `Ethanol`       |
| GM-702B | CO      |  `0x07` | `CO`            |

The GM-502B VOC channel is not included in the current telemetry record.

## Important Measurement Note

The values transmitted by this system are **raw sensor values**.

They are not ppm measurements.

The current milestone does not implement a validated raw-value-to-ppm calibration model.

## Hardware Connections

### Grove Sensor → STM32MP157-DK1

| Grove Sensor | STM32            |
| ------------ | ---------------- |
| SCL          | CN2 pin 5 / PA11 |
| SDA          | CN2 pin 3 / PA12 |
| VCC          | 3.3V             |
| GND          | CN2 pin 6        |

### STM32 → ESP32-A

| STM32                       | ESP32-A     |
| --------------------------- | ----------- |
| CN14 pin 2 / PE8 / UART7_TX | GPIO20 / RX |
| CN14 pin 1 / PE7 / UART7_RX | GPIO21 / TX |
| GND                         | GND         |
| 3.3V                        | 3.3V        |

## STM32 Device Tree

Linux I2C5 was enabled by changing the Linux I2C5 controller status from:

```dts
status = "disabled";
```

to:

```dts
status = "okay";
```

After reboot:

```text
/dev/i2c-1
```

was available.

The Cortex-M4 resource node was not changed.

## Sensor Protocol

The sensor uses I2C address:

```text
0x08
```

The selected commands are:

```text
0x01 -> GM-102B / NO₂
0x03 -> GM-302B / Ethanol
0x07 -> GM-702B / CO
```

The read sequence is:

```text
Write command
    ↓
Wait approximately 1 ms
    ↓
Read 4 bytes
    ↓
Interpret as little-endian raw value
```

## STM32 UART

UART7 is exposed as:

```text
/dev/ttySTM2
```

Configuration:

```text
115200 baud
8 data bits
No parity
1 stop bit
No hardware flow control
```

## Telemetry Format

The STM32 sends:

```text
GAS,<sequence>,<NO2_raw>,<Ethanol_raw>,<CO_raw>
```

Example:

```text
GAS,000006,309,64,161
```

## Software

### STM32

The STM32 currently uses a POSIX shell script:

```text
stm32/scripts/gas_uart_bridge.sh
```

The script:

1. Starts sensor warming.
2. Reads NO₂.
3. Reads Ethanol.
4. Reads CO.
5. Creates the telemetry record.
6. Sends the record through UART7.
7. Repeats every two seconds.

### ESP32-A

```text
esp32/esp32-a-uart-espnow/
```

ESP32-A:

1. Receives the STM32 UART record.
2. Prints the received record.
3. Sends the same record using ESP-NOW.

### ESP32-B

```text
esp32/esp32-b-gas-receiver/
```

ESP32-B:

1. Receives the ESP-NOW payload.
2. Parses the telemetry record.
3. Extracts the sequence number.
4. Extracts NO₂, Ethanol and CO.
5. Displays the raw values.
6. Reports whether the expected packet structure was successfully parsed.

## Example Output

### STM32

```text
STM32 TX: GAS,000006,219,66,162
STM32 TX: GAS,000007,216,66,162
```

### ESP32-A

```text
UART RX: GAS,000006,219,66,162
ESP-NOW TX: SUCCESS
```

### ESP32-B

```text
================================
       GAS SENSOR DATA
================================
Packet  : GAS,000006,309,64,161

Sequence : 6

NO2      : 309 RAW
Ethanol  : 64 RAW
CO       : 161 RAW

STATUS   : VALID
================================
```

## Validation Status

| Stage                      | Status |
| -------------------------- | ------ |
| Gas Sensor → STM32 I2C     | PASS   |
| STM32 sensor acquisition   | PASS   |
| STM32 UART7 → ESP32-A      | PASS   |
| ESP32-A UART reception     | PASS   |
| ESP32-A → ESP32-B ESP-NOW  | PASS   |
| ESP32-B packet parsing     | PASS   |
| Human-readable gas display | PASS   |

## Repository Structure

```text
grove-gas-telemetry/
│
├── README.md
├── LICENSE
├── .gitignore
│
├── docs/
│   └── Grove_Gas_Sensor_STM32_ESP32_Integration_Engineering_Document.docx
│
├── stm32/
│   ├── scripts/
│   │   └── gas_uart_bridge.sh
│   └── device-tree/
│
├── esp32/
│   ├── esp32-a-uart-espnow/
│   │   └── esp32-a-uart-espnow.ino
│   └── esp32-b-gas-receiver/
│       └── esp32-b-gas-receiver.ino
│
├── hardware/
│   ├── wiring.md
│   └── pinout.md
│
└── test-results/
    └── milestone-01-validation.md
```

## Current Scope

This repository represents the gas-telemetry milestone only.

The following are intentionally not implemented in this milestone:

* CRC
* ACK/retry
* Duplicate detection
* Sequence-gap handling
* Channel hopping
* Interference/jamming resistance
* Cryptographic security
* Sensor ppm calibration
* Production-grade STM32 service implementation

These features can be addressed in separate future milestones if required.
