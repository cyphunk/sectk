#include <stdio.h>
#include <unistd.h>
#include <usb.h>
#include "ftdibb.h"
#ifdef linux
#include <sys/types.h>
#include <stdlib.h>
#endif

// these are the bit definitions for the lines which are connected:
#define PIN0 1
#define PIN1 2
#define PIN2 4
#define PIN3 8
#define PIN4 16
#define PIN5 32
#define PIN6 64
#define PIN7 128

int main(int argc, char **argv)
{
	usb_dev_handle *dev_handle;
	unsigned char selpin;
#ifdef linux
	if(geteuid()!=0){
		fprintf(stderr,"ERROR: this program must run with effective uid=root.\n");
		/* you can do this:
		 * chown root.root bitbang_example
		 * chmod 4755 bitbang_example
		 * ls -al bitbang_example
		 * -rwsr-xr-x    1 root     root        11832 Aug 11 18:53 bitbang_example
                 * now you can run as normal user.
		 */
		exit(1);
	}
	// set also the real uid to be able to run rmmod
	if (setuid(0)==0){
		system("/sbin/rmmod ftdi_sio > /dev/null 2>&1");
	}else{
		fprintf(stderr,"Warning: suid root failed\n");
	}
#endif
	
	dev_handle = ftdibb_open(0x0403, 0x6001,0);

	if (dev_handle==NULL){
		exit(1);
	}
	if (ftdibb_setBaudRate(dev_handle,9600)!=0){
		exit(2);
	}
	// set which lines are output, the rest is input:
	if (ftdibb_setBitMode(dev_handle,PIN1|PIN2|PIN3, 1)!=0){
		exit(3);
	}
	printf("bitbang mode on\n");
	printf("lights will run until you press the button on pin4\n");

	while(ftdibb_getpin(dev_handle,PIN4)==1){
		printf("PIN1 on\n");
		selpin=PIN1;
		ftdibb_writeDatabyte(dev_handle,selpin);
		usleep(100000);
		selpin=0;
		ftdibb_writeDatabyte(dev_handle,selpin);
		printf("PIN2 on\n");
		selpin=PIN2;
		ftdibb_writeDatabyte(dev_handle,selpin);
		if (ftdibb_getpin(dev_handle,PIN4)==0){
			break;
		}
		usleep(100000);
		selpin=0;
		ftdibb_writeDatabyte(dev_handle,selpin);
		printf("PIN3 on\n");
		selpin=PIN3;
		ftdibb_writeDatabyte(dev_handle,selpin);
		usleep(100000);
	}
	printf("turning everything off\n");
	selpin=0;
	ftdibb_writeDatabyte(dev_handle,selpin);

	printf("Will not disable bitbang mode in order to leave lights in off state...\n");
	//if (ftdibb_setBitMode(dev_handle,0xff, 0)!=0){
	//	printf("DBG disable failed\n");
	//	exit(2);
	//}
	ftdibb_close(dev_handle);
	return(0);
}
