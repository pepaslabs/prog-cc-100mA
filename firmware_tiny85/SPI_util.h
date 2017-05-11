/*
  SPI_util.h
  Copyright Jason Pepas (Pepas Labs, LLC)
  Released under the terms of the MIT License.  See http://opensource.org/licenses/MIT
*/

#ifndef _SPI_UTIL_H_
#define _SPI_UTIL_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include "pins.h"
#include "buffer.h"


enum _SPI_bus_type_t
{
  HARDWARE=0,
  BITBANG=1,
  USI=2
};
typedef enum _SPI_bus_type_t SPI_bus_type_t;


typedef uint8_t pin_t;
#define PIN_NOT_CONNECTED UINT8_MAX


struct _SPI_bus_t
{
  pin_t MOSI_pin;
  pin_t MISO_pin;
  pin_t SCK_pin;
  SPI_bus_type_t type;
};
typedef _SPI_bus_t SPI_bus_t;


struct _SPI_device_t
{
  SPI_bus_t *bus;
  pin_t CS_pin;
  uint8_t bit_order; // either LSBFIRST or MSBFIRST (from Arduino.h)
};
typedef _SPI_device_t SPI_device_t;


// --- hardware SPI buses:

SPI_bus_t ATmega328_hardware_SPI_bus();
SPI_bus_t ATtiny85_USI_SPI_bus();


// --- functions:

void spi_write_buffer(SPI_device_t *device, uint8_buffer_t *buffer);


#endif

