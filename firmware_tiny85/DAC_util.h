/*
  DAC_util.h
  Copyright Jason Pepas (Pepas Labs, LLC)
  Released under the terms of the MIT License.  See http://opensource.org/licenses/MIT
*/

#ifndef _DAC_UTIL_H_
#define _DAC_UTIL_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>


// --- DAC_config_t


struct _DAC_config_t
{
  uint8_t num_bits;
  uint16_t resolution;
  float vref;
};
typedef _DAC_config_t DAC_config_t;


uint16_t dac_config_max_code(DAC_config_t *dac_config);


// --- DAC_data_t


struct _DAC_data_t
{
  DAC_config_t *config;
  bool gain;
  uint16_t code;
};
typedef struct _DAC_data_t DAC_data_t;


bool dac_data_set_code(DAC_data_t *dac_data, uint16_t code);

bool dac_data_increment_code(DAC_data_t *dac_data);
bool dac_data_decrement_code(DAC_data_t *dac_data);

bool dac_data_set_voltage(DAC_data_t *dac_data, float voltage);


#endif

