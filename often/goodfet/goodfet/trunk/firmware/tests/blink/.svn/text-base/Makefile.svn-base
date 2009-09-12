
PORT=/dev/ttyUSB0
BSL=tos-bsl --invert-reset --invert-test -c $(PORT)

#mcu=msp430x1611
mcu=msp430x2618

#GCCINC=-T ../../ldscripts/161x.x
CC=msp430-gcc -g -mmcu=$(mcu) -DGCC $(GCCINC) -I ../../include




app=blink

all: $(app).hex
$(app).hex: $(app)
	msp430-objcopy $(app) -O ihex $(app).hex
install: $(app)
	$(BSL) -e -p $(app) 
	#$(BSL) -P $(app) -g 0x2500
	$(BSL) -P $(app) -r
erase:
	$(BSL) -e 
clean:
	rm $(app)
