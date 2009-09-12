WORK IN PROGRESS

GoodFET bit jigging platform from Travis Goodspeed:
http://goodfet.sourceforge.net/

There are various hardware incarnations that Travis has made.  I happen to be using the GoodFET 11. The notes herein relate to getting his toolkit working on OSX. The code I'm working off of was grabbed today, Sept 11 2009.
http://travisgoodspeed.blogspot.com/2009/06/goodfet-msp430-tutorial.html


1. install MSPGCC
I took this route:
http://sourceforge.net/apps/mediawiki/mspgcc/index.php?title=Linux_installation
http://www.sics.se/contiki/tutorials/tutorial-installing-and-using-contiki/tmote-sky-on-an-intel-based-mac.html
http://www.mikrocontroller.net/articles/MSPGCC

2. Make client
   cd trunk/client && sudo make link

3. Make firmware
   cd trunk/firmware && make
If you get an error, make sure that msp430 gcc is in your PATH.

4. Install firmware
Change the ttyUSB0 line in trunk/firmware/Makefile:
   #GOODFET?=/dev/ttyUSB0
   GOODFET?=/dev/tty.usbserial-A6007O8a
ls /dev/tty.* to determine the proper device.  Now install the firmware:
   make install

5. Dump BSL
   goodfet.msp430 dump bsl.hex 0xC00 0xFFF
This appears to get stuck, the LED's go on for a split second and then are off.  The common header "Initilizing MSP430" from the tutorial is not printed to the console.  When pressing ctrl+c I get:
Traceback (most recent call last):
  File "/usr/local/bin/goodfet.msp430", line 25, in <module>
    client.MSP430setup();
  File "/Users/cyphunk/sectk/often/goodfet/goodfet/trunk/client/GoodFETMSP430.py", line 20, in MSP430setup
    self.writecmd(self.MSP430APP,0x10,0,None);
  File "/Users/cyphunk/sectk/often/goodfet/goodfet/trunk/client/GoodFET.py", line 60, in writecmd
    self.readcmd(blocks);  #Uncomment this later, to ensure a response.
  File "/Users/cyphunk/sectk/often/goodfet/goodfet/trunk/client/GoodFET.py", line 63, in readcmd
    self.app=ord(self.serialport.read(1));
  File "/Library/Python/2.5/site-packages/serial/serialposix.py", line 452, in read
    ready,_,_ = select.select([self.fd], [], [], self._timeout)
KeyboardInterrupt

