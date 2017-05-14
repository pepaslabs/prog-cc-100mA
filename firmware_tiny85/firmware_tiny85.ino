/*
  firmware_tiny85.ino: Arduino sketch for the programmable 0-100mA current source project.
  See https://github.com/pepaslabs/prog-cc-100mA
  Copyright Jason Pepas (Pepas Labs, LLC)
  Released under the terms of the MIT License.  See http://opensource.org/licenses/MIT
*/

/*
 This sketch requires ATTinyCore.  See installation instructions at:
 https://github.com/SpenceKonde/ATTinyCore/blob/master/Installation.md
 */

/*
Serial command examples:

'i': Set the output current to 5 milliamps:

    i5; or i5.0;

'c': Set the DAC output code to 37, no gain:

    c37;
    
'C': Set the DAC output code to 217, with 2x gain:

    C217;

'+': Increase the DAC output by one LSB:

    +;

'-': Decrease the DAC output by one LSB:

    -;

(Note: '+' and '-' do not alter the DAC gain boolean.)

'z': Calibrate the zero output to 0.05mA:

    z0.05;

'f': Calibrate full-scale output to 101.3mA:

    f101.3;

'?': Dump the current state:

    ?;

'd': Disable verbose debug output (default):

    d;

'D': Enable verbose debug output:

    D;
*/

/*
A note about DAC selection and the 'C' command:

The 'c' and 'C' commands are relative to the resolution of the DAC.

To set an MCP4801 (8-bit) DAC to max output current:

    C255;

To set an MCP4811 (10-bit) DAC to max output current:

    C1023;

To set an MCP4821 (12-bit) DAC to max output current:

    C4095;
*/    

// Hex file compiled using Arduino 1.8.2 for ATtiny85 is 7,132 bytes (of 8,192 max)

// Pull in uint8_t, uint16_t, etc.
#define __STDC_LIMIT_MACROS
#include <stdint.h>

// Pull in EEMEM, etc.
// see http://www.atmel.com/webdoc/avrlibcreferencemanual/group__avr__eeprom.html
#include <avr/eeprom.h>

#include "features.h"

#include <SoftwareSerial.h>

#include "buffer.h"
#include "pins.h"
#include "SPI_util.h"
#include "DAC_util.h"
#include "MCP4821.h"

#include "command.h"


// --- Pinouts / connections:


// ATtiny85 pinout:
//
//                            +--\/--+
//                       D5  -|1    8|-  Vcc   (to 5V)
//      TTL serial TX    D3  -|2    7|-  D2    SPI SCK (to DAC SCK, pin 3)
//      TTL serial RX    D4  -|3    6|-  D1    SPI SDO (to DAC SDI, pin 4)
//                      GND  -|4    5|-  D0    SPI CS (to DAC CS, pin 2)
//                            +------+

// MCP4801 pinout:
//
//                            +--\/--+
//             (to 5V)  Vcc  -|1    8|-  Vout
//   (to tiny85 pin 5)  ~CS  -|2    7|-  GND
//   (to tiny85 pin 7)  SCK  -|3    6|-  ~SHDN  (to 5V)
//   (to tiny85 pin 6)  SDI  -|4    5|-  ~LDAC  (to GND)
//                            +------+


// --- DAC / SPI:


// Uncomment one of the following to choose your DAC:
//DAC_config_t dac_config = MCP4801_config(); // 8-bit, 0.4mA output resolution
//DAC_config_t dac_config = MCP4811_config(); // 10-bit, 0.1mA output resolution
DAC_config_t dac_config = MCP4821_config(); // 12-bit, 0.025mA output resolution


DAC_data_t dac_data = { .config = &dac_config, .gain = false, .code = 0x0 };


SPI_bus_t spi_bus = { .MOSI_pin = ATTINY85_PIN_6,
                      .MISO_pin = PIN_NOT_CONNECTED,
                      .SCK_pin = ATTINY85_PIN_7,
                      .type = BITBANG
                    };


SPI_device_t spi_dac = { .bus = &spi_bus,
                         .CS_pin = ATTINY85_PIN_5,
                         .bit_order = MSBFIRST
                       };


// --- Serial interface:


#define RX_pin ATTINY85_PIN_3
#define TX_pin ATTINY85_PIN_2

SoftwareSerial serial(RX_pin, TX_pin);


// --- buffer


