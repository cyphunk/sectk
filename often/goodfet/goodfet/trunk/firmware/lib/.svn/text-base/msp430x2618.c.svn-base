//! MSP430F2618 clock and I/O definitions

// Ought to be portable to other 2xx chips.
// 2274 looks particularly appealing.

#include "platform.h"

#include <signal.h>
#include <io.h>
#include <iomacros.h>


//! Receive a byte.
unsigned char serial_rx(){
  char c;
  
  while(!(IFG2&UCA0RXIFG));//wait for a byte
  c = UCA0RXBUF;
  IFG2&=~UCA0RXIFG;
  
  //UCA0CTL1 &= ~UCA0RXSE;
  return c;
}

//! Receive a byte.
unsigned char serial1_rx(){
  //TODO
  return 00;
}

//! Transmit a byte.
void serial_tx(unsigned char x){
  while ((IFG2 & UCA0TXIFG) == 0); //loop until buffer is free
  UCA0TXBUF = x;	/* send the character */
  while(!(IFG2 & UCA0TXIFG));
}
//! Transmit a byte.
void serial_tx_old(unsigned char x){
  while ((IFG2 & UCA0TXIFG) == 0); //loop until buffer is free
  UCA0TXBUF = x;	/* send the character */
  while(!(IFG2 & UCA0TXIFG));
}

//! Transmit a byte on the second UART.
void serial1_tx(unsigned char x){

}

//! Set the baud rate.
void setbaud(unsigned char rate){
  
  //Table 15-4, page 481 of 2xx Family Guide
  switch(rate){
  case 1://9600 baud
    UCA0BR1 = 0x06;
    UCA0BR0 = 0x82;
    break;
  case 2://19200 baud
    UCA0BR1 = 0x03;
    UCA0BR0 = 0x41;
    break;
  case 3://38400 baud
    UCA0BR1 = 0xa0;
    UCA0BR0 = 0x01;
    break;
  case 4://57600 baud
    UCA0BR1 = 0x1d;
    UCA0BR0 = 0x01;
    break;
  default:
  case 5://115200 baud
    UCA0BR0 = 0x8a;
    UCA0BR1 = 0x00;
    break;
  }
}

//! Set the baud rate of the second uart.
void setbaud1(unsigned char rate){
  
  //http://mspgcc.sourceforge.net/baudrate.html
  switch(rate){
  case 1://9600 baud
    
    break;
  case 2://19200 baud
    break;
  case 3://38400 baud
    
    break;
  case 4://57600 baud
    
    break;
  default:
  case 5://115200 baud
    
    break;
  }
}

#define BAUD0EN 0x41
#define BAUD1EN 0x03

void msp430_init_uart(){

  // Serial on P3.4, P3.5                                                                                                                                                 
  P3SEL |= BIT4 + BIT5;
  P3DIR |= BIT4;

  //UCA0CTL1 |= UCSWRST;                    /* disable UART */                                                                                                            

  UCA0CTL0 = 0x00;
  //UCA0CTL0 |= UCMSB ;                                                                                                                                                   
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK                                                                                                                      

  //UCA0BR0 = BAUD0EN;                        // 115200                                                                                                                     
  //UCA0BR1 = BAUD1EN;
  setbaud(5);//default baud, 115200

  UCA0MCTL = 0;                             // Modulation UCBRSx = 5                                                                                                      
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**                                                                                          


  //Leave this commented!                                                                                                                                                 
  //Interrupt is handled by target code, not by bootloader.                                                                                                               
  //IE2 |= UCA0RXIE;         

}

//! Initialize the MSP430 clock.
void msp430_init_dco() {
  //LED lights when init_dco begins,
  //then dims when successful.
  PLEDOUT|=PLEDPIN;
  
  if(CALBC1_16MHZ!=0xFF && CALDCO_16MHZ!=0xFF){
    //Info is intact, use it.
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;
  }else{
    //Info is missing, guess at a good value.
    switch(*((int*)0xff0)){
    default:
    case 0x6ff2:        //f26f, the MSP430F2618
      BCSCTL1 = 0x8f;   //CALBC1_16MHZ at 0x10f9
      DCOCTL = 0x7f;    //CALDCO_16MHZ at 0x10f8
      break;
    }
  }
  
  //Hopefully by here the clock is meaningful.
  PLEDOUT&=~PLEDPIN;
  return;
}

