# GRASPion
The GRASPion is an intelligent bristlebot designed for research and education purposes around the
SAMD21E18A microcontroller and the Arduino environment. It relies on two vibrating motors embedded
in the bot to move around and can execute controlled trajectories by moving straight or turning,
making it a platform for the experimental study of active matter.

<img width="1024" height="620" alt="image" src="https://github.com/user-attachments/assets/87e0c45d-7d9f-4518-a012-564adfe0e1b0" />

The physical properties of the bot and the results of the examples in this repository are given in
our technical paper (see here). The GRASPion, with a printed and fully assembled circuit board, 3D
printed body, battery, motors and legs is available for order through
[graspion.be](https://graspion.be).

## 3D printing

The bot is made out of three 3D printed objects: the chassis, the legs and the wedge plate in
between them. The STL files can be found in the printing directory.

### The chassis

The chassis (or body) is printed out of ABS plastic using the Stratasys J35 PolyJet printer. This
allows for the resolution needed to properly fit the vibrating motors and battery while providing a
hard body able to handle collisions.

### The legs

The legs and wedge plate are printed out of PLA. Prusament PLA Prusa Galaxy gives consistently
reproducible results using the Prusa XL printer, Structural setting and 15% infill. It is important
that the legs are printed one-by-one rather than multiple legs at the same time, since this changes
layer adhesion and therefore leg flexibility.

## Arduino setup

The GRASPion Body is based on the **SAMD21E18A** and keeps the Arduino pin mapping of the
**Adafruit QT Py M0** where practical. For new development, select the dedicated board definition:

```text
GRASPion Body (SAMD21)
```

This keeps existing QT Py M0 sketches largely compatible while exposing names that correspond to
the actual GRASPion hardware.

### 1. Install the GRASPion board package

Open the Arduino IDE preferences and add the following URL to **Additional Boards Manager URLs**:

```text
https://raw.githubusercontent.com/GRASP-LAB/GRASPion/main/package_graspion_index.json
```

Then open the **Boards Manager**, search for **GRASPion**, install the GRASPion SAMD package and
select:

```text
GRASPion Body (SAMD21)
```

Recommended settings:

| Setting | Value |
| --- | --- |
| Board | GRASPion Body (SAMD21) |
| Optimization | Ofast |
| USB Stack | TinyUSB |

### GRASPion Body API

The dedicated variant provides readable names for the main hardware functions:

```cpp
BUZ_LEFT
BUZ_RIGHT
BUZ_ENABLE

IR_TX
IR_RX
SerialIR

HEAD_TX
HEAD_RX
SerialHead

LIGHT_SENSOR

PIN_NEOPIXEL
NUM_NEOPIXELS
NEOPIXEL_BOTTOM
NEOPIXEL_TOP

MLX90393_I2C_ADDRESS

getGRASPionID(id)
```

`getGRASPionID()` returns a stable **5-character URL-safe Base64 ID** derived from the complete
factory-programmed 128-bit SAMD21 unique serial number. The UID is hashed to 30 bits before
encoding; the short ID is intended as a convenient robot identifier, not as a replacement for the
full 128-bit UID when mathematically guaranteed uniqueness is required.

The variant also:

- disables the vibration-motor driver during startup;
- sets Arduino PWM writes to 8-bit resolution, so the motor neutral value is `127`;
- keeps the QT Py M0 SPI mapping for compatibility.

### Bundled libraries

A fresh installation of the GRASPion SAMD package already includes the libraries used by the main
GRASPion examples. No separate Library Manager installation is required for the standard setup.

| Library | Bundled version |
| --- | ---: |
| Adafruit NeoPixel | 1.15.5 |
| Adafruit MLX90393 | 2.0.5 |
| Adafruit BusIO | 1.17.4 |
| Adafruit Unified Sensor | 1.1.15 |
| Arduino-IRremote | 4.7.1 |

The bundled copies are installed inside the GRASPion platform. Advanced users can still install a
newer version through Arduino Library Manager when needed; a user-installed copy can override the
bundled one.

### Infrared interface

The GRASPion Body has dedicated infrared transmit and receive hardware.

```cpp
SerialIR.begin(2400);
```

uses:

- `IR_TX` / PA06 to send the unmodulated data envelope to the on-board **AVR32DD20**;
- the AVR32DD20 to generate the **38 kHz carrier** and drive the IR LED;
- `IR_RX` / PA07 to receive the demodulated signal from the **TSOP75438** IR receiver.

For **Arduino-IRremote**, the GRASPion board definition automatically defines
`USE_NO_SEND_PWM`. IRremote therefore generates only the modulation envelope on `IR_TX`; the
AVR32DD20 generates the physical 38 kHz carrier.

Because the transmitter and receiver are on the same robot, a GRASPion can optically receive its
own IR transmission. Applications using addressed packets should ignore packets containing their
own robot ID when appropriate.

### Head interface

The head connector provides a dedicated UART:

```cpp
SerialHead.begin(115200);
```

with:

```cpp
HEAD_RX   // PA09
HEAD_TX   // PA10
```

For QT Py M0 compatibility, PA09/PA10/PA11 are still available as `MISO`, `MOSI` and `SCK`.
`SerialHead` and the standard `SPI` interface share **SERCOM2** and must therefore not be used at
the same time.

### NeoPixels

The GRASPion Body contains two addressable RGB LEDs connected as one chain:

```cpp
PIN_NEOPIXEL
NUM_NEOPIXELS
NEOPIXEL_BOTTOM
NEOPIXEL_TOP
```

The data output of the top Body NeoPixel is also routed to the Head connector, allowing an optional
Head NeoPixel to be chained after the two Body LEDs.

### Sensors and board information

The ambient light sensor is available through:

```cpp
analogRead(LIGHT_SENSOR);
```

The on-board **MLX90393** magnetic-field sensor is connected over I2C at:

```cpp
MLX90393_I2C_ADDRESS
```

With the Adafruit MLX90393 library, `readData()` returns the X, Y and Z magnetic flux-density
components in **microteslas (uT)**.

The on-board AVR32DD20 also exposes board power information over I2C address `0x36`. A read of five
bytes returns the measured USB-bus voltage and battery voltage used by the examples.

### Uploading firmware

Connect the GRASPion Body to the computer using USB-C and select the corresponding serial port in
the Arduino IDE. The battery is not required for programming when the board is powered through USB.

If the board has been fully disconnected from both the battery and external power, reconnecting it
to a powered USB-C cable or charger may be required before normal operation resumes.

## Code examples

### HelloGRASPion

`HelloGRASPion` is installed with the GRASPion SAMD package and appears directly in the Arduino IDE
examples menu. The source is also kept in this repository as `examples/HelloGRASPion.ino`.

It demonstrates, in one small human-readable program:

- the 5-character robot ID;
- USB serial diagnostics;
- ambient-light measurement;
- MLX90393 magnetic-field measurement;
- USB and battery voltage reporting from the AVR32DD20;
- `SerialIR`;
- `SerialHead`;
- both Body NeoPixels;
- both vibration motors.

The USB console reports values in explicit scientific units, for example:

```text
GRASPion #z_kzN | Light=40 | B[uT]=(-12.4, 38.1, 4.7) | VBUS=4987 mV | VBAT=4012 mV | USB=yes
```

Older examples in this repository may still use direct QT Py M0 pin names. The dedicated
`GRASPion Body (SAMD21)` definition is recommended for new code.

An upload script is also included to facilitate programming multiple robots through a USB hub. The
upload script relies on [arduino-cli](https://github.com/arduino/arduino-cli), which can easily be
installed on Linux distributions.
