#!/usr/bin/env python
# GoodFET Client Library
# 
# (C) 2009 Travis Goodspeed <travis at radiantmachines.com>
#
# This code is being rewritten and refactored.  You've been warned!

import sys, time, string, cStringIO, struct, glob, serial, os;


class GoodFET:
    """GoodFET Client Library"""
    def __init__(self, *args, **kargs):
        self.data=[0];
    def timeout(self):
        print "timeout\n";
    def serInit(self, port=None):
        """Open the serial port"""
        
        if port is None and os.environ.get("GOODFET")!=None:
            glob_list = glob.glob(os.environ.get("GOODFET"));
            if len(glob_list) > 0:
                port = glob_list[0];
        if port is None:
            glob_list = glob.glob("/dev/tty.usbserial*");
            if len(glob_list) > 0:
                port = glob_list[0];
        if port is None:
            glob_list = glob.glob("/dev/ttyUSB*");
            if len(glob_list) > 0:
                port = glob_list[0];
        
        self.serialport = serial.Serial(
            port,
            #9600,
            115200,
            parity = serial.PARITY_NONE
            )
        #Drop DTR, which is !RST, low to begin the app.
        self.serialport.setDTR(0);
        self.serialport.flushInput()
        self.serialport.flushOutput()
        
        #Read and handle the initial command.
        #time.sleep(1);
        self.readcmd(); #Read the first command.
        if(self.verb!=0x7F):
            print "Verb %02x is wrong.  Incorrect firmware?" % self.verb;
        #print "Connected."
    def writecmd(self, app, verb, count=0, data=[], blocks=1):
        """Write a command and some data to the GoodFET."""
        self.serialport.write(chr(app));
        self.serialport.write(chr(verb));
        self.serialport.write(chr(count));
        #print "count=%02x, len(data)=%04x" % (count,len(data));
        if count!=0:
            for d in data:
                self.serialport.write(chr(d));
        
        self.readcmd(blocks);  #Uncomment this later, to ensure a response.
    def readcmd(self,blocks=1):
        """Read a reply from the GoodFET."""
        self.app=ord(self.serialport.read(1));
        self.verb=ord(self.serialport.read(1));
        self.count=ord(self.serialport.read(1));
        self.data=self.serialport.read(self.count*blocks);
        #print "READ %02x %02x %02x " % (self.app, self.verb, self.count);
        return self.data;
        
    #Monitor stuff
    def out(self,byte):
        """Write a byte to P5OUT."""
        self.writecmd(0,0xA1,1,[byte]);
    def dir(self,byte):
        """Write a byte to P5DIR."""
        self.writecmd(0,0xA0,1,[byte]);
    def peekbyte(self,address):
        """Read a byte of memory from the monitor."""
        self.data=[address&0xff,address>>8];
        self.writecmd(0,0x02,2,self.data);
        #self.readcmd();
        return ord(self.data[0]);
    def peekword(self,address):
        """Read a word of memory from the monitor."""
        return self.peekbyte(address)+(self.peekbyte(address+1)<<8);
    def pokebyte(self,address,value):
        """Set a byte of memory by the monitor."""
        self.data=[address&0xff,address>>8,value];
        self.writecmd(0,0x03,3,self.data);
        return ord(self.data[0]);
    def dumpmem(self,begin,end):
        i=begin;
        while i<end:
            print "%04x %04x" % (i, self.peekword(i));
            i+=2;
    def monitor_ram_pattern(self):
        """Overwrite all of RAM with 0xBEEF."""
        self.writecmd(0,0x90,0,self.data);
        return;
    def monitor_ram_depth(self):
        """Determine how many bytes of RAM are unused by looking for 0xBEEF.."""
        self.writecmd(0,0x91,0,self.data);
        return ord(self.data[0])+(ord(self.data[1])<<8);
    
    #Baud rates.
    baudrates=[115200, 
               9600,
               19200,
               38400,
               57600,
               115200];
    def setBaud(self,baud):
        """Change the baud rate.  TODO fix this."""
        rates=self.baudrates;
        self.data=[baud];
        print "Changing FET baud."
        self.serialport.write(chr(0x00));
        self.serialport.write(chr(0x80));
        self.serialport.write(chr(1));
        self.serialport.write(chr(baud));
        
        print "Changed host baud."
        self.serialport.setBaudrate(rates[baud]);
        time.sleep(1);
        self.serialport.flushInput()
        self.serialport.flushOutput()
        
        print "Baud is now %i." % rates[baud];
        return;
    def readbyte(self):
        return ord(self.serialport.read(1));
    def findbaud(self):
        for r in self.baudrates:
            print "\nTrying %i" % r;
            self.serialport.setBaudrate(r);
            #time.sleep(1);
            self.serialport.flushInput()
            self.serialport.flushOutput()
            
            for i in range(1,10):
                self.readbyte();
            
            print "Read %02x %02x %02x %02x" % (
                self.readbyte(),self.readbyte(),self.readbyte(),self.readbyte());
    def monitortest(self):
        """Self-test several functions through the monitor."""
        print "Performing monitor self-test.";
        
        if self.peekword(0x0c00)!=0x0c04 and self.peekword(0x0c00)!=0x0c06:
            print "ERROR Fetched wrong value from 0x0c04.";
        self.pokebyte(0x0021,0); #Drop LED
        if self.peekbyte(0x0021)!=0:
            print "ERROR, P1OUT not cleared.";
        self.pokebyte(0x0021,1); #Light LED
        
        print "Self-test complete.";
    
    

    def I2Csetup(self):
        """Move the FET into the I2C application."""
        self.writecmd(0x02,0x10,0,self.data); #SPI/SETUP
    def I2Cstart(self):
        """Start an I2C transaction."""
        self.writecmd(0x02,0x20,0,self.data); #SPI/SETUP
    def I2Cstop(self):
        """Stop an I2C transaction."""
        self.writecmd(0x02,0x21,0,self.data); #SPI/SETUP
    def I2Cread(self,len=1):
        """Read len bytes by I2C."""
        self.writecmd(0x02,0x00,1,[len]); #SPI/SETUP
        return self.data;
    def I2Cwrite(self,bytes):
        """Write bytes by I2C."""
        self.writecmd(0x02,0x01,len(bytes),bytes); #SPI/SETUP
        return ord(self.data[0]);
