/*
  MCP4821.cpp
  Copyright Jason Pepas (Pepas Labs, LLC)
  Released under the terms of the MIT License.  See http://opensource.org/licenses/MIT
*/

#include "MCP4821.h"
#include "buffer.h"

#define MCP48X1_SHUTDOWN_BIT_POSITION 12
#define MCP48X1_GAIN_BIT_POSITION 13


MCP4821_packet_t dac_data_as_MCP4821_packet(DAC_data_t *dac_data)
{
  MCP4821_packet_t packet = dac_data->code;
  
  uint8_t num_needed_bit_shifts = 12 - dac_data->config->num_bits;
  packet = packet << num_needed_bit_shifts;
  
  if (packet > 0xFFF) packet = 0xFFF;
  
  // configure the ~shutdown bit
  packet |= 1 << MCP48X1_SHUTDOWN_BIT_POSITION;

  // configure the gain bit
  if (dac_data->gain == false)
  {
    packet |= 1 << MCP48X1_GAIN_BIT_POSITION;
  }

  return packet;
}


void spi_write_MCP4821_packet(SPI_device_t *device, MCP4821_packet_t packet)
{
  uint8_t bytes[2] = { highByte(packet), lowByte(packet) };
  uint8_buffer_t buffer = { .len = 2, .bytes = bytes };
  spi_write_buffer(device, &buffer);
}


DAC_config_t MCP4801_config()
{
  DAC_config_t cfg = { .num_bits = 8, .resolution = 256, .vref = 2.048 };
  return cfg;
}


DAC_config_t MCP4811_config()
{
  DAC_config_t cfg = { .num_bits = 10, .resolution = 1024, .vref = 2.048 };
  return cfg;
}


DAC_config_t MCP4821_config()
{
  DAC_config_t cfg = { .num_bits = 12, .resolution = 4096, .vref = 2.048 };
  return cfg;
}


// thanks to https://github.com/cversek/multimeter for hints on the SPI interface.
// see https://github.com/cversek/multimeter/blob/master/arduino/MultimeterLib/MCP4801.cpp


