# FAQ

**Is this real storage, or just a novelty shape?**

It’s real storage. Floppy256 uses a 256Kbit (32KB) I²C EEPROM, so you can store configuration data, calibration values, settings, small logs, or other non-volatile data for your projects. The floppy shape is just our way of making a familiar component a little more interesting.

**Why no QWIIC/STEMMA connector?**

Floppy256 keeps things simple with a standard 4-pin header: **VCC, GND, SDA and SCL**. It keeps the board small and preserves the floppy-disk-inspired design without adding another connector standard.

**What's the real capacity?**

Floppy256 has **256Kbit of EEPROM**, which is **32KB (32,768 bytes)** of usable non-volatile storage.

**Can I chain multiple Floppy256 boards on one bus?**

Yes, I²C allows multiple devices on the same bus, but the current Floppy256 design has A0, A1, and A2 strapped low, so every board uses the same default address (`0x57`). Because of this, multiple standard Floppy256 boards cannot be used directly on the same bus without changing the addressing arrangement or using an I²C multiplexer.

**Does it need external pull-ups?**

No, not normally. Floppy256 already includes **4.7kΩ pull-up resistors on SDA and SCL**. Just connect it to your I²C bus and power it appropriately.

**What microcontrollers does it work with?**

Floppy256 works with most microcontrollers that support **I²C**, including platforms such as **Arduino, ESP32, ESP8266, RP2040/Pico and STM32**.

The EEPROM itself supports **1.7V–5.5V operation**. Use a supply voltage and I²C logic level that are appropriate for your microcontroller.
