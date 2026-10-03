/*
  FruitWorks Floppy256 — Example 1: Basic Read/Write
  ----------------------------------------------------
  Writes a small block of bytes to the Floppy256, reads it back, and
  verifies it matches. This is the simplest possible "does my board
  work" sketch.

  WIRING (Raspberry Pi Pico / RP2040):
    Pico GPIO18  -> Floppy256 SDA
    Pico GPIO19  -> Floppy256 SCL
    Pico GND     -> Floppy256 GND
    Pico VBUS/5V -> Floppy256 VCC

  NOTE: GPIO18/19 belong to the RP2040's I2C1 hardware block, not I2C0.
  This sketch uses Wire1 for that reason. If you wire Floppy256 to
  different pins, or a different MCU entirely, change Wire1 to the
  correct I2C port for your pins and update myFloppy.begin() to match.

  EXPECTED SERIAL MONITOR OUTPUT (on a working board):

    =================================
    FruitWorks Floppy256 - Basic Read/Write Example
    =================================
    Connecting to Floppy256 at 0x57...
    Floppy256 EEPROM detected.

    Writing test data to address 0x0010...
    Write complete.

    Reading back from address 0x0010...
    Read: 10 20 30 40 50 60 70 80 90 A0

    SUCCESS: Data matches what was written.

  If the EEPROM is not detected, check wiring, solder joints, and
  that you're using the correct I2C port for your pins.
*/

#include <Wire.h>
#include "FruitWorks_Floppy256.h"

Floppy256 myFloppy;

const uint16_t TEST_ADDRESS = 0x0010;
const uint8_t TEST_LENGTH = 10;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10); // wait for USB serial (needed on some boards, harmless on others)
    }

    delay(1000);
    Serial.println("=================================");
    Serial.println("FruitWorks Floppy256 - Basic Read/Write Example");
    Serial.println("=================================");

    // Start I2C on GPIO18/19 (RP2040 I2C1 block)
    Wire1.setSDA(18);
    Wire1.setSCL(19);
    Wire1.begin();

    Serial.println("Connecting to Floppy256 at 0x57...");
    if (!myFloppy.begin(Wire1))
    {
        Serial.println("Floppy256 EEPROM NOT detected.");
        Serial.println("Check wiring, solder joints, and I2C pins/port.");
        while (1)
        {
            delay(1000); // halt here -- nothing else to do without the chip
        }
    }
    Serial.println("Floppy256 EEPROM detected.");

    // Build a simple, recognizable test pattern: 0x10, 0x20, 0x30 ...
    uint8_t writeData[TEST_LENGTH];
    for (uint8_t i = 0; i < TEST_LENGTH; i++)
    {
        writeData[i] = (i + 1) * 0x10;
    }

    Serial.print("\nWriting test data to address 0x");
    Serial.print(TEST_ADDRESS, HEX);
    Serial.println("...");

    if (!myFloppy.writeBlock(TEST_ADDRESS, writeData, TEST_LENGTH))
    {
        Serial.println("Write FAILED.");
        return;
    }
    Serial.println("Write complete.");

    uint8_t readData[TEST_LENGTH];
    Serial.print("\nReading back from address 0x");
    Serial.print(TEST_ADDRESS, HEX);
    Serial.println("...");

    if (!myFloppy.readBlock(TEST_ADDRESS, readData, TEST_LENGTH))
    {
        Serial.println("Read FAILED.");
        return;
    }

    Serial.print("Read: ");
    for (uint8_t i = 0; i < TEST_LENGTH; i++)
    {
        if (readData[i] < 0x10)
            Serial.print("0");
        Serial.print(readData[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    // Verify
    bool match = true;
    for (uint8_t i = 0; i < TEST_LENGTH; i++)
    {
        if (readData[i] != writeData[i])
        {
            match = false;
            break;
        }
    }

    Serial.println();
    if (match)
    {
        Serial.println("SUCCESS: Data matches what was written.");
    }
    else
    {
        Serial.println("FAILURE: Data does not match what was written.");
    }
}

void loop()
{
    // Nothing to do -- this example runs once in setup().
}
