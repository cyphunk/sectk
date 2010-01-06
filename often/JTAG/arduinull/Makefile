all:
	avr-gcc -Wall -O3 -o arduinull -mmcu=atmega328p arduinull.c
	avr-objdump -h -S arduinull > arduinull.lst
	avr-objcopy -j .text -j .data -O ihex arduinull arduinull.hex 

clean:
	rm -f arduinull *.hex *.lst

load:
	avrdude -p m328p -P /dev/ttyUSB0 -c stk500v1 -b 57600 -F -u -U flash:w:arduinull.hex
