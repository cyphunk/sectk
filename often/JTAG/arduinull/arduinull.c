#include <avr/io.h>
#include <stdint.h>
#include <string.h>

static void uart_init(void)
{
	/* USART 0 */
	/* no USART IRQ, disable TX/RX */
	UCSR0B = 0x00;
	/* clk 2x */
	UCSR0A = 0x00;
	/* async, no parity, 8bit */
	UCSR0C = 0x06;
	/* baud rate (115200bps, 16MHz, 2x) */
	/* real rate: 111111bps (9 microsecs/bit) */
	UBRR0H = 0;
	UBRR0L = 8;
	/* no USART IRQ, enable TX (no RX) */
	UCSR0B = ((1<<RXEN0) | (1<<TXEN0));
}

static char uart_rcv()
{
	while(!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

static void uart_tx(char byte)
{
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = byte;
}

static void print(const char *str)
{
	while(*str) {
		uart_tx(*str);
		str++;
	}
}

static void digit(int d)
{
	uart_tx(d+'0');
}

static void pulse_tms(int tck, int tms, int s_tms)
{
	int v;
	v = s_tms ? (1 << tms) : 0;
	PORTC = v;
	PORTC = v|(1 << tck);
}

static void pulse_tdi(int tck, int tdi, int s_tdi)
{
	int v;
	v = s_tdi ? (1 << tdi) : 0;
	PORTC = v;
	PORTC = v|(1 << tck);
}

/*
 * Sets all pins to input except TCK and TMS
 * and attempt to go to Shift IR state.
 */
static void setup(int tck, int tms, int tdi)
{
	int i;
	
	PORTC = 0;
	DDRC = (1 << tck)|(1 << tms)|(1 << tdi);
	
	// Go to Test-Logic-Reset
	for(i=0;i<5;i++)
		pulse_tms(tck, tms, 1);
	// Go to Run-Test-Idle
	pulse_tms(tck, tms, 0);
	// Go to Select DR
	pulse_tms(tck, tms, 1);
	// Go to Select IR
	pulse_tms(tck, tms, 1);
	// Go to Capture IR
	pulse_tms(tck, tms, 0);
	// Go to Shift IR
	pulse_tms(tck, tms, 0);
}

#define PATTERN_LEN 64
static char pattern[PATTERN_LEN] = "0110011101001101101000010111001001";

static int check_data(int tck, int tdi, int tdo)
{
	int i;
	int wpointer;
	int tdo_read;
	int tdo_expected;
	int tdo_first;
	int recognized;
	int plen;
	
	wpointer = 0;
	recognized = 0;
	plen = strlen(pattern);
	
	if(pattern[0] == '0')
		tdo_first = 0;
	else
		tdo_first = 1;
	
	for(i=0;i<(2*PATTERN_LEN);i++) {
		/* Shift in */
		if(pattern[wpointer] == 0)
			wpointer = 0;
		if(pattern[wpointer] == '0')
			pulse_tdi(tck, tdi, 0);
		else
			pulse_tdi(tck, tdi, 1);
		wpointer++;
		
		/* Check what comes out */
		if(PINC & (1 << tdo))
			tdo_read = 1;
		else
			tdo_read = 0;
		if(pattern[recognized] == '0')
			tdo_expected = 0;
		else
			tdo_expected = 1;
		if(tdo_read == tdo_expected) {
			recognized++;
			if(recognized == plen)
				return 1;
		} else {
			if(tdo_read == tdo_first)
				recognized = 1;
			else
				recognized = 0;
		}
	}
	
	return 0;
}

static void scan()
{
	int tck, tms, tdo, tdi;
	
	print(
		"================================\n"
		"Starting scan...\n");
	for(tck=0;tck<6;tck++) {
		for(tms=0;tms<6;tms++) {
			if(tms == tck) continue;
			for(tdo=0;tdo<6;tdo++) {
				if(tdo == tck) continue;
				if(tdo == tms) continue;
				for(tdi=0;tdi<6;tdi++) {
					if(tdi == tck) continue;
					if(tdi == tms) continue;
					if(tdi == tdo) continue;
					setup(tck, tms, tdi);
					if(check_data(tck, tdi, tdo)) {
						print("Found! ");
						print("tck:");
						digit(tck);
						print(" tms:");
						digit(tms);
						print(" tdo:");
						digit(tdo);
						print(" tdi:");
						digit(tdi);
						print("\n");
					}
				}
			}
		}
	}
	print("================================\n");
}

void set_pattern()
{
	int i;
	char c;
	
	print("Enter new pattern: ");
	i = 0;
	while(1) {
		c = uart_rcv();
		switch(c) {
			case '0':
			case '1':
				if(i < (PATTERN_LEN-1)) {
					pattern[i++] = c;
					uart_tx(c);
				}
				break;
			case '\r':
			case '\n':
				pattern[i] = 0;
				uart_tx('\n');
				return;
		}
	}
}

int main(void)
{
	char c;
	
	uart_init();
	
	print("jtag scanner for atmega328p\ncompatible with diy arduinoob duemilanewb 2.0\nso that it has a chance to make hackaday\n -- lekernel@hsb, /tmp/export tour, june 2009\n\n");
	while(1) {
		print("\n"
		      "s > scan\n"
		      "p > set pattern [");
		print(pattern);
		print("]\n");
		c = uart_rcv();
		switch(c) {
			case 's':
				scan();
				break;
			case 'p':
				set_pattern();
				break;
		}
	}
	
	return 0;
}
