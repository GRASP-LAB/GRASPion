# GRASPion SAMD Arduino package

This directory builds the Arduino Boards Manager package for the GRASPion Body.

The package is based on **Adafruit ArduinoCore-samd 1.7.17** and intentionally
exposes only:

- Adafruit QT Py M0 (SAMD21)
- GRASPion Body (SAMD21)

The GRASPion Body uses the QT Py M0 bootloader and pin architecture, with a
dedicated `graspionBody` variant that adds board-specific names such as
`BUZ_LEFT`, `BUZ_RIGHT`, `BUZ_ENABLE`, `IR_TX`, `IR_RX`, `LIGHT_SENSOR`,
NeoPixel indices and `MLX90393_I2C_ADDRESS`.

Unlike the upstream QT Py M0 variant, D12 is **not** driven high during
`initVariant()`: on GRASPion Body D12 is `BUZ_ENABLE`.

## Core dependencies

ArduinoCore-samd 1.7.17 references two libraries as git submodules. GitHub
source archives do not include their contents, so the builder downloads the
exact commits used by Adafruit 1.7.17:

- Adafruit_TinyUSB_Arduino: `c953968c468218d8968138b5ed8482fa565373df`
- Adafruit_ZeroDMA: `acc5dadb458b2c329757a61dc4f18dda945a0c36`

The build fails if the expected headers are missing from the generated package.

Application libraries such as Adafruit NeoPixel, Adafruit MLX90393, Adafruit
BusIO and Adafruit Unified Sensor are intentionally not bundled in the core;
they should be installed through Arduino Library Manager when needed.

## Local build

From the repository root:

```bash
python3 arduino/samd/build_package.py --version 1.0.0
```

The generated files are written to `dist/`.

## Release

Run the GitHub Actions workflow **Build GRASPion SAMD Arduino package** and set
the desired package version. The workflow:

1. downloads Adafruit ArduinoCore-samd 1.7.17;
2. downloads the exact TinyUSB and ZeroDMA submodule commits used by that core;
3. keeps only the QT Py M0 and GRASPion Body board definitions;
4. keeps the upstream `qtpy_m0` variant and adds `graspionBody`;
5. validates the generated archive contents;
6. builds `graspion-samd-VERSION.tar.bz2`;
7. creates or updates release `arduino-samd-VERSION`;
8. merges the SAMD platform into the common `package_graspion_index.json`
   without removing the nRF52 platform.
