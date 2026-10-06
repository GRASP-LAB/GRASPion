# GRASPion Head nRF52832 Arduino support

This directory contains the Arduino support files for the nRF52832 / u-blox ANNA-B112 used in the GRASPion Head.

GRASPion uses three microcontrollers:

- SAMD21 — Body controller
- STM32U575 — Head main controller
- nRF52832 / ANNA-B112 — Head wireless controller

The nRF52 package is based on the Adafruit nRF52 Arduino core 1.7.0 and adds the `graspionHead` board variant.

## GRASPion Head nRF52832 pin mapping

### LED

- P0.15 — built-in LED

### Application UART to STM32U575

- P0.27 TX -> STM32 PA3 / LPUART1_RX
- P0.28 RX <- STM32 PA2 / LPUART1_TX

### Bootloader DFU UART

- P0.29 TX
- P0.30 RX
- 115200 baud

The bootloader UART is intentionally different from the application UART.

### I2C

- P0.16 — SDA
- P0.18 — SCL

### I2S slave RX

- P0.03 — SD / DI
- P0.02 — WS / LRCK
- P0.31 — SCK / BCLK

### Low-frequency clock

The GRASPion Head does not use an external 32.768 kHz crystal for the ANNA-B112. The variant therefore uses the nRF52832 internal LF RC oscillator (`USE_LFRC`).

## Arduino Boards Manager installation

After the first package release has been generated, add this URL to **Arduino IDE -> Preferences -> Additional Boards Manager URLs**:

```text
https://raw.githubusercontent.com/GRASP-LAB/GRASPion/main/package_graspion_index.json
```

Then open **Boards Manager**, search for **GRASPion**, install the package and select:

```text
GRASPion Head nRF52832
```

## Building a package release

The workflow `.github/workflows/build-graspion-nrf52-package.yml` creates the Boards Manager package.

From GitHub:

1. Open **Actions**.
2. Select **Build GRASPion nRF52 Arduino package**.
3. Select **Run workflow**.
4. Enter the package version, for example `1.0.0`.

The workflow:

1. downloads Adafruit nRF52 Arduino core 1.7.0;
2. appends the GRASPion Head board definition;
3. injects `variants/graspionHead`;
4. builds the `.tar.bz2` platform archive;
5. calculates its SHA-256 checksum and size;
6. creates or updates the GitHub release;
7. updates `package_graspion_index.json`.

## Source files

- `variants/graspionHead/variant.h` — board pin/peripheral definitions
- `variants/graspionHead/variant.cpp` — Arduino pin-to-nRF GPIO mapping and variant initialization
- `graspionHead.boards.txt` — board definition appended to Adafruit `boards.txt` during packaging
- `build_package.py` — package/index generator
