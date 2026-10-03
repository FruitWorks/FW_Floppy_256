/*
 * Generic I2C EEPROM Test Firmware
 * For 24LC256-family EEPROMs (256Kbit / 32KB, 64-byte page, 2-byte address)
 * MCU: any Arduino-framework board with a configurable I2C port
 *
 * This is a generic hardware-bring-up test, not tied to any specific
 * product or vendor. Edit the CONFIG block below for your board's
 * wiring and I2C address, then upload.
 *
 * Works with any chip in the 24LC256 family (ST M24256, Microchip
 * 24LC256/24AA256, ON Semi CAT24C256, etc.) since they share the same
 * 32KB / 64-byte-page / 2-byte-address electrical interface.
 */

#include <Wire.h>

// ====================== CONFIG — edit for your board ======================

#define EEPROM_I2C_ADDR   0x50   // default 24LC256 address with A0/A1/A2 low.
                                  // Change if your board straps A0/A1/A2 differently.

#define USE_SECOND_I2C_BUS false // set true if your SDA/SCL pins require a
                                  // second I2C hardware block (e.g. RP2040
                                  // GPIO18/19 = I2C1, not I2C0 = Wire).
                                  // If true, this sketch uses Wire1 instead
                                  // of Wire -- check your MCU's pinout docs.

#define SDA_PIN           -1     // set to your SDA GPIO number, or -1 to use
                                  // the board's default I2C pins (no remap)
#define SCL_PIN           -1     // set to your SCL GPIO number, or -1 to use
                                  // the board's default I2C pins (no remap)

// NOTE: setSDA()/setSCL() pin remapping (used below) is available on
// RP2040 and some ESP32 cores. On AVR boards (Uno, Nano, Mega) and most
// other MCUs, I2C pins are fixed in hardware and cannot be remapped --
// leave SDA_PIN/SCL_PIN at -1 on those boards.

// ====================== Chip constants (24LC256 family) ===================

#define PAGE_SIZE         64      // bytes per write page
#define EEPROM_SIZE       32768   // 32KB total
#define WRITE_DELAY_MS    10      // datasheet max write time is 5ms, margin added
#define MAX_WRITE_RETRIES 20      // ACK-polling retry count

#if USE_SECOND_I2C_BUS
  #define I2C_PORT Wire1
#else
  #define I2C_PORT Wire
#endif

bool overallPass = true;

// ---------- Low-level EEPROM access ----------

// Write up to PAGE_SIZE bytes, MUST NOT cross a page boundary.
bool eepromWritePage(uint16_t addr, const uint8_t *data, uint8_t len) {
  if (len == 0 || len > PAGE_SIZE) return false;

  // Hard check: reject any write that would cross a page boundary
  uint16_t pageStart = addr & ~(PAGE_SIZE - 1);
  if ((uint32_t)addr + len > (uint32_t)pageStart + PAGE_SIZE) {
    Serial.print("  [ERROR] Write at 0x");
    Serial.print(addr, HEX);
    Serial.println(" would cross page boundary — aborting this write.");
    return false;
  }
  if ((uint32_t)addr + len > EEPROM_SIZE) {
    Serial.println("  [ERROR] Write exceeds EEPROM address range.");
    return false;
  }

  I2C_PORT.beginTransmission(EEPROM_I2C_ADDR);
  I2C_PORT.write((uint8_t)(addr >> 8));   // address high byte
  I2C_PORT.write((uint8_t)(addr & 0xFF)); // address low byte
  for (uint8_t i = 0; i < len; i++) {
    I2C_PORT.write(data[i]);
  }
  uint8_t result = I2C_PORT.endTransmission();
  if (result != 0) {
    Serial.print("  [ERROR] I2C write failed, code=");
    Serial.println(result);
    return false;
  }

  // ACK polling: wait for internal write cycle to finish.
  // During write cycle, EEPROM NACKs its own address.
  for (int i = 0; i < MAX_WRITE_RETRIES; i++) {
    delay(1);
    I2C_PORT.beginTransmission(EEPROM_I2C_ADDR);
    if (I2C_PORT.endTransmission() == 0) {
      delay(WRITE_DELAY_MS); // small extra margin
      return true;
    }
  }

  Serial.println("  [ERROR] Write cycle did not complete (ACK poll timeout).");
  return false;
}

