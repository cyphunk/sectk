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
If you get an error, make sure that msp430 gcc is installed properly and that it is in your PATH.

4. Install firmware
The Makefile looks for the GoodFET at /dev/ttyUSB0 by default.  On OSX FTDI serial devices are more commonly found on /dev/tty.usbserial-*.  Set the GOODFET enviornment variable to the handle found on your machine (ls -l /dev/tty.*):
   export GOODFET=/dev/tty.usbserial-A6007O8a
Now install firmware:
   make install

5. Dump BSL
This is a the step Travis recommends in the tutorial.
   goodfet.msp430 dump bsl.hex 0xC00 0xFFF
This is the step at which you will need to plug your target MSP430 up to the goodfet.  I dont have a target so im skipping this step.  Instead I will try some of the monitoring and test applications:

5. Check IVT and Test on goodfet
   goodfet.monitor ivt
This appears to get stuck, the LED's go on for a split second and then are off.  The common header "Initilizing MSP430" from the tutorial is not printed to the console.  When pressing ctrl+c I get:
  File "/goodfet/goodfet/trunk/client/GoodFET.py", line 60, in writecmd
    self.readcmd(blocks);  #Uncomment this later, to ensure a response.
  File "/goodfet/trunk/client/GoodFET.py", line 63, in readcmd
    self.app=ord(self.serialport.read(1));
  File "/Library/Python/2.5/site-packages/serial/serialposix.py", line 452, in read
    ready,_,_ = select.select([self.fd], [], [], self._timeout)
KeyboardInterrupt

After talking to travis he suggested that Pin 33 of the MSP430 was not properly soldered.  Sure enough I had not bonded it properly during soldering.
"The log shows that the crystal is proper, the timing works, and thetransmit pin of the MSP430 are all properly soldered.  Being able toprogram it means that the pins of the FTDI are properly soldered. The receive pin of the MSP430 is not properly soldered, and because ofthis the FET cannot receive traffic from the GoodFET.  It is pin 33,on the corner opposite the chip's orientation marking." -- travis

After correcting this (http://www.flickr.com/photos/deadhacker/3915240445/) commands such as goodfet.monitor ivt, goodfet.monitor test -- both work.

NOTE: if you get the message "Verb 00 is wrong.  Incorrect firmware?" Travis suggest that this is an OSX issue when you execute the first command with the GoodFET. Attempt the command again.
