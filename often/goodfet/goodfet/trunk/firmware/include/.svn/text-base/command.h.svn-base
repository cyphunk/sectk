// Command handling functions.

//! Global data buffer.
extern unsigned char cmddata[256];
#define cmddataword ((unsigned int*) cmddata)
#define cmddatalong ((unsigned long*) cmddata)
#define memorybyte ((unsigned char*) 0)
#define memoryword ((unsigned int*) 0)

// Global Commands
#define READ  0x00
#define WRITE 0x01
#define PEEK  0x02
#define POKE  0x03
#define SETUP 0x10
#define START 0x20
#define STOP  0x21
#define NOK   0x7E
#define OK    0x7F

// Monitor Commands
#define MONITOR_CHANGE_BAUD 0x80
#define MONITOR_RAM_PATTERN 0x90
#define MONITOR_RAM_DEPTH 0x91

#define MONITOR_DIR 0xA0
#define MONITOR_OUT 0xA1
#define MONITOR_IN  0xA2

//CHIPCON commands
#define CC_CHIP_ERASE 0x80
#define CC_WR_CONFIG 0x81
#define CC_RD_CONFIG 0x82
#define CC_GET_PC 0x83
#define CC_READ_STATUS 0x84
#define CC_SET_HW_BRKPNT 0x85
#define CC_HALT 0x86
#define CC_RESUME 0x87
#define CC_DEBUG_INSTR 0x88
#define CC_STEP_INSTR 0x89
#define CC_STEP_REPLACE 0x8a
#define CC_GET_CHIP_ID 0x8b
//CHIPCON macros
#define CC_READ_CODE_MEMORY 0x90
#define CC_READ_XDATA_MEMORY 0x91
#define CC_WRITE_XDATA_MEMORY 0x92
#define CC_SET_PC 0x93
#define CC_CLOCK_INIT 0x94
#define CC_WRITE_FLASH_PAGE 0x95
#define CC_READ_FLASH_PAGE 0x96
#define CC_MASS_ERASE_FLASH 0x97
#define CC_PROGRAM_FLASH 0x98

//JTAG commands
#define JTAG_IR_SHIFT 0x80
#define JTAG_DR_SHIFT 0x81
#define JTAG_DR_SHIFT20 0x91

//SPI commands
#define SPI_JEDEC 0x80
#define SPI_ERASE 0x81

//OCT commands
#define OCT_CMP 0x90
#define OCT_RES 0x91

//JTAG430 commands
#define JTAG430_HALTCPU 0xA0
#define JTAG430_RELEASECPU 0xA1
#define JTAG430_SETINSTRFETCH 0xC1
#define JTAG430_SETPC 0xC2
#define JTAG430_WRITEMEM 0xE0
#define JTAG430_WRITEFLASH 0xE1
#define JTAG430_READMEM 0xE2
#define JTAG430_ERASEFLASH 0xE3
#define JTAG430_ERASECHECK 0xE4
#define JTAG430_VERIFYMEM 0xE5
#define JTAG430_BLOWFUSE 0xE6
#define JTAG430_ISFUSEBLOWN 0xE7
#define JTAG430_COREIP_ID 0xF0
#define JTAG430_DEVICE_ID 0xF1

//! Handle a command.  Defined in goodfet.c
void handle(unsigned char app,
	    unsigned char verb,
	    unsigned  char len);

//! Transmit data.
void txdata(unsigned char app,
	    unsigned char verb,
	    unsigned char len);
//! Transmit a string.
void txstring(unsigned char app,
	      unsigned char verb,
	      const char *str);

//! Delay
void delay(unsigned int count);
//! MSDelay
void msdelay(unsigned int ms);


void monitorhandle(unsigned char, unsigned char, unsigned char);
void spihandle(unsigned char, unsigned char, unsigned char);
void i2chandle(unsigned char, unsigned char, unsigned char);
void cchandle(unsigned char, unsigned char, unsigned char);
void jtaghandle(unsigned char, unsigned char, unsigned char);
void jtag430handle(unsigned char, unsigned char, unsigned char);
void jtag430x2handle(unsigned char app, unsigned char verb,
		     unsigned char len);
  
