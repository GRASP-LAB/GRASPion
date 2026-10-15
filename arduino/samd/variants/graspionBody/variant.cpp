/*
  Copyright (c) 2014-2015 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
*/

#include "variant.h"
#include "Arduino.h"

/*
 * Pins descriptions
 */
const PinDescription g_APinDescription[] =
{
  { PORTA,  2, PIO_ANALOG, (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG), ADC_Channel0, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_2 }, // A0 / D0 / DAC / LIGHT_SENSOR
  { PORTA,  3, PIO_ANALOG, (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG), ADC_Channel1, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_3 }, // A1 / D1 / HEAD connector
  { PORTA,  4, PIO_ANALOG, (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG|PIN_ATTR_PWM|PIN_ATTR_TIMER), ADC_Channel4, PWM0_CH0, TCC0_CH0, EXTERNAL_INT_4 }, // A2 / D2 / BUZ_RIGHT
  { PORTA,  5, PIO_ANALOG, (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG|PIN_ATTR_PWM|PIN_ATTR_TIMER), ADC_Channel5, PWM0_CH1, TCC0_CH1, EXTERNAL_INT_5 }, // A3 / D3 / BUZ_LEFT

  // I2C
  { PORTA, 16, PIO_SERCOM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), No_ADC_Channel, PWM2_CH0, TCC2_CH0, EXTERNAL_INT_0 }, // D4 / SDA
  { PORTA, 17, PIO_SERCOM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), No_ADC_Channel, PWM2_CH1, TCC2_CH1, EXTERNAL_INT_1 }, // D5 / SCL

  // Infrared - SERCOM0
  { PORTA,  6, PIO_SERCOM_ALT, (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG|PIN_ATTR_PWM|PIN_ATTR_TIMER), ADC_Channel6, PWM1_CH0, TCC1_CH0, EXTERNAL_INT_6 }, // A6 / D6 / IR_TX
  { PORTA,  7, PIO_SERCOM_ALT, (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG|PIN_ATTR_PWM|PIN_ATTR_TIMER), ADC_Channel7, PWM1_CH1, TCC1_CH1, EXTERNAL_INT_7 }, // A7 / D7 / IR_RX

  // Head SPI/UART - SERCOM2
  { PORTA, 11, PIO_SERCOM_ALT, (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG|PIN_ATTR_PWM|PIN_ATTR_TIMER_ALT), ADC_Channel19, PWM0_CH3, TCC0_CH3, EXTERNAL_INT_11 }, // A8 / D8 / SCK
  { PORTA,  9, PIO_SERCOM_ALT, (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG|PIN_ATTR_PWM|PIN_ATTR_TIMER_ALT), ADC_Channel17, PWM1_CH3, TCC1_CH3, EXTERNAL_INT_9 },  // A9 / D9 / MISO / HEAD_RX
  { PORTA, 10, PIO_SERCOM_ALT, (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG|PIN_ATTR_PWM|PIN_ATTR_TIMER_ALT), ADC_Channel18, PWM0_CH2, TCC0_CH2, EXTERNAL_INT_10 }, // A10 / D10 / MOSI / HEAD_TX

  { PORTA, 18, PIO_DIGITAL, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_2 },  // D11 / PIN_NEOPIXEL
  { PORTA, 15, PIO_DIGITAL, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_15 }, // D12 / BUZ_ENABLE

  // Fake D13 retained for core compatibility
  { PORTA, 27, PIO_OUTPUT, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },

  // SPI1 - on-board flash
  { PORTA, 23, PIO_SERCOM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER_ALT), No_ADC_Channel, PWM0_CH5, TCC0_CH5, EXTERNAL_INT_7 }, // D14 / SCK1
  { PORTA, 19, PIO_SERCOM_ALT, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), No_ADC_Channel, PWM3_CH1, TC3_CH1, EXTERNAL_INT_3 }, // D15 / MISO1
  { PORTA, 22, PIO_SERCOM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER_ALT), No_ADC_Channel, PWM0_CH4, TCC0_CH4, EXTERNAL_INT_6 }, // D16 / MOSI1
  { PORTA,  8, PIO_SERCOM_ALT, (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG|PIN_ATTR_PWM|PIN_ATTR_TIMER_ALT), ADC_Channel16, PWM1_CH2, TCC1_CH2, EXTERNAL_INT_NMI }, // D17 / FLASH_CS

  // USB
  { PORTA, 28, PIO_COM, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // USB Host enable
  { PORTA, 24, PIO_COM, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // USB DM
  { PORTA, 25, PIO_COM, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // USB DP
};

const void* g_apTCInstances[TCC_INST_NUM+TC_INST_NUM] =
{
  TCC0, TCC1, TCC2, TC3, TC4, TC5
};

// SERCOM objects
SERCOM sercom0(SERCOM0);
SERCOM sercom1(SERCOM1);
SERCOM sercom2(SERCOM2);
SERCOM sercom3(SERCOM3);

// IR link
Uart SerialIR(
  &sercom0,
  PIN_SERIALIR_RX,
  PIN_SERIALIR_TX,
  PAD_SERIALIR_RX,
  PAD_SERIALIR_TX
);

void SERCOM0_Handler()
{
  SerialIR.IrqHandler();
}

// Head link
Uart SerialHead(
  &sercom2,
  PIN_SERIALHEAD_RX,
  PIN_SERIALHEAD_TX,
  PAD_SERIALHEAD_RX,
  PAD_SERIALHEAD_TX
);

void SERCOM2_Handler()
{
  SerialHead.IrqHandler();
}

/*
 * Board-specific startup
 */
void initVariant(void)
{
  // Keep the motor driver disabled from the earliest possible point.
  // Do not initialize PWM, UART, I2C or NeoPixels here.
  digitalWrite(BUZ_ENABLE, LOW);
  pinMode(BUZ_ENABLE, OUTPUT);
  analogWriteResolution(8);
}

/*
 * GRASPion short ID
 *
 * Hashes the complete factory-programmed 128-bit SAMD21 serial number
 * with FNV-1a, retains 30 bits and encodes them as 5 URL-safe Base64
 * characters.
 */
void getGRASPionID(char id[6])
{
  const uint32_t uid[4] =
  {
    *(const uint32_t *)0x0080A00C,
    *(const uint32_t *)0x0080A040,
    *(const uint32_t *)0x0080A044,
    *(const uint32_t *)0x0080A048
  };

  uint32_t hash = 2166136261u;
  const uint8_t *data = (const uint8_t *)uid;

  for (uint8_t i = 0; i < 16; i++)
  {
    hash ^= data[i];
    hash *= 16777619u;
  }

  hash &= 0x3FFFFFFFu; // 30 bits

  static const char base64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789-_";

  for (int8_t i = 4; i >= 0; i--)
  {
    id[i] = base64[hash & 0x3F];
    hash >>= 6;
  }

  id[5] = '\0';
}
