/** SPI **/


//Pins and I/O
#define SS   BIT0
#define MOSI BIT1
#define MISO BIT2
#define SCK  BIT3

#define SETSS P5OUT|=SS
#define CLRSS P5OUT&=~SS

#define SETMOSI P5OUT|=MOSI
#define CLRMOSI P5OUT&=~MOSI
#define SETCLK P5OUT|=SCK
#define CLRCLK P5OUT&=~SCK
#define READMISO (P5IN&MISO?1:0)
