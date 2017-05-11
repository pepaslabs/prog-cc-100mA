/*
  DAC_util.h
  Copyright Jason Pepas (Pepas Labs, LLC)
  Released under the terms of the MIT License.  See http://opensource.org/licenses/MIT
*/

#include "DAC_util.h"


// --- DAC_config_t


uint16_t dac_config_max_code(DAC_config_t *dac_config)
{
  return dac_config->resolution - 1;
}

  
// --- DAC_data_t


bool dac_data_set_code(DAC_data_t *dac_data, uint16_t code)
{
  if (code > dac_config_max_code(dac_data->config))
  {
    return false; // failure: would truncate
  }

  dac_data->code = code;  
  return true; // success
}


bool dac_data_increment_code(DAC_data_t *dac_data)
{
  bool would_overflow = (dac_data->code == dac_config_max_code(dac_data->config));
  if (would_overflow == true)
  {
    return false; // failure: would overflow
  }
  
  dac_data->code++;
  return true; // success
}


bool dac_data_decrement_code(DAC_data_t *dac_data)
{
  bool would_underflow = (dac_data->code == 0);
  if (would_underflow == true)
  {
    return false; // failure: would underflow
  }
  
  dac_data->code--;
  return true; // success
}


bool dac_data_set_voltage(DAC_data_t *dac_data, float voltage)
{
  uint16_t new_code;
  
  if (voltage < dac_data->config->vref)
  {
    dac_data->gain = false;
    new_code = uint16_t(voltage / dac_data->config->vref * dac_data->config->resolution);
  }
  else
  {
    dac_data->gain = true;
    new_code = uint16_t(voltage / (dac_data->config->vref * 2) * dac_data->config->resolution);
  }
  
  return dac_data_set_code(dac_data, new_code);
}

