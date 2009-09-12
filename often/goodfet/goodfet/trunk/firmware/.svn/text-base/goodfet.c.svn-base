//GOODFET Main File
//Includes several applications.

#include "platform.h"
#include "command.h"
#include "apps.h"



//LED on P1.0
//IO on P5

//! Initialize registers and all that jazz.
void init(){
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  
  //LED out and on.
  PLEDDIR |= PLEDPIN;
  PLEDOUT |= PLEDPIN;
  
  //Setup clocks, unique to each '430.
  msp430_init_dco();
  msp430_init_uart();
  
  //Enable Interrupts.
  //eint();
}

//! Handle a command.
void handle(unsigned char app,
	    unsigned char verb,
	    unsigned char len){
  switch(app){
  case MONITOR:
    monitorhandle(app,verb,len);
    break;
  case SPI:
    spihandle(app,verb,len);
    break;
  case I2CAPP:
    i2chandle(app,verb,len);
    break;
  case CHIPCON:
    cchandle(app,verb,len);
    break;
  case JTAG:
    jtaghandle(app,verb,len);
    break;
  case JTAG430: //Also JTAG430X, JTAG430X2
    jtag430x2handle(app,verb,len);
    break;
  default:
    #ifdef HANDLEOTHER
    HANDLEOTHER(app,verb,len);
    #else
    txdata(app,NOK,0);
    #endif
    break;
  }
}

//! Main loop.
int main(void)
{
  volatile unsigned int i;
  unsigned char app, verb, len;
  
  init();
  
  txstring(MONITOR,OK,"http://goodfet.sf.net/");
  
  //Command loop.  There's no end!
  while(1){
    //Magic 3
    app=serial_rx();
    verb=serial_rx();
    len=serial_rx();
    
    //Read data, if any
    for(i=0;i<len;i++){
      cmddata[i]=serial_rx();
    }
    handle(app,verb,len);
  }
}

