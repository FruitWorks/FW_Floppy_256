# Pinout

| Pin | Function | Notes |
|---|---|---|
| SDA | I2C data | Onboard 4.7kΩ pull-up |
| SCL | I2C clock | Onboard 4.7kΩ pull-up |
| VCC | Power | __ V range |
| GND | Ground | |

## I2C Address

<!-- Fill in based on A0/A1/A2 strapping on the actual shipped board -->

Address: `0x__`

## Example wiring (Raspberry Pi Pico / RP2040)

| Pico pin | Floppy256 pin |
|---|---|
| GPIO__ | SDA |
| GPIO__ | SCL |
| GND | GND |
| __ | VCC |

<!-- Note which I2C hardware block (I2C0/I2C1) these GPIOs belong to -->
