//! MSP430F1612/1611 clock and I/O definitions

#include "platform.h"

#include <signal.h>
#include <io.h>
#include <iomacros.h>


//! Receive a byte.
unsigned char serial_rx(){
  char c;
  
  while(!(IFG1&URXIFG0));//wait for a byte
  c = RXBUF0;
  IFG1&=~URXIFG0;
  U0TCTL &= ~URXSE;
  
  return c;
}

//! Receive a byte.
unsigned char serial1_rx(){
  char c;
  
  while(!(IFG2&URXIFG1));//wait for a byte
  c = RXBUF1;
  IFG2&=~URXIFG1;
  U1TCTL &= ~URXSE;
  
  return c;
}


//! Transmit a byte.
void serial_tx(unsigned char x){
  while ((IFG1 & UTXIFG0) == 0); //loop until buffer is free
  TXBUF0 = x;
}

//! Transmit a byte on the second UART.
void serial1_tx(unsigned char x){
  while ((IFG2 & UTXIFG1) == 0); //loop until buffer is free
  TXBUF1 = x;
}

//! Set the baud rate.
void setbaud(unsigned char rate){
  
  //http://mspgcc.sourceforge.net/baudrate.html
  switch(rate){
  case 1://9600 baud
    UBR00=0x7F; UBR10=0x01; UMCTL0=0x5B; /* uart0 3683400Hz 9599bps */
    break;
  case 2://19200 baud
    UBR00=0xBF; UBR10=0x00; UMCTL0=0xF7; /* uart0 3683400Hz 19194bps */
    break;
  case 3://38400 baud
    UBR00=0x5F; UBR10=0x00; UMCTL0=0xBF; /* uart0 3683400Hz 38408bps */
    break;
  case 4://57600 baud
    UBR00=0x40; UBR10=0x00; UMCTL0=0x00; /* uart0 3683400Hz 57553bps */
    break;
  default:
  case 5://115200 baud
    UBR00=0x20; UBR10=0x00; UMCTL0=0x00; /* uart0 3683400Hz 115106bps */
    break;
  }
}

//! Set the baud rate of the second uart.
void setbaud1(unsigned char rate){
  
  //http://mspgcc.sourceforge.net/baudrate.html
  switch(rate){
  case 1://9600 baud
    //    UBR01=0x7F; UBR11=0x01; UMCTL1=0x5B; /* uart0 3683400Hz 9599bps */
    break;
  case 2://19200 baud
    //UBR01=0xBF; UBR11=0x00; UMCTL1=0xF7; /* uart0 3683400Hz 19194bps */
    break;
  case 3://38400 baud
    //UBR01=0x5F; UBR11=0x00; UMCTL1=0xBF; /* uart0 3683400Hz 38408bps */
    break;
  case 4://57600 baud
    //UBR01=0x40; UBR11=0x00; UMCTL1=0x00; /* uart0 3683400Hz 57553bps */
    break;
  default:
  case 5://115200 baud
    //UBR01=0x20; UBR11=0x00; UMCTL1=0x00; /* uart0 3683400Hz 115106bps */
    break;
  }
}


void msp430_init_uart(){
  
  /* RS232 */
  
  P3SEL |= BIT4|BIT5;                        // P3.4,5 = USART0 TXD/RXD
  P3DIR |= BIT4;
  
  UCTL0 = SWRST | CHAR;                 /* 8-bit character, UART mode */
  UTCTL0 = SSEL1;                       /* UCLK = MCLK */
  
  setbaud(0);
  
  ME1 &= ~USPIE0;			/* USART1 SPI module disable */
  ME1 |= (UTXE0 | URXE0);               /* Enable USART1 TXD/RXD */

  UCTL0 &= ~SWRST;

  /* XXX Clear pending interrupts before enable!!! */
  U0TCTL |= URXSE;
  
  
  //IE1 |= URXIE1;                        /* Enable USART1 RX interrupt  */
}


void msp430_init_dco() {
/* This code taken from the FU Berlin sources and reformatted. */
  //

//Works well.
//#define MSP430_CPU_SPEED 2457600UL

//Too fast for internal resistor.
//#define MSP430_CPU_SPEED 4915200UL

//Max speed.
//#deefine MSP430_CPU_SPEED 4500000UL

//baud rate speed
#define MSP430_CPU_SPEED 3683400UL
#define DELTA    ((MSP430_CPU_SPEED) / (32768 / 8))
  unsigned int compare, oldcapture = 0;
  unsigned int i;
  
  WDTCTL = WDTPW + WDTHOLD; //stop WDT
  
  
  DCOCTL=0xF0;
  //a4
  //1100

  /* ACLK is devided by 4. RSEL=6 no division for MCLK
     and SSMCLK. XT2 is off. */
  //BCSCTL1 = 0xa8;
  
  BCSCTL2 = 0x00; /* Init FLL to desired frequency using the 32762Hz
		     crystal DCO frquenzy = 2,4576 MHz  */
  
  P1OUT|=1;
  
  BCSCTL1 |= DIVA1 + DIVA0;             /* ACLK = LFXT1CLK/8 */
  for(i = 0xffff; i > 0; i--) {         /* Delay for XTAL to settle */
    asm("nop");
  }

  CCTL2 = CCIS0 + CM0 + CAP;            // Define CCR2, CAP, ACLK
  TACTL = TASSEL1 + TACLR + MC1;        // SMCLK, continous mode


  while(1) {

    while((CCTL2 & CCIFG) != CCIFG);    /* Wait until capture occured! */
    CCTL2 &= ~CCIFG;                    /* Capture occured, clear flag */
    compare = CCR2;                     /* Get current captured SMCLK */
    compare = compare - oldcapture;     /* SMCLK difference */
    oldcapture = CCR2;                  /* Save current captured SMCLK */

    if(DELTA == compare) {
      break;                            /* if equal, leave "while(1)" */
    } else if(DELTA < compare) {        /* DCO is too fast, slow it down */
      DCOCTL--;
      if(DCOCTL == 0xFF) {              /* Did DCO role under? */
	BCSCTL1--;
      }
    } else {                            /* -> Select next lower RSEL */
      DCOCTL++;
      if(DCOCTL == 0x00) {              /* Did DCO role over? */
	BCSCTL1++;
      }
                                        /* -> Select next higher RSEL  */
    }
  }
  
  CCTL2 = 0;                            /* Stop CCR2 function */
  TACTL = 0;                            /* Stop Timer_A */

  BCSCTL1 &= ~(DIVA1 + DIVA0);          /* remove /8 divisor from ACLK again */
  
  P1OUT=0;

}

