# Pinout

| Pin | Function | Notes |
|---|---|---|
| SDA | I2C data | Onboard 4.7kΩ pull-up |
| SCL | I2C clock | Onboard 4.7kΩ pull-up |
| VCC | Power | __ V range |
| GND | Ground | |

## I2C Address

Address: `0x57`

## Example wiring (Raspberry Pi Pico / RP2040)

| Pico pin | Floppy256 pin |
|---|---|
| GPIO__ | SDA |
| GPIO__ | SCL |
| GND | GND |
| 3V3_OUT | VCC |

<!-- Note which I2C hardware block (I2C0/I2C1) these GPIOs belong to -->
