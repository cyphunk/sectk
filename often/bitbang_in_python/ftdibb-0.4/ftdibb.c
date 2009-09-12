/* ftdi bit bang lib written by Guido Socher (guido at linuxfocus.org)
 * Copyright: GPL 
 * Requires: libusb , libusb writes to the files in /proc/bus/usb/00X
 *                    root rights or suid root (chmod 4755) is therefore
 *                    needed.
 */
#include <usb.h>
#include <stdio.h>

// open the usb devices identified by vendor and product.
// claim exlclusive access to the interface and enable bit bang mode
// return NULL on failure otherwise a valid usb_dev_handle descriptor
//
// The standard ftdi232bm vendor and product IDs are: 0x0403, 0x6001
usb_dev_handle *ftdibb_open(int vendor, int product,int silent)
{
	usb_dev_handle *dev_handle;
	struct usb_bus *busses;
	struct usb_bus *bus;
	struct usb_device *dev;
	int founddev=0;
    
	usb_init();
	usb_find_busses();
	usb_find_devices();

	busses = usb_get_busses();
	for (bus = usb_busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idVendor == vendor 
				&& dev->descriptor.idProduct == product) {
				dev_handle = usb_open(dev);
				if (dev_handle){
					founddev=1;	
					break;
				}else{
					if (silent==0 || silent==1){
						fprintf(stderr,"ftdibb error: Device found but usb_open failed\n");
					}
					return(NULL); // device found but open fail
				}
			}
		}
	}
	if(founddev==0){
		if (silent==0){
			fprintf(stderr,"ftdibb error: Device not connected\n");
		}
		return(NULL); // device not found 
	}
	if (usb_claim_interface(dev_handle, 0) != 0) {
		usb_close (dev_handle);
		if (silent==0 || silent==1){
			fprintf(stderr,"ftdibb error: unable to claim exclusive access to ftdi chip. Make sure ftdi_sio is not loaded (check with lsmod) and make sure you have write access to /proc/bus/usb (root right or suid executable)\n");
		}
		return(NULL); // usb_claim_interface failed
	}
	// now we are ready to configure the device
	// the syntax to send a control urb is this:
	// usb_control_msg(usb_dev_handle *dev, int requesttype, int request, int value, int index, char *bytes, int size, int timeout);

	// reset  it first
	if (usb_control_msg(dev_handle, 0x40, 0, 0, 0, NULL, 0, 4000) != 0){
		usb_close (dev_handle);
		if (silent==0 || silent==1){
			fprintf(stderr,"ftdibb error: Device communication failed during reset\n");
		}
		return(NULL); // reset fail
	}
	return(dev_handle);
}

// sets the baudrate to 9600, the bitbang baudrate is actually 16 times higher
// but you do not need to care about this.
// dev_handle is whatever you get returned when you do a ftdibb_open
int ftdibb_setBaudRate(usb_dev_handle *dev_handle,int baudrate)
{
	unsigned short value, index;
	/* I did not quite understand how they calculate the baud rate
 	* therefore you can not set any baud rate at this time. Some bits
	* in the bmRequest value are representing values to be added to the 
	* base baud rate.
	* The basic formula seems to be 3000000/Value=baudrate
	* For 9600 this is: 3000000/312=9615 This is approx. 9600
	* 312 is binary 100111000
	* As value we write: 01000001 00111000 = 16696
	* 
	* Values to use for a 48MHz FPGA (external crystal to the ft232bm is still 6MHz):
	* Value  Baud Rate speed
	* 0x2710 300
	* 0x1388 600
	* 0x09C4 1200
	* 0x04E2 2400
	* 0x0271 4800
	* 0x4138 9600
	* 0x809C 19200
	* 0xC04E 38400
	* 0x0034 57600
	* 0x001A 115200
	* 0x000D 230400
	* 0x4006 460800
	* 0x8003 921600
	*/
	int brate[]={300,600,1200,2400,4800,9600,19200,38400,57600,115200,230400,460800,921600,0};
	unsigned short brateval[]={0x2710,0x1388,0x09C4,0x04E2,0x0271,0x4138,0x809C,0xC04E,0x0034,0x001A,0x000D,0x4006,0x8003};
	int i=0;
	value=16696; // default is 9600
	// select the best fitting baud rate:
	while(brate[i]){
		if (baudrate<=brate[i]){
			value=brateval[i];
			break;
		}
		i++;
	}
	index=0;
	if (usb_control_msg(dev_handle, 0x40, 3, value, index, NULL, 0, 4000) != 0){
		usb_close (dev_handle);
		fprintf(stderr,"ftdibb error: setBaudRate failed\n");
		return(-1);
	}
	return(0);
}

