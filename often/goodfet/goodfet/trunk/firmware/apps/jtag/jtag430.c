
#include "platform.h"
#include "command.h"
#include "jtag.h"


unsigned int jtag430mode=MSP430X2MODE;

//! Set the program counter.
void jtag430_setpc(unsigned int adr){
  jtag_ir_shift8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift16(0x3401);// release low byte
  jtag_ir_shift8(IR_DATA_16BIT);
  jtag_dr_shift16(0x4030);//Instruction to load PC
  CLRTCLK;
  SETTCLK;
  jtag_dr_shift16(adr);// Value for PC
  CLRTCLK;
  jtag_ir_shift8(IR_ADDR_CAPTURE);
  SETTCLK;
  CLRTCLK ;// Now PC is set to "PC_Value"
  jtag_ir_shift8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift16(0x2401);// low byte controlled by JTAG
}

//! Halt the CPU
void jtag430_haltcpu(){
  //jtag430_setinstrfetch();
  
  jtag_ir_shift8(IR_DATA_16BIT);
  jtag_dr_shift16(0x3FFF);//JMP $+0
  
  CLRTCLK;
  jtag_ir_shift8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift16(0x2409);//set JTAG_HALT bit
  SETTCLK;
}

//! Release the CPU
void jtag430_releasecpu(){
  CLRTCLK;
  jtag_ir_shift8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift16(0x2401);
  jtag_ir_shift8(IR_ADDR_CAPTURE);
  SETTCLK;
}

//! Read data from address
unsigned int jtag430_readmem(unsigned int adr){
  unsigned int toret;
  
  CLRTCLK;
  jtag_ir_shift8(IR_CNTRL_SIG_16BIT);
  
  if(adr>0xFF)
    jtag_dr_shift16(0x2409);//word read
  else
    jtag_dr_shift16(0x2419);//byte read
  jtag_ir_shift8(IR_ADDR_16BIT);
  jtag_dr_shift16(adr);//address
  jtag_ir_shift8(IR_DATA_TO_ADDR);
  SETTCLK;

  CLRTCLK;
  toret=jtag_dr_shift16(0x0000);//16 bit return
  
  return toret;
}

//! Write data to address.
void jtag430_writemem(unsigned int adr, unsigned int data){
  CLRTCLK;
  jtag_ir_shift8(IR_CNTRL_SIG_16BIT);
  if(adr>0xFF)
    jtag_dr_shift16(0x2408);//word write
  else
    jtag_dr_shift16(0x2418);//byte write
  jtag_ir_shift8(IR_ADDR_16BIT);
  jtag_dr_shift16(adr);
  jtag_ir_shift8(IR_DATA_TO_ADDR);
  jtag_dr_shift16(data);
  SETTCLK;
}

//! Write data to flash memory.  Must be preconfigured.
void jtag430_writeflashword(unsigned int adr, unsigned int data){
  /*
  CLRTCLK;
  jtag_ir_shift8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift16(0x2408);//word write
  jtag_ir_shift8(IR_ADDR_16BIT);
  jtag_dr_shift16(adr);
  jtag_ir_shift8(IR_DATA_TO_ADDR);
  jtag_dr_shift16(data);
  SETTCLK;
  
  //Return to read mode.
  CLRTCLK;
  jtag_ir_shift8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift16(0x2409);
  */
  
  jtag430_writemem(adr,data);
  CLRTCLK;
  jtag_ir_shift8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift16(0x2409);
  
  //Pulse TCLK
  jtag430_tclk_flashpulses(35); //35 standard
  
}

//! Configure flash, then write a word.
void jtag430_writeflash(unsigned int adr, unsigned int data){
  jtag430_haltcpu();
  
  //FCTL1=0xA540, enabling flash write
  jtag430_writemem(0x0128, 0xA540);
  //FCTL2=0xA540, selecting MCLK as source, DIV=1
  jtag430_writemem(0x012A, 0xA540);
  //FCTL3=0xA500, should be 0xA540 for Info Seg A on 2xx chips.
  jtag430_writemem(0x012C, 0xA500);
  
  //Write the word itself.
  jtag430_writeflashword(adr,data);
  
  //FCTL1=0xA500, disabling flash write
  jtag430_writemem(0x0128, 0xA500);
  
  jtag430_releasecpu();
}



//! Power-On Reset
void jtag430_por(){
  unsigned int jtagid;

  // Perform Reset
  jtag_ir_shift8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift16(0x2C01); // apply
  jtag_dr_shift16(0x2401); // remove
  CLRTCLK;
  SETTCLK;
  CLRTCLK;
  SETTCLK;
  CLRTCLK;
  jtagid = jtag_ir_shift8(IR_ADDR_CAPTURE); // get JTAG identifier
  SETTCLK;
  
  jtag430_writemem(0x0120, 0x5A80);   // Diabled Watchdog
}



#define ERASE_GLOB 0xA50E
#define ERASE_ALLMAIN 0xA50C
#define ERASE_MASS 0xA506
#define ERASE_MAIN 0xA504
#define ERASE_SGMT 0xA502

