#include "all_header.h"

ISR(SIG_PIN_CHANGE1)
{
	// this is the interrupt vector for when the pin state changes
	serSend0(LApinin); // place in serial buffer
}

void init_main()
{
	cbi(MCUCR, 4); // enable pullups

	WDTCSR = 0; // watchdog off
	sei(); // enables all interrupts, macro

	serInit0();

	LAport = 0; // disable pullups
	LAddr = 0;

	LAmask = 0b00111111; // enable all except for PCINT14 which is not on port C
	sbi(PCICR, 1); // enable interrupt 8 to 14, but the previous line disables 14
	// we only want 8 to 13
}

void main_loop()
{
	// do nothing
}

int main()
{
	// this is my standard program structure
	init_main();
	while(1)
	{
		main_loop();
		
	}
	return 0;
}
