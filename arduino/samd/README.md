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
2. keeps only the QT Py M0 and GRASPion Body board definitions;
3. keeps the upstream `qtpy_m0` variant and adds `graspionBody`;
4. builds `graspion-samd-VERSION.tar.bz2`;
5. creates or updates release `arduino-samd-VERSION`;
6. merges the SAMD platform into the common `package_graspion_index.json`
   without removing the nRF52 platform.