//! Configure flash, then write a word.
void jtag430_eraseflash(unsigned int mode, unsigned int adr, unsigned int count){
  jtag430_haltcpu();
  
  //FCTL1= erase mode
  jtag430_writemem(0x0128, mode);
  //FCTL2=0xA540, selecting MCLK as source, DIV=1
  jtag430_writemem(0x012A, 0xA540);
  //FCTL3=0xA500, should be 0xA540 for Info Seg A on 2xx chips.
  jtag430_writemem(0x012C, 0xA500);
  
  //Write the erase word.
  jtag430_writemem(adr, 0x55AA);
  //Return to read mode.
  CLRTCLK;
  jtag_ir_shift8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift16(0x2409);
  
  //Send the pulses.
  jtag430_tclk_flashpulses(count);
  
  //FCTL1=0xA500, disabling flash write
  jtag430_writemem(0x0128, 0xA500);
  
  jtag430_releasecpu();
}


//! Reset the TAP state machine.
void jtag430_resettap(){
  int i;
  // Settle output
  SETTMS;
  SETTDI;
  SETTCK;

  // Navigate to reset state.
  // Should be at least six.
  for(i=0;i<4;i++){
    CLRTCK;
    SETTCK;
  }

  // test-logic-reset
  CLRTCK;
  CLRTMS;
  SETTCK;
  SETTMS;
  // idle

    
  /* sacred, by spec.
     Sometimes this isn't necessary. */
  // fuse check
  CLRTMS;
  delay(50);
  SETTMS;
  CLRTMS;
  delay(50);
  SETTMS;
  /**/
  
}

//! Start JTAG, take pins
void jtag430_start(){
  jtagsetup();
  
  //Known-good starting position.
  //Might be unnecessary.
  SETTST;
  SETRST;
  delay(0xFFFF);
  
  //Entry sequence from Page 67 of SLAU265A for 4-wire MSP430 JTAG
  CLRRST;
  delay(100);
  CLRTST;
  delay(50);
  SETTST;
  delay(50);
  SETRST;
  P5DIR&=~RST;
  delay(0xFFFF);
  
  //Perform a reset and disable watchdog.
  jtag430_por();
}

//! Set CPU to Instruction Fetch
void jtag430_setinstrfetch(){
  jtag_ir_shift8(IR_CNTRL_SIG_CAPTURE);

  // Wait until instruction fetch state.
  while(1){
    if (jtag_dr_shift16(0x0000) & 0x0080)
      return;
    CLRTCLK;
    SETTCLK;
  }
}


//! Handles classic MSP430 JTAG commands.  Forwards others to JTAG.
void oldjtag430handle(unsigned char app,
		   unsigned char verb,
		   unsigned char len){
  
  switch(verb){
  case START:
    //Enter JTAG mode.
    jtag430_start();
    //TAP setup, fuse check
    jtag430_resettap();
    txdata(app,verb,0);
    break;
  case JTAG430_HALTCPU:
    jtag430_haltcpu();
    txdata(app,verb,0);
    break;
  case JTAG430_RELEASECPU:
    jtag430_releasecpu();
    txdata(app,verb,0);
    break;
  case JTAG430_SETINSTRFETCH:
    jtag430_setinstrfetch();
    txdata(app,verb,0);
    break;

    
  case JTAG430_READMEM:
  case PEEK:
    cmddataword[0]=jtag430_readmem(cmddataword[0]);
    txdata(app,verb,2);
    break;
  case JTAG430_WRITEMEM:
  case POKE:
    jtag430_writemem(cmddataword[0],cmddataword[1]);
    cmddataword[0]=jtag430_readmem(cmddataword[0]);
    txdata(app,verb,2);
    break;
  case JTAG430_WRITEFLASH:
    jtag430_writeflash(cmddataword[0],cmddataword[1]);
    cmddataword[0]=jtag430_readmem(cmddataword[0]);
    txdata(app,verb,2);
    break;
  case JTAG430_ERASEFLASH:
    jtag430_eraseflash(ERASE_MASS,0xFFFE,0xFFFF);
    jtag430_eraseflash(ERASE_MASS,0xFFFE,0xFFFF);
    jtag430_eraseflash(ERASE_MASS,0xFFFE,0xFFFF);
    txdata(app,verb,0);
    break;
  case JTAG430_SETPC:
    jtag430_setpc(cmddataword[0]);
    txdata(app,verb,0);
    break;
  default:
    jtaghandle(app,verb,len);
  }
  jtag430_resettap();
}

//! Handles unique MSP430 JTAG commands.  Forwards others to JTAG.
void jtag430handle(unsigned char app,
		   unsigned char verb,
		   unsigned char len){
  switch(jtag430mode){
  case MSP430MODE:
    return oldjtag430handle(app,verb,len);
  case MSP430X2MODE:
    return jtag430x2handle(app,verb,len);
  default:
    txdata(app,NOK,0);
  }
}