#define MIN_EXPECTED_BUFF_LEN 2 // "+;"
#define MAX_EXPECTED_BUFF_LEN 9 // "i102.400;"
#define BUFF_LEN (MAX_EXPECTED_BUFF_LEN + 1)

char buffer_bytes[BUFF_LEN];
char_buffer_t buffer = { .len = BUFF_LEN, .bytes = buffer_bytes };


// --- globals


// Default values.  These can be calibrated over the serial connection, and are
// remembered via EEPROM.

// This is the measured output current (in milliamps) when the DAC is at min output (code 0).
float zero = 0.0;

// This is the measured output current (in milliamps) when the DAC is at max output (e.g., code 255 for an 8-bit DAC).
float full_scale = 102.4;


// A correction factor to apply to any requested current setting, based on the calibration settings.
struct _correction_t
{
  float offset;
  float scale;
};
typedef _correction_t correction_t;

correction_t correction = { .offset = -(zero), .scale = 102.4 / (full_scale - zero) };


#ifdef HAS_RUNTIME_VERBOSITY_CONFIG
bool verbose = false;
#endif


// --- Strings:


const char *STRING_BOOT_MESSAGE = "prog-cc-100mA OK;";
const char *STRING_PARSED_CURRENT = "current: ";
const char *STRING_CORRECTED_CURRENT = "corrected: ";
const char *STRING_DAC_CODE = "code: ";
const char *STRING_ZERO = "zero: ";
const char *STRING_FULL_SCALE = "full scale: ";
const char *STRING_DAC_RESOLUTION = "resolution: ";
const char *STRING_DAC_GAIN = "gain: ";
const char *STRING_CORRECTION_OFFSET = "offset: ";
const char *STRING_CORRECTION_SCALE = "scale: ";
const char *STRING_OK = "OK";
const char *STRING_ERROR = "!E";

// --- setup():


void setup()
{
  serial.begin(9600); // 9600 8N1

  #ifdef HAS_BOOT_MESSAGE
  {
    delay(10);
    serial.println(STRING_BOOT_MESSAGE);
    serial.flush();
  }
  #endif
  
  #ifdef HAS_EEPROM_BACKED_CALIBRATION_VALUES
  {
    bootstrap_EEPROM();
  }
  #endif

  // calculate the correction factor
  recalculate_correction_factor(zero, full_scale, &correction);
  
  // setup soft SPI
  pinMode(spi_bus.MOSI_pin, OUTPUT);
  pinMode(spi_bus.SCK_pin, OUTPUT);
  pinMode(spi_dac.CS_pin, OUTPUT);
  
  digitalWrite(spi_dac.CS_pin, HIGH); // start with the DAC un-selected.

  clear_char_buffer(&buffer);
}


void loop()
{
  command_t command = read_command(&serial, &buffer);
  if (command >= END_OF_COMMANDS_SECTION)
  {
    #ifdef HAS_ERROR_PRINTING
    {
      printError(command);
    }
    #endif
    
    return;
  }

  error_t error;
  
  switch(command)
  {
    
    #ifdef HAS_INCREMENT_COMMAND
    case COMMAND_INCREMENT:
    {
      error = increment_output_current(&dac_data, &spi_dac);
      break;
    }
    #endif
  
    #ifdef HAS_DECREMENT_COMMAND
    case COMMAND_DECREMENT:
    {
      error = decrement_output_current(&dac_data, &spi_dac);
      break;
    }
    #endif
  
    #ifdef HAS_CURRENT_COMMAND
    case COMMAND_SET_CURRENT:
    {
      error = parse_and_run_current_command(&buffer, &dac_data, &spi_dac);
      break;
    }
    #endif
  
    #ifdef HAS_CODE_COMMAND
    case COMMAND_SET_CODE:
    {
      error = parse_and_run_code_command(&buffer, &dac_data, &spi_dac, false);
      break;
    }
    case COMMAND_SET_CODE_W_GAIN:
    {
      error = parse_and_run_code_command(&buffer, &dac_data, &spi_dac, true);
      break;
    }
    #endif

    #ifdef HAS_CALIBRATE_ZERO_COMMAND
    case COMMAND_CALIBRATE_ZERO:
    {
      error = parse_and_run_calibrate_zero_command(&buffer);
      break;
    }
    #endif
  
    #ifdef HAS_CALIBRATE_FULL_SCALE_COMMAND
    case COMMAND_CALIBRATE_FULL_SCALE:
    {
      error = parse_and_run_calibrate_full_scale_command(&buffer);
      break;
    }
    #endif
  
    #ifdef HAS_DUMP_COMMAND
    case COMMAND_DUMP:
    {
      error = dump(&dac_data);
      break;
    }
    #endif

    #ifdef HAS_RUNTIME_VERBOSITY_CONFIG
    case COMMAND_DISABLE_VERBOSE_DEBUG_OUTPUT:
    {
      verbose = false;
      error = OK_NO_ERROR;
      break;
    }
    case COMMAND_ENABLE_VERBOSE_DEBUG_OUTPUT:
    {
      verbose = true;
      error = OK_NO_ERROR;
      break;
    }
    #endif
    
    default:
    {
      error = ERROR_UNKNOWN_COMMAND;
      break;
    }
  }
  
  if (error != OK_NO_ERROR)
  {
    #ifdef HAS_ERROR_PRINTING
    {
      printError(error);
    }
    #endif
    
    return;
  }
  
  #ifdef HAS_SUCCESS_PRINTING
  {
    printSuccess(command);
  }
  #endif
}


