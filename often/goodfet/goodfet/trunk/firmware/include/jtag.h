
#include <signal.h>
#include <io.h>
#include <iomacros.h>


extern unsigned int drwidth;

#define MSP430MODE 0
#define MSP430XMODE 1
#define MSP430X2MODE 2
extern unsigned int jtag430mode;

// Generic Commands

//! Shift n bytes.
unsigned long jtagtransn(unsigned long word,
			 unsigned int bitcount);
//! Shift 8 bits of the IR.
unsigned char jtag_ir_shift8(unsigned char);
//! Shift 16 bits of the DR.
unsigned int jtag_dr_shift16(unsigned int);
//! Stop JTAG, release pins
void jtag_stop();

void jtagsetup();

// JTAG430 Commands

//! Start JTAG, unique to the '430.
void jtag430_start();
//! Reset the TAP state machine, check the fuse.
void jtag430_resettap();

//! Defined in jtag430asm.S
void jtag430_tclk_flashpulses(int);

//High-level Macros follow
//! Write data to address.
void jtag430_writemem(unsigned int adr, unsigned int data);
//! Read data from address
unsigned int jtag430_readmem(unsigned int adr);
//! Halt the CPU
void jtag430_haltcpu();
//! Release the CPU
void jtag430_releasecpu();
//! Set CPU to Instruction Fetch
void jtag430_setinstrfetch();
//! Set the program counter.
void jtag430_setpc(unsigned int adr);
//! Write data to address.
void jtag430_writeflash(unsigned int adr, unsigned int data);

//Pins.  Both SPI and JTAG names are acceptable.
//#define SS   BIT0
#define MOSI BIT1
#define MISO BIT2
#define SCK  BIT3

#define TMS BIT0
#define TDI BIT1
#define TDO BIT2
#define TCK BIT3

#define TCLK TDI

//These are not on P5
#define RST BIT6
#define TST BIT0

//This could be more accurate.
//Does it ever need to be?
#define JTAGSPEED 20
#define JTAGDELAY(x) delay(x)

#define SETMOSI P5OUT|=MOSI
#define CLRMOSI P5OUT&=~MOSI
#define SETCLK P5OUT|=SCK
#define CLRCLK P5OUT&=~SCK
#define READMISO (P5IN&MISO?1:0)
#define SETTMS P5OUT|=TMS
#define CLRTMS P5OUT&=~TMS
#define SETTCK P5OUT|=TCK
#define CLRTCK P5OUT&=~TCK
#define SETTDI P5OUT|=TDI
#define CLRTDI P5OUT&=~TDI

#define SETTST P4OUT|=TST
#define CLRTST P4OUT&=~TST
#define SETRST P2OUT|=RST
#define CLRRST P2OUT&=~RST

#define SETTCLK SETTDI
#define CLRTCLK CLRTDI

extern int savedtclk;
#define SAVETCLK savedtclk=P5OUT&TCLK;
#define RESTORETCLK if(savedtclk) P5OUT|=TCLK; else P5OUT&=~TCLK


//16-bit MSP430 JTAG commands, bit-swapped
#define IR_CNTRL_SIG_16BIT         0xC8   // 0x13
#define IR_CNTRL_SIG_CAPTURE       0x28   // 0x14
#define IR_CNTRL_SIG_RELEASE       0xA8   // 0x15
// Instructions for the JTAG Fuse
#define IR_PREPARE_BLOW            0x44   // 0x22
#define IR_EX_BLOW                 0x24   // 0x24
// Instructions for the JTAG data register
#define IR_DATA_16BIT              0x82   // 0x41
#define IR_DATA_QUICK              0xC2   // 0x43
// Instructions for the JTAG PSA mode
#define IR_DATA_PSA                0x22   // 0x44
#define IR_SHIFT_OUT_PSA           0x62   // 0x46
// Instructions for the JTAG address register
#define IR_ADDR_16BIT              0xC1   // 0x83
#define IR_ADDR_CAPTURE            0x21   // 0x84
#define IR_DATA_TO_ADDR            0xA1   // 0x85
// Bypass instruction
#define IR_BYPASS                  0xFF   // 0xFF

//MSP430X2 unique
#define IR_COREIP_ID               0xE8   // 0x17 
#define IR_DEVICE_ID               0xE1   // 0x87

//MSP430 or MSP430X
#define MSP430JTAGID 0x89
//MSP430X2 only
#define MSP430X2JTAGID 0x91
