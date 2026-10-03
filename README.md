# Floppy256

A 256Kbit (32KB) I²C EEPROM shaped like a miniature 3.5" floppy disk.

Made by [FruitWorks](#).

![Floppy256](https://github.com/user-attachments/assets/4fb0afd0-9ba6-4024-9965-b57a5103d7a0)

<img width="1723" height="965" alt="floppyrear" src="https://github.com/user-attachments/assets/a2372e47-2a81-4f66-b33d-1b80e0c999a5" />


## What it is

Floppy256 is a small I²C EEPROM module inspired by the classic 3.5" floppy disk and the familiar save icon.

The idea came from looking at generic EEPROM breakout boards and wondering why a memory module had to look like every other little PCB. I wanted to take something simple and useful and give it a bit of retro-computing personality.

It uses a 256Kbit EEPROM and provides a simple four-pin I²C interface for adding non-volatile storage to microcontroller projects.

## Specs

| | |
|---|---|
| EEPROM | ST M24256-DFMN6TP |
| Capacity | 256 Kbit / 32 KB |
| Interface | I²C |
| Default I²C Address | `0x50` |
| Address Pins | A0, A1, A2 strapped low |
| Page Size | 64 bytes |
| EEPROM Supply Voltage | 1.7V–5.5V |
| Pinout | SDA, SCL, VCC, GND |

The EEPROM address is determined by A0/A1/A2. In the current Floppy256 hardware design, these pins are tied low, giving a default address of `0x50`.

For a 3.3V microcontroller, power the module from 3.3V. For a 5V system, power it from 5V. The host MCU I²C logic level should match the module supply voltage.

Datasheet: [ST M24256-DF](https://www.st.com/en/memories/m24256-df.html)

## Pinout

| Pin | Function |
|---|---|
| SDA | I²C data |
| SCL | I²C clock |
| VCC | Power |
| GND | Ground |

## Wiring

```text
Floppy256       Microcontroller
-----------     --------------
VCC       ----> 3.3V or 5V*
GND       ----> GND
SDA       ----> I²C SDA
SCL       ----> I²C SCL
```

`*` Use a supply voltage appropriate for your MCU's I²C logic level.

The board includes the I²C pull-up resistors required for normal operation.

## Getting Started

### Arduino example

The following example writes a short message to address `0` and reads it back.

```cpp
#include <Wire.h>

#define EEPROM_ADDR 0x57

void writeEEPROM(uint16_t address, const char *text) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write(address >> 8);
  Wire.write(address & 0xFF);

  while (*text) {
    Wire.write(*text++);
  }

  Wire.endTransmission();
  delay(10);
}

void readEEPROM(uint16_t address, char *buffer, size_t length) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write(address >> 8);
  Wire.write(address & 0xFF);
  Wire.endTransmission(false);

  Wire.requestFrom(EEPROM_ADDR, (uint8_t)length);

  size_t i = 0;
  while (Wire.available() && i < length - 1) {
    buffer[i++] = Wire.read();
  }

  buffer[i] = '\0';
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  writeEEPROM(0, "Hello from Floppy256!");

  char data[32];
  readEEPROM(0, data, sizeof(data));

  Serial.println(data);
}

void loop() {
}
```

Serial Monitor output:

```text
Hello from Floppy256!
```
## Floppy256 in Action

Floppy256 works with a wide range of microcontrollers through the standard I²C interface.

<img width="1199" height="1286" alt="RPI_Pico" src="https://github.com/user-attachments/assets/82848b34-5f4a-4016-9b77-f342b84a86bc" />

Floppy256 connected to Raspberry Pi Pico

<img width="867" height="793" alt="STM32_Bluepill" src="https://github.com/user-attachments/assets/17db4517-8372-464b-8661-40e35703e458" />

Floppy256 connected to STM32 Bluepill

<img width="3120" height="3920" alt="ESP32" src="https://github.com/user-attachments/assets/76e014cd-4b12-40eb-b1e3-a38097144165" />

Floppy256 connected to ESP32 Development Boards



## Open Source / Build Your Own

Floppy256 is an open hardware project.

If you build your own Floppy256, a credit or link back to FruitWorks is appreciated.

If you make improvements or create your own version, feel free to share them.

## Manufacturing

Floppy256 was designed in KiCad and is produced in very small batches.

The design uses standard PCB manufacturing and assembly processes and commonly available components.

## License

The hardware, software, documentation, and images in this repository are intended to remain open.

See the license files included in this repository for the applicable terms.

## Credits

Designed and built by **FruitWorks**.

Floppy256 is a small independent hardware project made in very small batches.

## Get One

Floppy256 is available from [Tindie](#).

Made for makers, retro-computing fans, and anyone who thinks a floppy-shaped EEPROM is a little more fun than a generic breakout board.
