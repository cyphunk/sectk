/***************************************************************************
 *            jtagscan-avr.c
 *
 *  Wed May 31 18:57:23 2006
 *  Copyright  2006  Benedikt 'Hunz' Heinz
 *  jtagscan at hunz.org
 *  $Id: jtagscan-avr.c,v 1.1 2006/05/31 17:17:50 hunz Exp $
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <avr/io.h>
#include <stdint.h>

void uart_init(void) {
	UBRRL = UBRRH = UCSRA = 0;
	UCSRB = ((1<<RXEN) | (1<<TXEN));
	UCSRC = ((1 << URSEL) | (1<<UPM1) | (3 << UCSZ0));
}

uint8_t uart_rcv() {
	while(!(UCSRA & (1<<RXC)));
	return UDR;
}

void uart_tx(uint8_t byte) {
	while(!(UCSRA & (1<<UDRE)));
	UDR = byte;
}

/* DIV, shifts, alterations, IOs */
#define CFG_DELAY		0
#define CFG_SHIFTS		1
#define CFG_TDIBITS	2
#define CFG_IOS			3
#define CFG_OUTS		7
#define CFG_LEN		(1+1+1+8)

uint8_t cfg[CFG_LEN];

#define TCK_PIN		0
#define TMS_PIN	1
#define TDI_PIN		2

uint8_t pins[3];

#define DELAY		delay=cfg[CFG_DELAY]; while(delay--);

void set_ios(void) {
	uint8_t iocfg[8], cnt;
	
	for(cnt=0; cnt<8; cnt++)
		iocfg[cnt] = cfg[CFG_IOS+cnt];
	
	/* TCK, TMS, TDI are outputs */
	iocfg[pins[0]>>3] |= (1<<(pins[0]&7));
	iocfg[pins[1]>>3] |= (1<<(pins[1]&7));
	iocfg[pins[2]>>3] |= (1<<(pins[2]&7));
	
	DDRA = iocfg[0];
	DDRB = iocfg[1];
	DDRC = iocfg[2];
	DDRD = iocfg[3];
	
	PORTA = iocfg[4];
	PORTB = iocfg[5];
	PORTC = iocfg[6];
	PORTD = iocfg[7];
	
}

uint8_t shifts[32], matches[32];

#define SHIFT_TMS			0
#define SHIFT_TDI				0x10

void shift(uint8_t data, uint8_t len) {
	uint8_t cnt, tck_port,tck_pin,port,pin;
	uint8_t delay, inp=0, count_matches = len & SHIFT_TDI;
	
	if(len&SHIFT_TDI) {
		port=pins[TDI_PIN]>>3;
		pin=pins[TDI_PIN]&7;
	}
	else {
		port=pins[TMS_PIN]>>3;
		pin=pins[TMS_PIN]&7;
	}
	
	tck_port=pins[TCK_PIN]>>3;
	tck_pin=pins[TCK_PIN]&7;
	
	len&=0x0f;
	
	/* shift */
	while(len--) {
		
		/* TCK=0 */
		if(tck_port == 0)
			PORTA &= ~(1<<tck_pin);
		else if(tck_port == 1)
			PORTB &= ~(1<<tck_pin);
		else if(tck_port == 2)
			PORTC &= ~(1<<tck_pin);
		else
			PORTD &= ~(1<<tck_pin);
		
		/* data bit */
		if(data & 1) {
			if(port == 0)
				PORTA |= (1<<pin);
			else if(port == 1)
				PORTB |= (1<<pin);
			else if(port == 2)
				PORTC |= (1<<pin);
			else
				PORTD |= (1<<pin);
		}
		else {
			if(port == 0)
				PORTA &= ~(1<<pin);
			else if(port == 1)
				PORTB &= ~(1<<pin);
			else if(port == 2)
				PORTC &= ~(1<<pin);
			else
				PORTD &= ~(1<<pin);
		}
		
		DELAY;
		
		/* shift in */
		if(count_matches) {
			
			for(cnt=0;cnt<32;cnt++) {
			
				if(!(cnt&7)) {
					if((cnt>>3) == 0)
						inp=PINA;
					else if((cnt>>3) == 1)
						inp=PINB;
					else if((cnt>>3) == 2)
						inp=PINC;
					else
						inp=PIND;
				}
			
				shifts[cnt]>>=1;
				shifts[cnt]|= (inp&1)<<7;
				inp>>=1;
				
				//if(cnt == pins[TDI_PIN])
					//uart_tx(shifts[cnt]);
			
				if( (shifts[cnt] == cfg[CFG_TDIBITS]) && (matches[cnt]<255) )
					matches[cnt]++;
			}
			
		} /* count_matches */
		
		/* TCK=1 */
		if(tck_port == 0)
			PORTA |= (1<<tck_pin);
		else if(tck_port == 1)
			PORTB |= (1<<tck_pin);
		else if(tck_port == 2)
			PORTC |= (1<<tck_pin);
		else
			PORTD |= (1<<tck_pin);
		
		DELAY;
		
		data >>= 1;
	}
	
}

void scan() {
	uint8_t cnt;
	
	for(cnt=0;cnt<32; cnt++) {
		matches[cnt]=0;
		shifts[cnt]=0;
	}
	
	/* get into shift IR state */
	shift(0x5f,8|SHIFT_TMS);
	
	/* shift IR */
	for(cnt=0; cnt<cfg[CFG_SHIFTS]; cnt++)
		shift(cfg[CFG_TDIBITS],8|SHIFT_TDI);
	
	/* get back into reset state */
	shift(0x1f,5|SHIFT_TMS);
	
	/* send the results */
	for(cnt=0; cnt<32;cnt++)
		uart_tx(matches[cnt]);
	
}

int main(void) {
	uint8_t val;
	
	uart_init();
	
	while(1) {
		val = uart_rcv();

		/* jtscan */		
		if(val&0x80) {
			val &= 0x7f;
			
			pins[0] = (val >> 2);
			pins[2] = uart_rcv();
			
			pins[1] = (val & 3) << 3;
			pins[1] |= (pins[2] >> 5);
			
			pins[2] &= 0x1f;
			
			set_ios();
			
			scan();
		}
		
		/* config */
		else if(val == 0x40) {
			
			for(val=0; val < CFG_LEN; val++)
				cfg[val] = uart_rcv();
			
			uart_tx(0x40);
		}
		
		else if(val == 0x00) {
			
			DDRA = uart_rcv();
			DDRB = uart_rcv();
			DDRC = uart_rcv();
			DDRD = uart_rcv();
			
			uart_tx(0);
		}
		
		else if(val == 0x01) {
			
			PORTA = uart_rcv();
			PORTB = uart_rcv();
			PORTC = uart_rcv();
			PORTD = uart_rcv();
			
			uart_tx(1);
		}
		
		else if(val == 0x02) {
			
			uart_tx(PINA);
			uart_tx(PINB);
			uart_tx(PINC);
			uart_tx(PIND);
			
		}
		
		/* TODO: TMS / TDI / get TDO / direct IO */
		
	} /* loop */
	
	return 0;
}
