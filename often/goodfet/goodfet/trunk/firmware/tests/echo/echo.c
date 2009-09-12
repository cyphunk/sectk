//GOODFET Echo test.


#include "platform.h"

#include <signal.h>
#include <io.h>
#include <iomacros.h>


//LED on P1.0
//IO on P5

//! Initialize registers and all that jazz.
void init(){
  volatile unsigned int i;
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  
  //LED and TX OUT
  PLEDDIR |= PLEDPIN;
  
  msp430_init_dco();
  msp430_init_uart();
  
  //Enable Interrupts.
  //eint();
}

//! Main loop.
int main(void)
{
  volatile unsigned int i;
  init();
  
  
  PLEDOUT^=PLEDPIN;  // Blink
  
  //while(1) serial_tx(serial_rx());
  while(1) serial_tx('G');
  
  while(1){
    i = 10000;
    while(i--);
    
    PLEDOUT^=PLEDPIN;  // Blink
  }
}

