# FruitWorks Floppy256 Arduino Library

A small, dependency-free I2C EEPROM library for the **FruitWorks Floppy256**
board — a 256Kbit (32KB) ST M24256 EEPROM (24LC256-compatible), shaped like
a miniature 3.5" floppy disk, running at I2C address **0x57**.

This library only talks to the Floppy256's specific chip (32KB, 64-byte
pages, 2-byte addressing, address 0x57). It is intentionally small and
single-purpose rather than a general multi-chip EEPROM library.

## Credit

This library's function layout and general approach — `begin()` /
`isConnected()` / `read()` / `write()`, automatic page-boundary splitting
on writes, and ACK-polling to detect when a write cycle has finished —
is modeled after the excellent **[SparkFun External EEPROM Arduino
Library](https://github.com/sparkfun/SparkFun_External_EEPROM_Arduino_Library)**.

The code in this repository is a from-scratch implementation written
specifically for the single fixed chip on the Floppy256 board — it is not
a copy of SparkFun's source. But this library would not exist in its
current shape without SparkFun's library as a reference, and credit goes
to them for the original design pattern.

**If you need a library that supports many different EEPROM sizes**
(24xx00 through 24xx2048), use SparkFun's library instead — it's more
capable and well maintained. This library trades that flexibility for
being as short and easy to read as possible for one specific board.

## Installation

1. Download this repository as a ZIP (or clone it).
2. In the Arduino IDE: **Sketch → Include Library → Add .ZIP Library...**
   and select the downloaded file.
   — or —
   Copy the `FruitWorks_Floppy256_Library` folder into your Arduino
   `libraries` folder directly.
3. Restart the Arduino IDE if it was already open.

## Wiring (Raspberry Pi Pico / RP2040 example)

| Pico pin | Floppy256 pin |
|---|---|
| GPIO18 | SDA |
| GPIO19 | SCL |
| GND | GND |
| VBUS (5V) | VCC |

**Important for RP2040 users:** GPIO18/19 belong to the RP2040's **I2C1**
hardware block, not I2C0. Use `Wire1`, not `Wire`, and call
`Wire1.setSDA()` / `Wire1.setSCL()` **before** calling `Floppy256::begin()`.
If you're using a different microcontroller or different pins, use
whichever `TwoWire` port matches your wiring instead.

The board already includes 4.7kΩ pull-ups on SDA/SCL and a 100nF
decoupling capacitor — no external components are needed.

## Usage

```cpp
#include <Wire.h>
#include "FruitWorks_Floppy256.h"

Floppy256 myFloppy;

void setup() {
  Serial.begin(115200);

  Wire1.setSDA(18);
  Wire1.setSCL(19);
  Wire1.begin();

  if (!myFloppy.begin(Wire1)) {
    Serial.println("Floppy256 not detected!");
    while (1);
  }

  // Write a single byte
  myFloppy.writeByte(0x0000, 0xAB);

  // Read a single byte
  uint8_t value = myFloppy.readByte(0x0000);

  // Write a block (page boundaries are handled automatically)
  uint8_t data[10] = {1,2,3,4,5,6,7,8,9,10};
  myFloppy.writeBlock(0x0100, data, 10);

  // Read a block
  uint8_t readBack[10];
  myFloppy.readBlock(0x0100, readBack, 10);
}

void loop() {}
```

See [`examples/Example1_BasicReadWrite`](examples/Example1_BasicReadWrite)
for a complete, runnable sketch with the exact Serial Monitor output you
should expect on a working board, documented in the sketch's header
comment.

## Function reference

| Function | Description |
|---|---|
| `bool begin(TwoWire &wirePort, uint8_t i2cAddress = 0x57)` | Call after you've already started your I2C port. Returns `true` if the EEPROM responds. |
| `bool isConnected()` | Returns `true` if the EEPROM currently acknowledges on the bus. |
| `bool writeByte(uint16_t address, uint8_t data)` | Writes a single byte. |
| `uint8_t readByte(uint16_t address)` | Reads a single byte. |
| `bool writeBlock(uint16_t address, const uint8_t *data, uint16_t length)` | Writes multiple bytes. Automatically splits the write across 64-byte page boundaries. |
| `bool readBlock(uint16_t address, uint8_t *data, uint16_t length)` | Reads multiple bytes. |
| `uint32_t length()` | Returns the fixed capacity of the Floppy256: 32768 bytes. |

All write functions return `false` immediately if the chip doesn't
acknowledge, if the address/length combination would exceed the chip's
32KB range, or if the internal write cycle doesn't complete within the
timeout — they will never silently report success on a failed write.

## License

- **This library's code** is released under the MIT License — see
  [LICENSE.md](LICENSE.md).
- The underlying **Floppy256 hardware design** (schematic, PCB, STEP
  files) is licensed separately — see the main
  [FruitWorks Floppy256 hardware repository](#) for details.

## Limitations

- Supports only the Floppy256's fixed chip configuration (32KB, 64-byte
  pages, 2-byte addressing). It does not auto-detect or support other
  EEPROM sizes.
- No `get()`/`put()` templated object storage (unlike SparkFun's
  library) — only raw byte/block read and write. This keeps the code
  short; feel free to add templated helpers yourself if you want them.
