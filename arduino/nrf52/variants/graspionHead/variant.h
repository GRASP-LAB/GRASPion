#ifndef _VARIANT_GRASPIONHEAD_
#define _VARIANT_GRASPIONHEAD_

/*
 * Graspion Head
 * ANNA-B112 / nRF52832
 */

#define VARIANT_MCK (64000000ul)

/*
 * No external 32.768 kHz crystal on Graspion Head.
 * Use internal LF RC oscillator.
 */
#define USE_LFRC

#include "WVariant.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*------------------------------------------------------------------*/
/* Pins
 *------------------------------------------------------------------*/

#define PINS_COUNT           (32u)
#define NUM_DIGITAL_PINS     (32u)
#define NUM_ANALOG_INPUTS    (8u)
#define NUM_ANALOG_OUTPUTS   (0u)

/*------------------------------------------------------------------*/
/* LED
 *
 * P0.15
 *------------------------------------------------------------------*/

#define PIN_LED1             (15)
#define LED_BUILTIN          PIN_LED1
#define LED_STATE_ON         1

/*------------------------------------------------------------------*/
/* Analog inputs
 *
 * nRF52832 SAADC:
 *
 * AIN0 = P0.02
 * AIN1 = P0.03
 * AIN2 = P0.04
 * AIN3 = P0.05
 * AIN4 = P0.28
 * AIN5 = P0.29
 * AIN6 = P0.30
 * AIN7 = P0.31
 *
 * Some of these pins are also assigned to other peripherals on the
 * Graspion Head PCB.
 *------------------------------------------------------------------*/

#define PIN_A0               (2)
#define PIN_A1               (3)
#define PIN_A2               (4)
#define PIN_A3               (5)
#define PIN_A4               (28)
#define PIN_A5               (29)
#define PIN_A6               (30)
#define PIN_A7               (31)

static const uint8_t A0 = PIN_A0;
static const uint8_t A1 = PIN_A1;
static const uint8_t A2 = PIN_A2;
static const uint8_t A3 = PIN_A3;
static const uint8_t A4 = PIN_A4;
static const uint8_t A5 = PIN_A5;
static const uint8_t A6 = PIN_A6;
static const uint8_t A7 = PIN_A7;

#define ADC_RESOLUTION       14

/*------------------------------------------------------------------*/
/* NFC
 *------------------------------------------------------------------*/

#define PIN_NFC1             (9)
#define PIN_NFC2             (10)

/*------------------------------------------------------------------*/
/* Application UART <-> STM32U575
 *
 * nRF P0.27 TX -> STM32 PA3 / LPUART1_RX
 * nRF P0.28 RX <- STM32 PA2 / LPUART1_TX
 *
 * IMPORTANT:
 * Bootloader DFU UART uses different pins:
 *
 * nRF P0.29 TX
 * nRF P0.30 RX
 *------------------------------------------------------------------*/

#define PIN_SERIAL_TX        (27)
#define PIN_SERIAL_RX        (28)

/*------------------------------------------------------------------*/
/* I2C
 *
 * SDA = P0.16
 * SCL = P0.18
 *------------------------------------------------------------------*/

#define WIRE_INTERFACES_COUNT 1

#define PIN_WIRE_SDA         (16u)
#define PIN_WIRE_SCL         (18u)

static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;

/*------------------------------------------------------------------*/
/* SPI
 *
 * No default SPI bus assigned yet.
 *------------------------------------------------------------------*/

#define SPI_INTERFACES_COUNT 0

/*------------------------------------------------------------------*/
/* I2S - slave RX only
 *
 * DI / SD   = P0.03
 * WS / LRCK = P0.02
 * SCK/BCLK  = P0.31
 *
 * MCK is not used.
 *------------------------------------------------------------------*/

#define I2S_INTERFACES_COUNT 1

#define PIN_I2S_SD           (3u)
#define PIN_I2S_FS           (2u)
#define PIN_I2S_SCK          (31u)

#ifdef __cplusplus
}
#endif

#endif // _VARIANT_GRASPIONHEAD_