// --- EEPROM


#ifdef HAS_EEPROM_BACKED_CALIBRATION_VALUES

uint8_t EEMEM eeprom_has_been_initialized_token_address;
#define EEPROM_HAS_BEEN_INITIALIZED_CODE 44

float EEMEM zero_EEPROM_address;
float EEMEM full_scale_EEPROM_address;

bool eeprom_has_been_initialized()
{
  uint8_t has_been_initialized_token = eeprom_read_byte(&eeprom_has_been_initialized_token_address);
  return (has_been_initialized_token == EEPROM_HAS_BEEN_INITIALIZED_CODE);
}


void initialize_eeprom()
{
  eeprom_write_byte(&eeprom_has_been_initialized_token_address, EEPROM_HAS_BEEN_INITIALIZED_CODE);
  eeprom_write_float(&zero_EEPROM_address, zero);
  eeprom_write_float(&full_scale_EEPROM_address, full_scale);  
}


void load_values_from_eeprom()
{
  zero = eeprom_read_float(&zero_EEPROM_address);
  full_scale = eeprom_read_float(&full_scale_EEPROM_address);

  // also update the correction factor
  recalculate_correction_factor(zero, full_scale, &correction);
}

void bootstrap_EEPROM()
{
  if (eeprom_has_been_initialized() == false)
  {
    initialize_eeprom();
  }
  else
  {
    load_values_from_eeprom();
  }
}

#endif // HAS_EEPROM_BACKED_CALIBRATION_VALUES


// --- Calibration / correction factor:


void recalculate_correction_factor(float zero, float full_scale, correction_t *correction) {
  correction->offset = -(zero);
  correction->scale = 102.4 / (full_scale - zero);
}


// --- Serial utility functions:


command_t read_command(SoftwareSerial *serial, char_buffer_t *buffer)
{
  // --- read the first character
  
  while(serial->available() == 0)
  {
    continue;
  }
    
  char ch = serial->read();

  error_t error = read_until_sentinel(serial, buffer, ';');
  if (error != OK_NO_ERROR)
  {
    return error;
  }
  
  switch(ch)
  {
    #ifdef HAS_INCREMENT_COMMAND
    case '+':
    {
      return COMMAND_INCREMENT;
    }
    #endif
    
    #ifdef HAS_DECREMENT_COMMAND
    case '-':
    {
      return COMMAND_DECREMENT;
    }
    #endif
    
    #ifdef HAS_CURRENT_COMMAND
    case 'i':
    {
      return COMMAND_SET_CURRENT;
    }
    #endif
    
    #ifdef HAS_CODE_COMMAND
    case 'c':
    {
      return COMMAND_SET_CODE;
    }
    case 'C':
    {
      return COMMAND_SET_CODE_W_GAIN;
    }
    #endif
    
    #ifdef HAS_CALIBRATE_ZERO_COMMAND
    case 'z':
    {
      return COMMAND_CALIBRATE_ZERO;
    }
    #endif
    
    #ifdef HAS_CALIBRATE_FULL_SCALE_COMMAND
    case 'f':
    {
      return COMMAND_CALIBRATE_FULL_SCALE;
    }
    #endif
    
    #ifdef HAS_DUMP_COMMAND
    case '?':
    {
      return COMMAND_DUMP;
    }
    #endif

    #ifdef HAS_RUNTIME_VERBOSITY_CONFIG
    case 'd':
    {
      return COMMAND_DISABLE_VERBOSE_DEBUG_OUTPUT;
    }
    case 'D':
    {
      return COMMAND_ENABLE_VERBOSE_DEBUG_OUTPUT;
    }
    #endif

    default:
    {
      return ERROR_UNKNOWN_COMMAND;
    }
  }
}


