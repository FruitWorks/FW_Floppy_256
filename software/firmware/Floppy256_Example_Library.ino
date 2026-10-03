/*
 * FruitWorks Floppy256 — Manufacturing Test (library-based)
 * MCU: Raspberry Pi Pico (RP2040), Arduino framework
 *
 * This version uses the FruitWorks_Floppy256 library instead of
 * duplicating raw I2C read/write logic -- the library already handles
 * page-boundary splitting and write-cycle ACK polling, so this sketch
 * only has to drive the test sequence and report PASS/FAIL.
 *
 * REQUIRES: FruitWorks_Floppy256 library installed
 * (Sketch -> Include Library -> Add .ZIP Library...)
 *
 * Wiring:
 *   Pico GPIO18  -> Floppy256 SDA
 *   Pico GPIO19  -> Floppy256 SCL
 *   Pico GND     -> Floppy256 GND
 *   Pico VBUS(5V)-> Floppy256 VCC
 *
 * IMPORTANT: GPIO18/19 on RP2040 belong to the I2C1 hardware block, not
 * I2C0 -- this sketch uses Wire1 for that reason.
 */

#include <Wire.h>
#include "FruitWorks_Floppy256.h"

#define SDA_PIN 18
#define SCL_PIN 19

Floppy256 myFloppy;
bool overallPass = true;

// ---------- Test routines ----------

bool testLocation(uint16_t addr, uint8_t testLen) {
  Serial.print("Testing address 0x");
  if (addr < 0x1000) Serial.print("0");
  if (addr < 0x100) Serial.print("0");
  if (addr < 0x10) Serial.print("0");
  Serial.print(addr, HEX);
  Serial.print(" (");
  Serial.print(testLen);
  Serial.println(" bytes)...");

  // Deterministic, address-dependent pattern -- avoids a false pass from
  // a stuck-at-0x00 or stuck-at-0xFF bus/chip fault.
  uint8_t writeBuf[64];
  for (uint8_t i = 0; i < testLen; i++) {
    writeBuf[i] = (uint8_t)((addr + i) ^ 0xA5);
  }

  if (!myFloppy.writeBlock(addr, writeBuf, testLen)) {
    Serial.println("  -> FAIL (write)");
    return false;
  }

  uint8_t readBuf[64];
  if (!myFloppy.readBlock(addr, readBuf, testLen)) {
    Serial.println("  -> FAIL (read)");
    return false;
  }

  for (uint8_t i = 0; i < testLen; i++) {
    if (readBuf[i] != writeBuf[i]) {
      Serial.print("  [MISMATCH] offset ");
      Serial.print(i);
      Serial.print(" wrote 0x");
      Serial.print(writeBuf[i], HEX);
      Serial.print(" read 0x");
      Serial.println(readBuf[i], HEX);
      Serial.println("  -> FAIL (data mismatch)");
      return false;
    }
  }

  Serial.println("  -> PASS");
  return true;
}

bool testSequentialBlock() {
  const uint16_t addr = 0x2000;
  const uint8_t len = 64; // one full page
  Serial.println("Running sequential 64-byte page test at 0x2000...");

  uint8_t writeBuf[len];
  for (int i = 0; i < len; i++) {
    writeBuf[i] = (uint8_t)i;
  }

  if (!myFloppy.writeBlock(addr, writeBuf, len)) {
    Serial.println("  -> FAIL (write)");
    return false;
  }

  uint8_t readBuf[len];
  if (!myFloppy.readBlock(addr, readBuf, len)) {
    Serial.println("  -> FAIL (read)");
    return false;
  }

  for (int i = 0; i < len; i++) {
    if (readBuf[i] != writeBuf[i]) {
      Serial.print("  [MISMATCH] offset ");
      Serial.println(i);
      Serial.println("  -> FAIL (data mismatch)");
      return false;
    }
  }

  Serial.println("  -> PASS");
  return true;
}

// ---------- Setup / Loop ----------

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  delay(1000);
  Serial.println("=================================");
  Serial.println("FruitWorks Floppy256 Production Test (library-based)");
  Serial.println("=================================");

  Wire1.setSDA(SDA_PIN);
  Wire1.setSCL(SCL_PIN);
  Wire1.begin();
  Wire1.setClock(100000);

  Serial.println("\n[1] Connecting to Floppy256 at 0x57...");
  if (!myFloppy.begin(Wire1)) {
    Serial.println("Floppy256 EEPROM NOT detected.");
    Serial.println("Check: VCC, GND, SDA/SCL wiring, solder joints on U1.");
    overallPass = false;
  } else {
    Serial.println("Floppy256 EEPROM detected.");

    Serial.println("\n[2] Running spot read/write tests...");
    overallPass &= testLocation(0x0000, 8);
    overallPass &= testLocation(0x0010, 8);
    overallPass &= testLocation(0x0100, 8);
    overallPass &= testLocation(0x1000, 8);
    overallPass &= testLocation(0x7FFF, 1); // last valid byte -- single-byte only

    Serial.println("\n[3] Running sequential block test...");
    overallPass &= testSequentialBlock();
  }

  Serial.println("\n=================================");
  if (overallPass) {
    Serial.println("FLOPPY256 TEST: PASS");
  } else {
    Serial.println("FLOPPY256 TEST: FAIL");
  }
  Serial.println("=================================");
}

void loop() {
  // Nothing to do -- test runs once in setup().
  // Re-power or reset the Pico to test the next board.
}
