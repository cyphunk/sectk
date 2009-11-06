#define serOutSize0 1000 // stores 1000 previous numbers in buffer
// why so much? the ATMEGA168 has 1024 bytes of memory, so why not?
volatile uint8_t serBusy0;
volatile uint8_t serOut0[serOutSize0];
volatile uint16_t serOutHead0;
volatile uint16_t serOutTail0;

void serInit0()
{
	// setup UART pins with pull ups
	sbi(serPort, serRxPin);
	sbi(serPort, serTxPin);
	sbi(serDDR, serTxPin);
	cbi(serDDR, serRxPin);

	// setup baud rate
	UBRR0L = compBaud; // Comp
	UBRR0H = 0;
	UCSR0C = 0b00000110; // 8N1	
	UCSR0B = 0b11011000; // all int rx tx enable
	serBusy0 = 0; // clear to send

	serOutHead0 = 0;
	serOutTail0 = 0;
}

ISR(SIG_USART_RECV)
{
	LAmask = UDR0; // if new mask received, apply it
}

ISR(SIG_USART_TRANS)
{
	// if finished all, set flag to 0
	if(((serOutSize0 + serOutHead0 - serOutTail0) % serOutSize0) == 0)
	{
		serBusy0 = 0;
	}
	else
	{
		// transmit next byte
		uint8_t c = serOut0[serOutTail0];
		serOutTail0 = (serOutTail0 + 1) % serOutSize0;
		serBusy0 = 1;
		UDR0 = c;
	}
}

void serWaitFinished0()
{
	// wait until finished
	while(serBusy0 == 1);
}

void serSend0(uint8_t data)
{
	uint8_t c = data;

	// store in outbox buffer
	uint16_t i = (serOutHead0 + 1) % serOutSize0;
	if(i != serOutTail0)
	{
		serOut0[serOutHead0] = c;
		serOutHead0 = i;
	}

	// if first, then trigger transmission
	if(serBusy0 == 0)
	{
		uint8_t d = serOut0[serOutTail0];
		serOutTail0 = (serOutTail0 + 1) % serOutSize0;
		serBusy0 = 1;
		UDR0 = d;
	}
}
