//GoodFET JTAG Application
//Handles basic I/O

//Higher level left to client application.

#include "platform.h"
#include "command.h"
#include "jtag.h"


//! Set up the pins for JTAG mode.
void jtagsetup(){
  P5DIR|=MOSI+SCK+TMS;
  P5DIR&=~MISO;
  P5OUT|=0xFFFF;
  P4DIR|=TST;
  P2DIR|=RST;
}

int savedtclk=0;
//! Shift 8 bits in and out.
unsigned char jtagtrans8(unsigned char byte){
  unsigned int bit;
  SAVETCLK;
  for (bit = 0; bit < 8; bit++) {
    /* write MOSI on trailing edge of previous clock */
    if (byte & 0x80)
      {SETMOSI;}
    else
      {CLRMOSI;}
    byte <<= 1;
    
    if(bit==7)
      SETTMS;//TMS high on last bit to exit.
    
    CLRTCK;
    SETTCK;
     /* read MISO on trailing edge */
    byte |= READMISO;
  }
  RESTORETCLK;
  
  // exit state
  CLRTCK;
  SETTCK;
  // update state
  CLRTMS;
  CLRTCK;
  SETTCK;
  
  return byte;
}

//! Shift n bits in and out.
unsigned long jtagtransn(unsigned long word,
			 unsigned int bitcount){
  unsigned int bit;
  //0x8000
  unsigned long high;
  
  if(bitcount==20)
    high=0x80000;
  if(bitcount==16)
    high= 0x8000;
  
  SAVETCLK;
  
  for (bit = 0; bit < bitcount; bit++) {
    /* write MOSI on trailing edge of previous clock */
    if (word & high)
      {SETMOSI;}
    else
      {CLRMOSI;}
    word <<= 1;
    
    if(bit==bitcount-1)
      SETTMS;//TMS high on last bit to exit.
    
    CLRTCK;
    SETTCK;
    /* read MISO on trailing edge */
    word |= READMISO;
  }
  
  if(bitcount==20){
    word = ((word << 16) | (word >> 4)) & 0x000FFFFF;
  }
  
  RESTORETCLK;
  
  // exit state
  CLRTCK;
  SETTCK;
  // update state
  CLRTMS;
  CLRTCK;
  SETTCK;
  
  return word;
}

/*
//! Shift 16 bits in and out.
unsigned int jtagtrans16(unsigned int word){ //REMOVEME
  unsigned int bit;
  SAVETCLK;
  
  for (bit = 0; bit < 16; bit++) {
    // write MOSI on trailing edge of previous clock 
    if (word & 0x8000)
      {SETMOSI;}
    else
      {CLRMOSI;}
    word <<= 1;
    
    if(bit==15)
      SETTMS;//TMS high on last bit to exit.
    
    CLRTCK;
    SETTCK;
    // read MISO on trailing edge 
    word |= READMISO;
  }
  RESTORETCLK;
  
  // exit state
  CLRTCK;
  SETTCK;
  // update state
  CLRTMS;
  CLRTCK;
  SETTCK;
  
  return word;
}*/

//! Stop JTAG, release pins
void jtag_stop(){
  P5OUT=0;
  P4OUT=0;
}

unsigned int drwidth=20;
//! Shift all bits of the DR.
unsigned long jtag_dr_shift20(unsigned long in){
  // idle
  SETTMS;
  CLRTCK;
  SETTCK;
  // select DR
  CLRTMS;
  CLRTCK;
  SETTCK;
  // capture IR
  CLRTCK;
  SETTCK;
  
  // shift DR, then idle
  return(jtagtransn(in,20));
}


//! Shift 16 bits of the DR.
unsigned int jtag_dr_shift16(unsigned int in){
  // idle
  SETTMS;
  CLRTCK;
  SETTCK;
  // select DR
  CLRTMS;
  CLRTCK;
  SETTCK;
  // capture IR
  CLRTCK;
  SETTCK;
  
  // shift DR, then idle
  return(jtagtransn(in,16));
}


//! Shift 8 bits of the IR.
unsigned char jtag_ir_shift8(unsigned char in){
  // idle
  SETTMS;
  CLRTCK;
  SETTCK;
  // select DR
  CLRTCK;
  SETTCK;
  // select IR
  CLRTMS;
  CLRTCK;
  SETTCK;
  // capture IR
  CLRTCK;
  SETTCK;
  
  // shift IR, then idle.
  return(jtagtrans8(in));
}

//! Handles a monitor command.
void jtaghandle(unsigned char app,
	       unsigned char verb,
	       unsigned char len){
  switch(verb){
    //START handled by specific JTAG
  case STOP:
    jtag_stop();
    txdata(app,verb,0);
    break;
  case SETUP:
    jtagsetup();
    txdata(app,verb,0);
    break;
  case JTAG_IR_SHIFT:
    cmddata[0]=jtag_ir_shift8(cmddata[0]);
    txdata(app,verb,1);
    break;
  case JTAG_DR_SHIFT:
    cmddataword[0]=jtag_dr_shift16(cmddataword[0]);
    txdata(app,verb,2);
    break;
  default:
    txdata(app,NOK,0);
  }
}


