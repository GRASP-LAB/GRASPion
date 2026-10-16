/*
  Copyright (c) 2014-2015 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
*/

#ifndef _VARIANT_GRASPION_BODY_
#define _VARIANT_GRASPION_BODY_

#define ARDUINO_SAMD_VARIANT_COMPLIANCE 10610

#define VARIANT_MAINOSC        (32768ul)
#define VARIANT_MCK            (F_CPU)

#include "WVariant.h"

#ifdef __cplusplus
#include "SERCOM.h"
#include "Uart.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*----------------------------------------------------------------------------
 * Pins
 *----------------------------------------------------------------------------*/

#define PINS_COUNT           (21u)
#define NUM_DIGITAL_PINS     (21u)
#define NUM_ANALOG_INPUTS    (9u)
#define NUM_ANALOG_OUTPUTS   (1u)

#define analogInputToDigitalPin(p)  (p)

#define digitalPinToPort(P)        ( &(PORT->Group[g_APinDescription[P].ulPort]) )
#define digitalPinToBitMask(P)     ( 1 << g_APinDescription[P].ulPin )
#define portOutputRegister(port)   ( &(port->OUT.reg) )
#define portInputRegister(port)    ( &(port->IN.reg) )
#define portModeRegister(port)     ( &(port->DIR.reg) )

#define digitalPinHasPWM(P) \
  ( g_APinDescription[P].ulPWMChannel != NOT_ON_PWM || \
    g_APinDescription[P].ulTCChannel != NOT_ON_TIMER )

/*----------------------------------------------------------------------------
 * LEDs / NeoPixels
 *----------------------------------------------------------------------------*/

// Fake D13 kept for QT Py / SAMD core compatibility
#define PIN_LED_13           (13u)
#define PIN_LED              PIN_LED_13
#define LED_BUILTIN          PIN_LED_13

// GRASPion Body NeoPixel chain
#define PIN_NEOPIXEL         (11u)   // PA18
#define NUM_NEOPIXELS        (2u)
#define NEOPIXEL_BOTTOM      (0u)
#define NEOPIXEL_TOP         (1u)

/*----------------------------------------------------------------------------
 * GRASPion Body hardware
 *----------------------------------------------------------------------------*/

// Vibromotors
#define BUZ_RIGHT            (2u)    // PA04
#define BUZ_LEFT             (3u)    // PA05
#define BUZ_ENABLE           (12u)   // PA15

// Infrared interface
#define IR_TX                (6u)    // PA06 -> AVR32DD20 carrier modulator
#define IR_RX                (7u)    // PA07 <- TSOP75438

// Head connector UART
#define HEAD_RX              (9u)    // PA09 / shared with SPI MISO
#define HEAD_TX              (10u)   // PA10 / shared with SPI MOSI

// Ambient light sensor
#define LIGHT_SENSOR         (A0)    // PA02

// On-board magnetometer
#define MLX90393_I2C_ADDRESS (0x10) // MLX90393ELW-ABA-012-RE

/*----------------------------------------------------------------------------
 * Analog pins
 *----------------------------------------------------------------------------*/

#define PIN_A0               (0ul)
#define PIN_A1               (PIN_A0 + 1)
#define PIN_A2               (PIN_A0 + 2)
#define PIN_A3               (PIN_A0 + 3)
#define PIN_A6               (PIN_A0 + 6)
#define PIN_A7               (PIN_A0 + 7)
#define PIN_A8               (PIN_A0 + 8)
#define PIN_A9               (PIN_A0 + 9)
#define PIN_A10              (PIN_A0 + 10)
#define PIN_DAC0             (0ul)

static const uint8_t A0   = PIN_A0;
static const uint8_t A1   = PIN_A1;
static const uint8_t A2   = PIN_A2;
static const uint8_t A3   = PIN_A3;
static const uint8_t A6   = PIN_A6;
static const uint8_t A7   = PIN_A7;
static const uint8_t A8   = PIN_A8;
static const uint8_t A9   = PIN_A9;
static const uint8_t A10  = PIN_A10;
static const uint8_t DAC0 = PIN_DAC0;

#define ADC_RESOLUTION        12