error_t read_until_sentinel(SoftwareSerial *serial, char_buffer_t *buffer, char sentinel)
{
  uint8_t num_chars_consumed = 0;
  char *buff_ptr = buffer->bytes;
  
  while(true)
  {
    while(serial->available() == 0)
    {
      continue;
    }
    
    *buff_ptr = serial->read();
    num_chars_consumed++;

    if (*buff_ptr == sentinel)
    {
      *buff_ptr = '\0';
      return OK_NO_ERROR;
    }
    else if (num_chars_consumed == buffer->len - 1)
    {
      *buff_ptr = '\0';
      return ERROR_BUFFER_FILLED_UP_BEFORE_SENTINEL_REACHED;
    }
    else
    {
      buff_ptr++;
      continue;
    }  
  }
}


// --- DAC functions:


void send_dac_data(DAC_data_t *dac_data, SPI_device_t *spi_dac)
{
  MCP4821_packet_t packet = dac_data_as_MCP4821_packet(dac_data);
  spi_write_MCP4821_packet(spi_dac, packet);
}


// --- Command implementations:


#ifdef HAS_INCREMENT_COMMAND
error_t increment_output_current(DAC_data_t *dac_data, SPI_device_t *spi_dac)
{
  if (dac_data_increment_code(dac_data) == false)
  {
    return ERROR_INCREMENT_WOULD_CAUSE_OVERFLOW;
  }

  send_dac_data(dac_data, spi_dac);
  return OK_NO_ERROR;
}
#endif


#ifdef HAS_DECREMENT_COMMAND
error_t decrement_output_current(DAC_data_t *dac_data, SPI_device_t *spi_dac)
{
  if (dac_data_decrement_code(dac_data) == false)
  {
    return ERROR_DECREMENT_WOULD_CAUSE_UNDERFLOW;
  }

  send_dac_data(dac_data, spi_dac);
  return OK_NO_ERROR;
}
#endif


#ifdef HAS_CURRENT_COMMAND
error_t parse_and_run_current_command(char_buffer_t *buffer, DAC_data_t *dac_data, SPI_device_t *spi_dac)
{
  float output_current = atof(buffer->bytes);

  #ifdef HAS_RUNTIME_VERBOSITY_CONFIG
  #ifdef HAS_CURRENT_COMMAND_DEBUGGING
  if (verbose == true)
  {
    serial.print(STRING_PARSED_CURRENT);
    serial.println(output_current, 4);
    serial.flush();
  }
  #endif
  #endif

  if (output_current < 0)
  {
    return ERROR_PARSED_CURRENT_OUTSIDE_SUPPORTED_RANGE;
  }

  // apply the correction factor to the requested output current
  output_current = (output_current + correction.offset) * correction.scale;
  if (output_current < 0) output_current = 0;
  if (output_current > 102.4) output_current = 102.4;

  #ifdef HAS_RUNTIME_VERBOSITY_CONFIG
  #ifdef HAS_CURRENT_COMMAND_DEBUGGING
  if (verbose == true)
  {
    serial.print(STRING_CORRECTED_CURRENT);
    serial.println(output_current, 4);
    serial.flush();
  }
  #endif
  #endif

  // We have a 10R shunt and a 2x divider
  // 102.4mA becomes a 1.024V drop across the shunt.  DAC needs to output 2.048V to achieve this.
  // 102.4 / 50 = 2.048
  float DAC_volts = output_current / 50.0;
    
  if (dac_data_set_voltage(dac_data, DAC_volts) == false)
  {
    return ERROR_PARSED_CURRENT_OUTSIDE_SUPPORTED_RANGE;
  }

  #ifdef HAS_RUNTIME_VERBOSITY_CONFIG
  #ifdef HAS_CURRENT_COMMAND_DEBUGGING
  if (verbose == true)
  {
    serial.print(STRING_DAC_CODE);
    serial.println(dac_data->code, 4);
    serial.flush();
  }
  #endif
  #endif
  
  MCP4821_packet_t packet = dac_data_as_MCP4821_packet(dac_data);
  spi_write_MCP4821_packet(spi_dac, packet);
  
  return OK_NO_ERROR;
}
#endif