class GoodFETCC(GoodFET):
    """A GoodFET variant for use with Chipcon 8051 Zigbe SoC."""
    def CChaltcpu(self):
        """Halt the CPU."""
        self.writecmd(0x30,0x86,0,self.data);
    def CCreleasecpu(self):
        """Resume the CPU."""
        self.writecmd(0x30,0x87,0,self.data);
    def CCtest(self):
        self.CCreleasecpu();
        self.CChaltcpu();
        #print "Status: %s" % self.CCstatusstr();
        
        #Grab ident three times, should be equal.
        ident1=self.CCident();
        ident2=self.CCident();
        ident3=self.CCident();
        if(ident1!=ident2 or ident2!=ident3):
            print "Error, repeated ident attempts unequal."
            print "%04x, %04x, %04x" % (ident1, ident2, ident3);
        
        #Single step, printing PC.
        print "Tracing execution at startup."
        for i in range(1,15):
            pc=self.CCgetPC();
            byte=self.CCpeekcodebyte(i);
            #print "PC=%04x, %02x" % (pc, byte);
            self.CCstep_instr();
        
        print "Verifying that debugging a NOP doesn't affect the PC."
        for i in range(1,15):
            pc=self.CCgetPC();
            self.CCdebuginstr([0x00]);
            if(pc!=self.CCgetPC()):
                print "ERROR: PC changed during CCdebuginstr([NOP])!";
        
        
        #print "Status: %s." % self.CCstatusstr();
        #Exit debugger
        self.CCstop();
        print "Done.";

    def CCsetup(self):
        """Move the FET into the CC2430/CC2530 application."""
        #print "Initializing Chipcon.";
        self.writecmd(0x30,0x10,0,self.data);
    def CCrd_config(self):
        """Read the config register of a Chipcon."""
        self.writecmd(0x30,0x82,0,self.data);
        return ord(self.data[0]);
    def CCwr_config(self,config):
        """Write the config register of a Chipcon."""
        self.writecmd(0x30,0x81,1,[config&0xFF]);
    
    CCversions={0x0100:"CC1110",
                0x8500:"CC2430",
                0x8900:"CC2431",
                0x8100:"CC2510",
                0x9100:"CC2511",
                0xFF00:"CCmissing"};
    def CCidentstr(self):
        ident=self.CCident();
        chip=self.CCversions.get(ident&0xFF00);
        return "%s/r%02x" % (chip, ident&0xFF); 
    def CCident(self):
        """Get a chipcon's ID."""
        self.writecmd(0x30,0x8B,0,None);
        chip=ord(self.data[0]);
        rev=ord(self.data[1]);
        return (chip<<8)+rev;
    def CCgetPC(self):
        """Get a chipcon's PC."""
        self.writecmd(0x30,0x83,0,None);
        hi=ord(self.data[0]);
        lo=ord(self.data[1]);
        return (hi<<8)+lo;
    def CCdebuginstr(self,instr):
        self.writecmd(0x30,0x88,len(instr),instr);
        return ord(self.data[0]);
    def CCpeekcodebyte(self,adr):
        """Read the contents of code memory at an address."""
        self.data=[adr&0xff, (adr&0xff00)>>8];
        self.writecmd(0x30,0x90,2,self.data);
        return ord(self.data[0]);
    def CCpeekdatabyte(self,adr):
        """Read the contents of data memory at an address."""
        self.data=[adr&0xff, (adr&0xff00)>>8];
        self.writecmd(0x30,0x91, 2, self.data);
        return ord(self.data[0]);
    def CCpokedatabyte(self,adr,val):
        """Write a byte to data memory."""
        self.data=[adr&0xff, (adr&0xff00)>>8, val];
        self.writecmd(0x30, 0x92, 3, self.data);
        return ord(self.data[0]);
    def CCchiperase(self):
        """Erase all of the target's memory."""
        self.writecmd(0x30,0x80,0,None);
    def CCstatus(self):
        """Check the status."""
        self.writecmd(0x30,0x84,0,None);
        return ord(self.data[0])
    CCstatusbits={0x80 : "erased",
                  0x40 : "pcon_idle",
                  0x20 : "halted",
                  0x10 : "pm0",
                  0x08 : "halted",
                  0x04 : "locked",
                  0x02 : "oscstable",
                  0x01 : "overflow"};
    def CCstatusstr(self):
        """Check the status as a string."""
        status=self.CCstatus();
        str="";
        i=1;
        while i<0x100:
            if(status&i):
                str="%s %s" %(self.CCstatusbits[i],str);
            i*=2;
        return str;
    def CCstart(self):
        """Start debugging."""
        self.writecmd(0x30,0x20,0,self.data);
        ident=self.CCidentstr();
        print "Target identifies as %s." % ident;
        #print "Status: %s." % self.CCstatusstr();
        self.CCreleasecpu();
        self.CChaltcpu();
        #print "Status: %s." % self.CCstatusstr();
        
    def CCstop(self):
        """Stop debugging."""
        self.writecmd(0x30,0x21,0,self.data);
    def CCstep_instr(self):
        """Step one instruction."""
        self.writecmd(0x30,0x89,0,self.data);

