/* ftdibb.h written by Guido Socher
 * copyright GPL
 * FTDI bit bang library
 */

#ifndef __ftdibb_h__
#define __ftdibb_h__

#include <usb.h>
// To use this library you need write access to /proc/bus/usb
// This is because we use libusb to send the urb message.
// You can write a suid program to make it usable for a normal user (chmod 4755)

// Call sequence:
//    ftdibb_open 
//    ftdibb_setBaudRate for baud rate 9600 (reasonable value bit bang)
//    ftdibb_setBitMode with onoff=1
//
extern usb_dev_handle *ftdibb_open(int vendor, int product, int silent);
extern int ftdibb_setBaudRate(usb_dev_handle *dev_handle,int baudrate);
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
extern int ftdibb_setBitMode(usb_dev_handle *dev_handle,unsigned char pin_bitmask, unsigned char onoff);

// read data via bit bang
extern unsigned char ftdibb_getBitMode(usb_dev_handle *dev_handle);
// write data via bit bang
extern int ftdibb_writeData(usb_dev_handle *dev_handle, unsigned char *buf, int size);
extern int ftdibb_writeDatabyte(usb_dev_handle *dev_handle, unsigned char byte);

extern void ftdibb_close(usb_dev_handle *dev_handle);

// just get the status of the one pin (e.g pin=4 for the third pin):
extern int ftdibb_getpin(usb_dev_handle *dev_handle,unsigned char pin);

extern int ftdibb_setLatencyTimer(usb_dev_handle *dev_handle,unsigned char latency);
#endif /* __ftdibb_h__ */