#ifdef HAS_CODE_COMMAND
error_t parse_and_run_code_command(char_buffer_t *buffer, DAC_data_t *dac_data, SPI_device_t *spi_dac, bool use_gain)
{
  uint16_t new_code = atoi(buffer->bytes);

  #ifdef HAS_RUNTIME_VERBOSITY_CONFIG  
  #ifdef HAS_CODE_COMMAND_DEBUGGING
  if (verbose == true)
  {
    serial.print(STRING_DAC_CODE);
    serial.println(new_code);
    serial.flush();
  }
  #endif
  #endif

  if (dac_data_set_code(dac_data, new_code) == false)
  {
    return ERROR_PARSED_CODE_OUTSIDE_SUPPORTED_RANGE;
  }
  
  dac_data->gain = use_gain;
  
  send_dac_data(dac_data, spi_dac);
  return OK_NO_ERROR;
}
#endif


#ifdef HAS_CALIBRATE_ZERO_COMMAND
error_t parse_and_run_calibrate_zero_command(char_buffer_t *buffer)
{
  float new_zero = atof(buffer->bytes);
  
  #ifdef HAS_RUNTIME_VERBOSITY_CONFIG  
  #ifdef HAS_CALIBRATE_ZERO_COMMAND_DEBUGGING
  if (verbose == true)
  {
    serial.print(STRING_ZERO);
    serial.println(new_zero, 4);
    serial.flush();
  }
  #endif
  #endif

  // check that the requested new zero value is sane
  if (new_zero < -1.0 || new_zero > 1.0)
  {
    return ERROR_PARSED_ZERO_OUTSIDE_SUPPORTED_RANGE;
  }

  zero = new_zero;
  
  #ifdef HAS_EEPROM_BACKED_CALIBRATION_VALUES
  {
    eeprom_write_float(&zero_EEPROM_address, zero);  
  }
  #endif

  // also update the correction factor
  recalculate_correction_factor(zero, full_scale, &correction);
  
  return OK_NO_ERROR;  
}
#endif


#ifdef HAS_CALIBRATE_FULL_SCALE_COMMAND
error_t parse_and_run_calibrate_full_scale_command(char_buffer_t *buffer)
{
  float new_full_scale = atof(buffer->bytes);
  
  #ifdef HAS_RUNTIME_VERBOSITY_CONFIG
  #ifdef HAS_CALIBRATE_FULL_SCALE_COMMAND_DEBUGGING
  if (verbose == true)
  {
    serial.print(STRING_FULL_SCALE);
    serial.println(new_full_scale, 4);
    serial.flush();
  }
  #endif
  #endif

  // check that the requested new full-scale value is sane
  if (new_full_scale < (102.4 * 0.9) || new_full_scale > (102.4 * 1.1))
  {
    return ERROR_PARSED_FULL_SCALE_OUTSIDE_SUPPORTED_RANGE;
  }
  
  full_scale = new_full_scale;
  
  #ifdef HAS_EEPROM_BACKED_CALIBRATION_VALUES
  {
    eeprom_write_float(&full_scale_EEPROM_address, full_scale);
  }
  #endif

  // also update the correction factor
  recalculate_correction_factor(zero, full_scale, &correction);

  return OK_NO_ERROR;  
}
#endif


#ifdef HAS_ERROR_PRINTING
void printError(error_t error)
{
  serial.print(STRING_ERROR);
  serial.println(error);
  serial.flush();
}
#endif


#ifdef HAS_SUCCESS_PRINTING
void printSuccess(command_t command)
{
  serial.print(STRING_OK);
  serial.println(command);
  serial.flush();
}
#endif


#ifdef HAS_DUMP_COMMAND
error_t dump(DAC_data_t *dac_data)
{
  serial.print(STRING_DAC_RESOLUTION);
  serial.println(dac_config.resolution);
  serial.print(STRING_DAC_CODE);
  serial.println(dac_data->code);
  serial.print(STRING_DAC_GAIN);
  serial.println(dac_data->gain);
  serial.print(STRING_ZERO);
  serial.println(zero, 4);
  serial.print(STRING_FULL_SCALE);
  serial.println(full_scale, 4);
  serial.print(STRING_CORRECTION_OFFSET);
  serial.println(correction.offset, 4);
  serial.print(STRING_CORRECTION_SCALE);
  serial.println(correction.scale, 4);
  serial.flush();
  return OK_NO_ERROR;
}
#endif