// enable or disable bitbang mode. pin_bitmask defines which pins are input and which are
// output (a one is an output, all 8 pins output = 0xff).
// dev_handle is whatever you get returned when you do a ftdibb_open
//
//  Pin Definitions
//  FT245BM FT232BM    Bit-Bang Data bit
//  Data0     TXD      Data0
//  Data1     RXD      Data1
//  Data2     RTS      Data2
//  Data3     CTS      Data3
//  Data4     DTR      Data4
//  Data5     DSR      Data5
//  Data6     DCD      Data6
//  Data7     RI       Data7
//
int ftdibb_setBitMode(usb_dev_handle *dev_handle,unsigned char pin_bitmask, unsigned char onoff)
{
	unsigned short value;

	// lower 8 byte is bitmask, 
	//Set Bit Bang Mode
	//40 -> bmRequestType
	//0B -> bmRequest
	//pin_bitmask -> lValue
	//Enable -> hValue
	// rest is zero
	//This configures which pins are input and which are output during bitbang. The
	//pin_bitmask byte sets the direction. A 1 means the corresponding pin is
	//an output (zero means input).  The Enable byte will turn the bit bang
	//mode off (0) and on (1).
	value=pin_bitmask & 0xff;
	if (onoff){
		value|= 0x100; // enable bitbang
	}
	if (usb_control_msg(dev_handle, 0x40, 0x0B, value, 0, NULL, 0, 4000) != 0){
		usb_close (dev_handle);
		fprintf(stderr,"ftdibb error: can not set bitbang mode. Not a ftdi BM chip??\n");
		return(-1); // bitbang init fail
	}
	return(0);
}

//This function does an immediate (unbuffered) read of the 8 pins and passes back the value.
//This function will exit (terminate the program if it fails. In other words
//no error checking is neede on the user side)
unsigned char ftdibb_getBitMode(usb_dev_handle *dev_handle)
{
	unsigned short value;
	//Read Data pins
	//C0 -> bmRequestType
	//0C -> bmRequest
	// rest is zero
	//This function does an immediate read of the 8 pins and passes back the
	//value. This is useful to see what the pins are doing now. The normal read
	//pipe will contain the same result but it has also been sampling the pins
	//continuously (up until its buffers become full). Therefore the data in
	//the read pipe will be old.
	if (usb_control_msg(dev_handle, 0xC0, 0x0C, 0, 0, (char *)&value, 1, 200) != 1){
		usb_close (dev_handle);
		fprintf(stderr,"ftdibb error: can not read data pins\n");
		exit(-1);
	}
	return((unsigned char)value);
}


// just get the status of the one pin (e.g pin=4 is thirs pin, pin=8 is fourth pin):
int ftdibb_getpin(usb_dev_handle *dev_handle,unsigned char pin)
{
	return((ftdibb_getBitMode(dev_handle) & pin)&&1);
}

// this function will exit if there is an error. No checking on
// user side needed.
int ftdibb_writeData(usb_dev_handle *dev_handle, unsigned char *buf, int size)
{
	int total_written;
	int writebuffer_chunksize = 4096;
	int in_ep = 0x02;
	//int out_ep = 0x81;
	if (size>writebuffer_chunksize){
		fprintf(stderr,"ftdibb error: can not write more than %d bytes at a time\n",writebuffer_chunksize);
		exit(-1);
	}
	total_written = usb_bulk_write(dev_handle, in_ep, (char *)buf, size, 200);
	if (total_written < 0){
		fprintf(stderr,"ftdibb error: write data\n");
		exit(-1);
	}
	return(total_written);
}

// most useful for bit bang
int ftdibb_writeDatabyte(usb_dev_handle *dev_handle, unsigned char byte)
{
	return(ftdibb_writeData(dev_handle,&byte,1));
}

void ftdibb_close(usb_dev_handle *dev_handle)
{
	usb_close (dev_handle);
}

// Set buffer latency for buffers which are not completley full
// The default is 16 after a reset
int ftdibb_setLatencyTimer(usb_dev_handle *dev_handle,unsigned char latency)
{
	//40 -> bmRequestType
	//09 -> bmRequest
	//latency -> lValue
	//0 -> hValue
	// rest is zero
	if (latency ==0 ){
		// only 1-255 are valid
		latency=1;
	}
	if (usb_control_msg(dev_handle, 0x40, 0x09,(int)latency, 0, NULL, 0, 4000) != 0){
		usb_close (dev_handle);
		fprintf(stderr,"ftdibb error: can not set buffer latency\n");
		return(-1); 
	}
	return(0);
}

// -- end of ftdibb
