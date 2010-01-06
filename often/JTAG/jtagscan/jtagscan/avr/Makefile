all:
	avr-gcc -Wall -O3 -o jtagscan-avr -mmcu=atmega32 jtagscan-avr.c
	avr-objdump -h -S jtagscan-avr > jtagscan-avr.lst
	avr-objcopy -j .text -j .data -O ihex jtagscan-avr jtagscan-avr.hex 

clean:
	rm -f jtagscan-avr *.hex *.lst

load:
	avrdude -c stk500v2 -p m32 -B 20 -P /dev/ttyUSB0 -U flash:w:jtagscan-avr.hex
