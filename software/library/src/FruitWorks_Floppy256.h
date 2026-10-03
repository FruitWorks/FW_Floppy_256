/*
  FruitWorks Floppy256 — Arduino Library
  ----------------------------------------

  A minimal I2C EEPROM library for the FruitWorks Floppy256 board
  (ST M24256 / 24LC256-compatible, 32KB, address 0x57).

  Credit: the function layout and general approach of this library
  (begin/isConnected/read/write, automatic page-boundary handling,
  ACK-polling for write completion) is modeled after the excellent
  SparkFun External EEPROM Arduino Library:
    https://github.com/sparkfun/SparkFun_External_EEPROM_Arduino_Library
  This is a from-scratch, much smaller implementation written
  specifically for the Floppy256 board — it is not a copy of SparkFun's
  source, but it would not exist in this shape without their library
  as a reference. If you need a library that supports many different
  EEPROM chip sizes, use SparkFun's library instead; this one only
  targets the 24LC256 / M24256 family.

  --------------------------------------------------------------------
  HOW TO USE THIS LIBRARY
  --------------------------------------------------------------------

  1. Wire the Floppy256 board to your microcontroller's I2C pins
     (SDA, SCL, VCC, GND). On RP2040 boards, remember GPIO18/19
     belong to the I2C1 hardware block, not I2C0 — use Wire1, and
     call Wire1.setSDA()/setSCL() with your chosen pins *before*
     calling Floppy256.begin().

  2. Create a global object:
         Floppy256 myFloppy;

  3. In setup(), start I2C yourself, then call begin():
         Wire1.setSDA(18);
         Wire1.setSCL(19);
         Wire1.begin();
         myFloppy.begin(Wire1);

     begin() returns true if the EEPROM responded at 0x57.

  4. Write and read single bytes:
         myFloppy.writeByte(0x0000, 0xAB);
         uint8_t value = myFloppy.readByte(0x0000);

  5. Write and read blocks of bytes (page boundaries are handled
     for you automatically — you can write across a page edge and
     it will be split into multiple page writes internally):
         uint8_t data[10] = {1,2,3,4,5,6,7,8,9,10};
         myFloppy.writeBlock(0x0100, data, 10);

         uint8_t readBack[10];
         myFloppy.readBlock(0x0100, readBack, 10);

  6. Check if the chip is present at any time:
         if (myFloppy.isConnected()) { ... }

  See examples/Example1_BasicReadWrite for a complete, runnable sketch
  with expected Serial Monitor output documented in comments.

  --------------------------------------------------------------------
  LIMITS
  --------------------------------------------------------------------
  - Fixed to the Floppy256 EEPROM: 32768 bytes (0x0000–0x7FFF),
    64-byte pages, 2-byte addressing, I2C address 0x57.
  - Does not support other EEPROM sizes — this is deliberate, to keep
    the library small and easy to read. Use SparkFun's library if you
    need multi-chip-size support.
*/

#ifndef FRUITWORKS_FLOPPY256_H
#define FRUITWORKS_FLOPPY256_H

#include <Arduino.h>
#include <Wire.h>

// ---- Fixed Floppy256 / M24256 characteristics ----
#define FLOPPY256_I2C_ADDRESS   0x57
#define FLOPPY256_SIZE_BYTES    32768UL
#define FLOPPY256_PAGE_SIZE     64
#define FLOPPY256_WRITE_TIMEOUT_MS 50   // safety ceiling for ACK-poll wait

class Floppy256
{
  public:
    Floppy256();

    // Starts the library against an already-begun TwoWire port.
    // Call Wire.begin() / Wire1.begin() etc. yourself BEFORE calling this.
    // Returns true if the EEPROM acknowledges at 0x57.
    bool begin(TwoWire &wirePort = Wire, uint8_t i2cAddress = FLOPPY256_I2C_ADDRESS);

    // Returns true if the EEPROM currently acknowledges on the bus.
    bool isConnected();

    // Single-byte operations
    bool writeByte(uint16_t eepromAddress, uint8_t data);
    uint8_t readByte(uint16_t eepromAddress);

    // Multi-byte operations. Automatically splits writes across
    // 64-byte page boundaries — safe to call with any length/address
    // combination within the chip's address range.
    bool writeBlock(uint16_t eepromAddress, const uint8_t *data, uint16_t length);
    bool readBlock(uint16_t eepromAddress, uint8_t *data, uint16_t length);

    // Returns the fixed capacity of the Floppy256 in bytes (32768).
    uint32_t length();

  private:
    TwoWire *_i2cPort;
    uint8_t _i2cAddress;
    bool _begun = false;

    // Writes one page-safe chunk (never crosses a 64-byte page boundary).
    bool writeChunk(uint16_t eepromAddress, const uint8_t *data, uint8_t length);

    // Waits for the EEPROM's internal write cycle to finish via ACK polling.
    bool waitForWriteComplete();
};

#endif // FRUITWORKS_FLOPPY256_H
