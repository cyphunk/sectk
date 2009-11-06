//Configuration File
#include "config.h"

//Pin Definition File
#include "pins.h"

// Standard AVR Headers
#include <avr/io.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Arduino Variable Types
typedef uint8_t boolean;
typedef uint8_t byte;

// Macros
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit)) // clear bit
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit)) // set bit
#define tog(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit)) // toggle bit

// Project Functions (in order from independent to dependent)

#include "usart.h"
