//! \file platform.h

#include <signal.h>
#include <io.h>
#include <iomacros.h>


//Use P3 instead of P5 for target I/O on chips without P5.
#ifndef __MSP430_HAS_PORT5__
#ifndef __MSP430_HAS_PORT5_R__
#warning "No P5, using P3 instead.  Will break 2618 and 1612 support."
#define P5OUT P3OUT
#define P5DIR P3DIR
#define P5REN P3REN
#define P5IN P3IN

#endif
#endif

//Use false P5REN for 1612.
#ifdef __MSP430_HAS_PORT5__
#ifndef __MSP430_HAS_PORT5_R__
#warning "1xx, using fake P5REN for external pulling resistors."
#define P5REN P5OUT
#endif
#endif

unsigned char serial_rx();
void serial_tx(unsigned char);

unsigned char serial1_rx();
void serial1_tx(unsigned char);

void setbaud(unsigned char);
void setbaud1(unsigned char);

//! Initialize the UART
void msp430_init_uart();
//! Initialize the DCO Clock
void msp430_init_dco();

//LED on P1.0
#define PLEDOUT P1OUT
#define PLEDDIR P1DIR
#define PLEDPIN 0x1

