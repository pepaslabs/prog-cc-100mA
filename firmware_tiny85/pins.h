/*
  pins.h
  Copyright Jason Pepas (Pepas Labs, LLC)
  Released under the terms of the MIT License.  See http://opensource.org/licenses/MIT
*/

#ifndef _PINS_H_
#define _PINS_H_

#include <Arduino.h>


// Arduino logical pin numbering:

#define ARDUINO_PIN_D0 0
#define ARDUINO_PIN_D1 1
#define ARDUINO_PIN_D2 2
#define ARDUINO_PIN_D3 3
#define ARDUINO_PIN_D4 4
#define ARDUINO_PIN_D5 5
#define ARDUINO_PIN_D6 6
#define ARDUINO_PIN_D7 7
#define ARDUINO_PIN_D8 8
#define ARDUINO_PIN_D9 9
#define ARDUINO_PIN_D10 10
#define ARDUINO_PIN_D11 11
#define ARDUINO_PIN_D12 12
#define ARDUINO_PIN_D13 13

#define ARDUINO_PIN_A0 A0
#define ARDUINO_PIN_A1 A1
#define ARDUINO_PIN_A2 A2
#define ARDUINO_PIN_A3 A3
#define ARDUINO_PIN_A4 A4
#define ARDUINO_PIN_A5 A5


// ATmega328 (PDIP) pinout:
//
//                            +---\/---+
//     (PCINT14/!RESET) PC6  -|1     28|-  PC5 (ADC5/SCL/PCINT13)
//        (PCINT16/RXD) PD0  -|2     27|-  PC4 (ADC4/SDA/PCINT12)
//        (PCINT17/TXD) PD1  -|3     26|-  PC3 (ADC3/PCINT11)
//       (PCINT18/INT0) PD2  -|4     25|-  PC2 (ADC2/PCINT10)
//  (PCINT19/OC2B/INT1) PD3  -|5     24|-  PC1 (ADC1/PCINT9)
//     (PCINT20/XCK/T0) PD4  -|6     23|-  PC0 (ADC0/PCINT8)
//                      Vcc  -|7     22|-  GND
//                      GND  -|8     21|-  AVREF
// (PCINT6/XTAL1/TOCS1) PB6  -|9     20|-  AVCC
// (PCINT7/XTAL2/TOSC2) PB7  -|10    19|-  PB5 (SCK/PCINT5)
//    (PCINT21/OC0B/T1) PD5  -|11    18|-  PB4 (MISO/PCINT4)
//  (PCINT22/OC0A/AIN0) PD6  -|12    17|-  PB3 (MOSI/OC2A/PCINT3)
//       (PCINT23/AIN1) PD7  -|13    16|-  PB2 (!SS/OC1B/PCINT2)
//   (PCINT0/CLK0/ICP1) PB0  -|14    15|-  PB1 (OC1A/PCINT1)
//                            +--------+

// ATmega328 -> Arduino pin mapping:
//
//                            +---\/---+
//                           -|1     28|-  A5
//                       D0  -|2     27|-  A4
//                       D1  -|3     26|-  A3
//                       D2  -|4     25|-  A2
//                       D3  -|5     24|-  A1
//                       D4  -|6     23|-  A0
//                      Vcc  -|7     22|-  GND
//                      GND  -|8     21|-  
//                           -|9     20|-  
//                           -|10    19|-  D13
//                       D5  -|11    18|-  D12
//                       D6  -|12    17|-  D11
//                       D7  -|13    16|-  D10
//                       D8  -|14    15|-  D9
//                            +--------+

// (for PDIP package:)

#define  ATMEGA328_PIN_2   ARDUINO_PIN_D0
#define  ATMEGA328_PIN_3   ARDUINO_PIN_D1
#define  ATMEGA328_PIN_4   ARDUINO_PIN_D2
#define  ATMEGA328_PIN_5   ARDUINO_PIN_D3
#define  ATMEGA328_PIN_6   ARDUINO_PIN_D4
#define  ATMEGA328_PIN_11  ARDUINO_PIN_D5
#define  ATMEGA328_PIN_12  ARDUINO_PIN_D6
#define  ATMEGA328_PIN_13  ARDUINO_PIN_D7
#define  ATMEGA328_PIN_14  ARDUINO_PIN_D8
#define  ATMEGA328_PIN_15  ARDUINO_PIN_D9
#define  ATMEGA328_PIN_16  ARDUINO_PIN_D10
#define  ATMEGA328_PIN_17  ARDUINO_PIN_D11
#define  ATMEGA328_PIN_18  ARDUINO_PIN_D12
#define  ATMEGA328_PIN_19  ARDUINO_PIN_D13

#define  ATMEGA328_PIN_23  ARDUINO_PIN_A0
#define  ATMEGA328_PIN_24  ARDUINO_PIN_A1
#define  ATMEGA328_PIN_25  ARDUINO_PIN_A2
#define  ATMEGA328_PIN_26  ARDUINO_PIN_A3
#define  ATMEGA328_PIN_27  ARDUINO_PIN_A4
#define  ATMEGA328_PIN_28  ARDUINO_PIN_A5


// ATtiny85 (PDIP) pinout:
//
//                                      +--\/--+
//        (PCINT5/!RESET/ADC0/dW) PB5  -|1    8|-  Vcc
// (PCINT3/XTAL1/CLKI/!OC1B/ADC3) PB3  -|2    7|-  PB2 (SCK/USCK/SCL/ADC1/T0/INT0/PCINT2)
//  (PCINT4/XTAL2/CLKO/OC1B/ADC2) PB4  -|3    6|-  PB1 (MISO/DO/AIN1/OC0B/OC1A/PCINT1)
//                                GND  -|4    5|-  PB0 (MOSI/DI/SDA/AIN0/OC0A/!OC1A/AREF/PCINT0)
//                                      +------+

// ATtiny85 -> Arduino pin mapping:
//
//                                      +--\/--+
//                                 D5  -|1    8|-  Vcc
//                                 D3  -|2    7|-  D2
//                                 D4  -|3    6|-  D1
//                                GND  -|4    5|-  D0
//                                      +------+

// (for PDIP / SOIC / TSSOP package:)

#define  ATTINY85_PIN_1  ARDUINO_PIN_D5
#define  ATTINY85_PIN_2  ARDUINO_PIN_D3
#define  ATTINY85_PIN_3  ARDUINO_PIN_D4
#define  ATTINY85_PIN_5  ARDUINO_PIN_D0
#define  ATTINY85_PIN_6  ARDUINO_PIN_D1
#define  ATTINY85_PIN_7  ARDUINO_PIN_D2


#endif


