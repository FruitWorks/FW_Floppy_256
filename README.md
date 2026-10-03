# Floppy256

A 256Kbit (32KB) I2C EEPROM, shaped like a miniature 3.5" floppy disk.

Made by [FruitWorks](#) — fill in link.

![Floppy256](images/front.png)

## What it is

<!-- Short description — what the board is and why it exists -->

## Specs

| | |
|---|---|
| EEPROM | ST M24256-DFMN6TP (24LC256-compatible) |
| Capacity | 256 Kbit / 32 KB |
| Interface | I2C |
| Address | 0x__ (fill in — depends on A0/A1/A2 strapping) |
| Page size | 64 bytes |
| Board size | __ mm x __ mm |
| Operating voltage | __ |
| Pinout | SDA, SCL, VCC, GND |

## Repo structure

```
/docs        — pinout, datasheet notes, build log, FAQ
/hardware    — KiCad source, gerbers, STEP file, BOM
/software    — firmware (test code) and example sketches
/images      — product photos and renders
```

## Getting started

<!-- Wiring diagram + minimal code snippet to read/write the EEPROM -->

```cpp
// fill in a minimal usage example
```

Full example: [`software/examples/`](software/examples/)

## Open source / build your own

This project is fully open source. All design files (schematic, PCB layout,
gerbers, STEP model) are included so anyone can fabricate their own board.

**If you build or sell your own version of this board, a credit/link back
to FruitWorks is appreciated** (not legally required unless your chosen
license below says otherwise — update this section once you've picked one).

## License

- **Hardware** (schematic, PCB layout, gerbers): <!-- e.g. CERN-OHL-S v2 — fill in -->
- **Software** (firmware/examples): <!-- e.g. MIT — fill in -->
- **Documentation/images**: <!-- e.g. CC BY 4.0 — fill in -->

See [LICENSE](LICENSE) for full text.

## Credits

Designed by <!-- your name / FruitWorks -->