/*----------------------------------------------------------------------------
 * Serial interfaces
 *----------------------------------------------------------------------------*/

// Infrared UART - SERCOM0
#define PIN_SERIALIR_TX       IR_TX
#define PIN_SERIALIR_RX       IR_RX
#define PAD_SERIALIR_TX       UART_TX_PAD_2
#define PAD_SERIALIR_RX       SERCOM_RX_PAD_3

// Head UART - SERCOM2
#define PIN_SERIALHEAD_TX     HEAD_TX
#define PIN_SERIALHEAD_RX     HEAD_RX
#define PAD_SERIALHEAD_TX     UART_TX_PAD_2
#define PAD_SERIALHEAD_RX     SERCOM_RX_PAD_1

/*----------------------------------------------------------------------------
 * SPI
 *
 * QT Py compatible SPI mapping is retained.
 * SerialHead shares PA09/PA10 and SERCOM2 with this SPI interface,
 * therefore SerialHead and SPI must not be used simultaneously.
 *----------------------------------------------------------------------------*/

#define SPI_INTERFACES_COUNT 2

#define PIN_SPI_SCK          (8u)    // PA11
#define PIN_SPI_MISO         (9u)    // PA09 / HEAD_RX
#define PIN_SPI_MOSI         (10u)   // PA10 / HEAD_TX
#define PERIPH_SPI           sercom2
#define PAD_SPI_TX           SPI_PAD_2_SCK_3
#define PAD_SPI_RX           SERCOM_RX_PAD_1

static const uint8_t SS   = PIN_A0;  // unused, retained for compatibility
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK  = PIN_SPI_SCK;

// On-board SPI flash
#define PIN_SPI1_SCK          (14u)  // PA23
#define PIN_SPI1_MISO         (15u)  // PA19
#define PIN_SPI1_MOSI         (16u)  // PA22
#define PERIPH_SPI1           sercom3
#define PAD_SPI1_TX           SPI_PAD_0_SCK_1
#define PAD_SPI1_RX           SERCOM_RX_PAD_3

static const uint8_t SS1   = 17;     // PA08 / FLASH_CS
static const uint8_t MOSI1 = PIN_SPI1_MOSI;
static const uint8_t MISO1 = PIN_SPI1_MISO;
static const uint8_t SCK1  = PIN_SPI1_SCK;

#define EXTERNAL_FLASH_DEVICES  GD25Q16C
#define EXTERNAL_FLASH_USE_SPI  SPI1
#define EXTERNAL_FLASH_USE_CS   SS1

/*----------------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------------*/

#define WIRE_INTERFACES_COUNT 1

#define PIN_WIRE_SDA         (4u)    // PA16
#define PIN_WIRE_SCL         (5u)    // PA17
#define PERIPH_WIRE          sercom1
#define WIRE_IT_HANDLER      SERCOM1_Handler

static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;

/*----------------------------------------------------------------------------
 * USB
 *----------------------------------------------------------------------------*/

#define PIN_USB_HOST_ENABLE  (18ul)
#define PIN_USB_DM           (19ul)
#define PIN_USB_DP           (20ul)

/*----------------------------------------------------------------------------
 * I2S
 *----------------------------------------------------------------------------*/

#define I2S_INTERFACES_COUNT 1

#define I2S_DEVICE           0
#define I2S_CLOCK_GENERATOR  3
#define PIN_I2S_SD           (A7)
#define PIN_I2S_SCK          (MOSI)
#define PIN_I2S_FS           (SCK)

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
 * Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus

extern SERCOM sercom0;
extern SERCOM sercom1;
extern SERCOM sercom2;
extern SERCOM sercom3;
extern SERCOM sercom4;
extern SERCOM sercom5;

extern Uart SerialIR;
extern Uart SerialHead;

// Returns a stable 5-character ID derived from the complete
// factory-programmed 128-bit SAMD21 serial number.
void getGRASPionID(char id[6]);

#endif

#define SERIAL_PORT_USBVIRTUAL      Serial
#define SERIAL_PORT_MONITOR         Serial
#define SERIAL_PORT_HARDWARE        SerialHead
#define SERIAL_PORT_HARDWARE_OPEN   SerialHead

#endif /* _VARIANT_GRASPION_BODY_ */
