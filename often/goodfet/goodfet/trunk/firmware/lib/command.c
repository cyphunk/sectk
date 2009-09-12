#include "command.h"
#include "platform.h"
#include <string.h>

//! Different command handling functions.

unsigned char cmddata[256];

//! Transmit a string.
void txstring(unsigned char app,
	      unsigned char verb,
	      const char *str){
  unsigned char len=strlen(str);
  serial_tx(app);
  serial_tx(verb);
  serial_tx(len);
  while(len--)
    serial_tx(*(str++));
}

//! Transmit data.
void txdata(unsigned char app,
	    unsigned char verb,
	    unsigned char len){
  unsigned int i=0;
  serial_tx(app);
  serial_tx(verb);
  serial_tx(len);
  for(i=0;i<len;i++){
    serial_tx(cmddata[i]);
  }
}

//Be very careful changing delay().
//It was chosen poorly by trial and error.

//! Delay for a count.
void delay(unsigned int count){
  volatile unsigned int i=count;
  while(i--) asm("nop");
}
//! MSDelay
void msdelay(unsigned int ms){
  volatile unsigned int i,j;
  i=100;
  while(i--){
    j=ms;
    while(j--) asm("nop");
  }
  //Using TimerA might be cleaner.
}
