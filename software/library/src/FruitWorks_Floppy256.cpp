/*
  FruitWorks Floppy256 — Arduino Library
  See FruitWorks_Floppy256.h for usage instructions and credit notes.
*/

#include "FruitWorks_Floppy256.h"

Floppy256::Floppy256()
{
    _i2cPort = &Wire;
    _i2cAddress = FLOPPY256_I2C_ADDRESS;
}

bool Floppy256::begin(TwoWire &wirePort, uint8_t i2cAddress)
{
    _i2cPort = &wirePort;
    _i2cAddress = i2cAddress;
    _begun = true;

    return isConnected();
}

bool Floppy256::isConnected()
{
    if (!_begun)
        return false;

    _i2cPort->beginTransmission(_i2cAddress);
    uint8_t result = _i2cPort->endTransmission();
    return (result == 0);
}

bool Floppy256::waitForWriteComplete()
{
    // The EEPROM NACKs its own address while an internal write cycle
    // is in progress. Poll until it ACKs again, with a hard timeout
    // so a genuinely broken board doesn't hang the sketch forever.
    unsigned long startTime = millis();

    while (millis() - startTime < FLOPPY256_WRITE_TIMEOUT_MS)
    {
        _i2cPort->beginTransmission(_i2cAddress);
        uint8_t result = _i2cPort->endTransmission();
        if (result == 0)
            return true; // chip ACKed -> write cycle complete

        delay(1);
    }

    return false; // timed out -- treat as a failed write
}

bool Floppy256::writeChunk(uint16_t eepromAddress, const uint8_t *data, uint8_t length)
{
    if (!_begun || length == 0 || length > FLOPPY256_PAGE_SIZE)
        return false;

    // Hard guard: this chunk must not cross a page boundary.
    uint16_t pageStart = eepromAddress & ~(FLOPPY256_PAGE_SIZE - 1);
    if ((uint32_t)eepromAddress + length > (uint32_t)pageStart + FLOPPY256_PAGE_SIZE)
        return false;

    if ((uint32_t)eepromAddress + length > FLOPPY256_SIZE_BYTES)
        return false;

    _i2cPort->beginTransmission(_i2cAddress);
    _i2cPort->write((uint8_t)(eepromAddress >> 8));   // address high byte
    _i2cPort->write((uint8_t)(eepromAddress & 0xFF)); // address low byte
    for (uint8_t i = 0; i < length; i++)
        _i2cPort->write(data[i]);

    uint8_t result = _i2cPort->endTransmission();
    if (result != 0)
        return false;

    return waitForWriteComplete();
}

bool Floppy256::writeByte(uint16_t eepromAddress, uint8_t data)
{
    return writeChunk(eepromAddress, &data, 1);
}

bool Floppy256::writeBlock(uint16_t eepromAddress, const uint8_t *data, uint16_t length)
{
    if (!_begun || length == 0)
        return false;

    if ((uint32_t)eepromAddress + length > FLOPPY256_SIZE_BYTES)
        return false;

    uint16_t bytesWritten = 0;

    while (bytesWritten < length)
    {
        uint16_t currentAddress = eepromAddress + bytesWritten;
        uint16_t pageStart = currentAddress & ~(FLOPPY256_PAGE_SIZE - 1);
        uint16_t spaceLeftInPage = (pageStart + FLOPPY256_PAGE_SIZE) - currentAddress;

        uint16_t remaining = length - bytesWritten;
        uint8_t chunkLength = (remaining < spaceLeftInPage) ? (uint8_t)remaining : (uint8_t)spaceLeftInPage;

        if (!writeChunk(currentAddress, data + bytesWritten, chunkLength))
            return false; // bail out immediately on any failed chunk

        bytesWritten += chunkLength;
    }

    return true;
}

uint8_t Floppy256::readByte(uint16_t eepromAddress)
{
    uint8_t value = 0;
    readBlock(eepromAddress, &value, 1);
    return value;
}

bool Floppy256::readBlock(uint16_t eepromAddress, uint8_t *data, uint16_t length)
{
    if (!_begun || length == 0)
        return false;

    if ((uint32_t)eepromAddress + length > FLOPPY256_SIZE_BYTES)
        return false;

    // Set the read pointer with a repeated start (no stop condition),
    // then request the bytes. This matches the 24LC256's random-read
    // sequence per its datasheet.
    _i2cPort->beginTransmission(_i2cAddress);
    _i2cPort->write((uint8_t)(eepromAddress >> 8));
    _i2cPort->write((uint8_t)(eepromAddress & 0xFF));
    if (_i2cPort->endTransmission(false) != 0)
        return false;

    // Most Arduino cores cap a single requestFrom() at 32 bytes
    // (classic AVR boards) up to 256 bytes (RP2040, ESP32). Chunking
    // at 32 bytes keeps this library safe on every supported platform,
    // including the smallest AVR boards, without needing per-platform
    // #ifdefs. Each chunk continues reading from where the last one
    // left off, per the 24LC256 datasheet's sequential-read behavior.
    const uint8_t READ_CHUNK = 32;

    uint16_t bytesRead = 0;
    while (bytesRead < length)
    {
        uint16_t remaining = length - bytesRead;
        uint8_t requestLength = (remaining < READ_CHUNK) ? (uint8_t)remaining : READ_CHUNK;

        uint8_t received = _i2cPort->requestFrom((uint8_t)_i2cAddress, requestLength);
        if (received != requestLength)
            return false;

        for (uint8_t i = 0; i < requestLength; i++)
            data[bytesRead + i] = _i2cPort->read();

        bytesRead += requestLength;
    }

    return true;
}

uint32_t Floppy256::length()
{
    return FLOPPY256_SIZE_BYTES;
}
