/*
  MCP4821.h
  Copyright Jason Pepas (Pepas Labs, LLC)
  Released under the terms of the MIT License.  See http://opensource.org/licenses/MIT
*/

#ifndef _MCP4821_H_
#define _MCP4821_H_

// Note: This library also works seamlessly with the MCP4811 (10-bit) and MCP4801 (8-bit)
// versions of this chip.

// MCP4821: a 12-bit SPI DAC in a DIP8 package.
// product page: http://www.microchip.com/wwwproducts/Devices.aspx?dDocName=en024015
// datasheet: http://ww1.microchip.com/downloads/en/DeviceDoc/22244B.pdf

// Price (Digikey.com, quantity of 1, as of 2015/3/29):
// MCP4821-E/P (12-bit, DIP8): $2.76
// MCP4811-E/P (10-bit, DIP8): $1.94
// MCP4801-E/P ( 8-bit, DIP8): $1.36

// MCP4821 pinout:
//
//         +--\/--+
//   Vcc  -|1    8|-  Vout
//   ~CS  -|2    7|-  GND
//   SCK  -|3    6|-  ~SHDN
//   SDI  -|4    5|-  ~LDAC
//         +------+


#include "DAC_util.h"
#include "SPI_util.h"


typedef uint16_t MCP4821_packet_t;


MCP4821_packet_t dac_data_as_MCP4821_packet(DAC_data_t *data);

void spi_write_MCP4821_packet(SPI_device_t *device, MCP4821_packet_t packet);


DAC_config_t MCP4801_config();
DAC_config_t MCP4811_config();
DAC_config_t MCP4821_config();


#endif

