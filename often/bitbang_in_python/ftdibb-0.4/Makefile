# makefile, written by guido socher
CFLAGS=-Wall -Wstrict-prototypes -I/usr/local/libusb/include
LDFLAGS=-L/usr/local/libusb/lib
STATIC=/usr/local/libusb/lib/libusb.a
#-------------------
all: bitbang_example bitbang_example_static bitbang_runlight_example bitbang_runlight_example_static
	@echo OK
#-------------------
ftdibb.o: ftdibb.h ftdibb.c 
	gcc ${CFLAGS} -c ftdibb.c
#-------------------
bitbang_example.o: ftdibb.h bitbang_example.c
	gcc ${CFLAGS} -c bitbang_example.c
#
bitbang_example: ftdibb.o bitbang_example.o 
	gcc ${LDFLAGS} -o bitbang_example bitbang_example.o ftdibb.o -lusb
#
bitbang_example_static: ftdibb.o bitbang_example.o 
	gcc ${LDFLAGS} -o bitbang_example_static bitbang_example.o ftdibb.o ${STATIC}
#-------------------
bitbang_runlight_example.o: ftdibb.h bitbang_runlight_example.c
	gcc ${CFLAGS} -c bitbang_runlight_example.c
#
bitbang_runlight_example: ftdibb.o bitbang_runlight_example.o 
	gcc ${LDFLAGS} -o bitbang_runlight_example bitbang_runlight_example.o ftdibb.o -lusb
#
bitbang_runlight_example_static: ftdibb.o bitbang_runlight_example.o 
	gcc ${LDFLAGS} -o bitbang_runlight_example_static bitbang_runlight_example.o ftdibb.o ${STATIC}
#-------------------
clean:
	rm -f *.o bitbang_example bitbang_runlight_example_static bitbang_runlight_example bitbang_example_static
#-------------------
