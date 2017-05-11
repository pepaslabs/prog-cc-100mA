/*
  SPI_util.h
  Copyright Jason Pepas (Pepas Labs, LLC)
  Released under the terms of the MIT License.  See http://opensource.org/licenses/MIT
*/

#include "features.h"

#include "SPI_util.h"

#include <Arduino.h>

#ifdef HAS_HARDWARE_SPI
#include <SPI.h>
#endif

#ifdef HAS_USI_SPI
#include "tinySPI.h"
#endif


void spi_write_buffer(SPI_device_t *device, uint8_buffer_t *buffer)
{
  digitalWrite(device->CS_pin, LOW);
  for (uint8_t i=0; i < buffer->len; i++)
  {
    switch(device->bus->type)
    {
      
      #ifdef HAS_HARDWARE_SPI
      case HARDWARE:
      {
        SPI.transfer(buffer->bytes[i]);
        break;
      }
      #endif

      #ifdef HAS_USI_SPI
      case USI:
      {
        SPI.transfer(buffer->bytes[i]);
        break;
      }
      #endif

      case BITBANG:
      default:
      {
        shiftOut(device->bus->MOSI_pin, device->bus->SCK_pin, device->bit_order, buffer->bytes[i]);
        break;
      }
      
    }
  }
  digitalWrite(device->CS_pin, HIGH);
}


SPI_bus_t ATmega328_hardware_SPI_bus()
{
  SPI_bus_t bus = { .MOSI_pin = ARDUINO_PIN_D11,
                    .MISO_pin = ARDUINO_PIN_D12,
                    .SCK_pin = ARDUINO_PIN_D13,
                   .type = HARDWARE
                  };
  return bus;
}


SPI_bus_t ATtiny85_USI_SPI_bus()
{
  SPI_bus_t bus = { .MOSI_pin = ATTINY85_PIN_6, // DO
                    .MISO_pin = ATTINY85_PIN_5, // DI
                    .SCK_pin = ATTINY85_PIN_7,  // USCK
                    .type = USI
                  };
  return bus;
}

             