bool eepromRead(uint16_t addr, uint8_t *data, uint8_t len) {
  if (len == 0 || (uint32_t)addr + len > EEPROM_SIZE) return false;

  I2C_PORT.beginTransmission(EEPROM_I2C_ADDR);
  I2C_PORT.write((uint8_t)(addr >> 8));
  I2C_PORT.write((uint8_t)(addr & 0xFF));
  if (I2C_PORT.endTransmission(false) != 0) { // repeated start, no stop
    Serial.println("  [ERROR] I2C read address-set failed.");
    return false;
  }

  uint8_t received = I2C_PORT.requestFrom((uint8_t)EEPROM_I2C_ADDR, len);
  if (received != len) {
    Serial.print("  [ERROR] Requested ");
    Serial.print(len);
    Serial.print(" bytes, got ");
    Serial.println(received);
    return false;
  }

  for (uint8_t i = 0; i < len; i++) {
    data[i] = I2C_PORT.read();
  }
  return true;
}

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

  // Build a deterministic, address-dependent pattern (avoids false pass
  // from a stuck-at-0x00 or stuck-at-0xFF bus/chip fault)
  uint8_t writeBuf[PAGE_SIZE];
  for (uint8_t i = 0; i < testLen; i++) {
    writeBuf[i] = (uint8_t)((addr + i) ^ 0xA5);
  }

  if (!eepromWritePage(addr, writeBuf, testLen)) {
    Serial.println("  -> FAIL (write)");
    return false;
  }

  uint8_t readBuf[PAGE_SIZE];
  if (!eepromRead(addr, readBuf, testLen)) {
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
  // One full page, sequential pattern, at an address not used by the spot tests.
  const uint16_t addr = 0x2000;
  Serial.println("Running sequential 64-byte page test at 0x2000...");

  uint8_t writeBuf[PAGE_SIZE];
  for (int i = 0; i < PAGE_SIZE; i++) {
    writeBuf[i] = (uint8_t)i;
  }

  if (!eepromWritePage(addr, writeBuf, PAGE_SIZE)) {
    Serial.println("  -> FAIL (write)");
    return false;
  }

  uint8_t readBuf[PAGE_SIZE];
  if (!eepromRead(addr, readBuf, PAGE_SIZE)) {
    Serial.println("  -> FAIL (read)");
    return false;
  }

  for (int i = 0; i < PAGE_SIZE; i++) {
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
  while (!Serial) { delay(10); } // wait for USB serial, harmless if none

  delay(1000);
  Serial.println("=================================");
  Serial.println("Generic 24LC256-family EEPROM Test");
  Serial.println("=================================");

#if SDA_PIN >= 0 && SCL_PIN >= 0
  I2C_PORT.setSDA(SDA_PIN);
  I2C_PORT.setSCL(SCL_PIN);
#endif
  I2C_PORT.begin();
  I2C_PORT.setClock(100000); // 100kHz standard mode, safest for test jigs

  // --- Step 1: Presence check ---
  Serial.print("\n[1] Scanning for EEPROM at 0x");
  Serial.print(EEPROM_I2C_ADDR, HEX);
  Serial.println("...");
  I2C_PORT.beginTransmission(EEPROM_I2C_ADDR);
  uint8_t scanResult = I2C_PORT.endTransmission();

  if (scanResult != 0) {
    Serial.println("EEPROM NOT detected.");
    Serial.println("Check: VCC, GND, SDA/SCL wiring, address pin strapping, solder joints.");
    overallPass = false;
  } else {
    Serial.println("EEPROM detected.");

    // --- Step 2: Spot address tests ---
    Serial.println("\n[2] Running spot read/write tests...");
    overallPass &= testLocation(0x0000, 8);
    overallPass &= testLocation(0x0010, 8);
    overallPass &= testLocation(0x0100, 8);
    overallPass &= testLocation(0x1000, 8);
    overallPass &= testLocation(0x7FFF, 1); // last valid byte — single-byte only

    // --- Step 3: Sequential page test ---
    Serial.println("\n[3] Running sequential block test...");
    overallPass &= testSequentialBlock();
  }

  // --- Final verdict ---
  Serial.println("\n=================================");
  if (overallPass) {
    Serial.println("EEPROM TEST: PASS");
  } else {
    Serial.println("EEPROM TEST: FAIL");
  }
  Serial.println("=================================");
}

void loop() {
  // Nothing to do — test runs once in setup().
  // Re-power or reset the board to test the next unit.
}
