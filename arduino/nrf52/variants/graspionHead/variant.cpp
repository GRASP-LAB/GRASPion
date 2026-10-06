#include "variant.h"

#include "wiring_constants.h"
#include "wiring_digital.h"
#include "nrf.h"

/*
 * Arduino pin number -> nRF52832 GPIO number
 *
 * Direct mapping:
 *
 * 0  -> P0.00
 * 1  -> P0.01
 * ...
 * 31 -> P0.31
 */
const uint32_t g_ADigitalPinMap[] =
{
   0,  1,  2,  3,  4,  5,  6,  7,
   8,  9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23,
  24, 25, 26, 27, 28, 29, 30, 31
};

void initVariant()
{
  pinMode(PIN_LED1, OUTPUT);
  digitalWrite(PIN_LED1, LED_STATE_ON);
}
