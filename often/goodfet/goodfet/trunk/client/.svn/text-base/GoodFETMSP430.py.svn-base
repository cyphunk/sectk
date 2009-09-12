#!/usr/bin/env python
# GoodFET Client Library
# 
# (C) 2009 Travis Goodspeed <travis at radiantmachines.com>
#
# Presently being rewritten.

import sys, time, string, cStringIO, struct, glob, serial, os;

from GoodFET import GoodFET;

class GoodFETMSP430(GoodFET):
    MSP430APP=0x11;  #Changed by inheritors.
    CoreID=0;
    DeviceID=0;
    JTAGID=0;
    MSP430ident=0;
    def MSP430setup(self):
        """Move the FET into the MSP430 JTAG application."""
        self.writecmd(self.MSP430APP,0x10,0,None);
        
    def MSP430stop(self):
        """Stop debugging."""
        self.writecmd(self.MSP430APP,0x21,0,self.data);
    
    def MSP430coreid(self):
        """Get the Core ID."""
        self.writecmd(self.MSP430APP,0xF0);
        CoreID=ord(self.data[0])+(ord(self.data[1])<<8);
        return CoreID;
    def MSP430deviceid(self):
        """Get the Core ID."""
        self.writecmd(self.MSP430APP,0xF1);
        DeviceID=(
            ord(self.data[0])+(ord(self.data[1])<<8)+
            (ord(self.data[2])<<16)+(ord(self.data[3])<<24));
        return DeviceID;
    def MSP430peek(self,adr):
        """Read the contents of memory at an address."""
        self.data=[adr&0xff, (adr&0xff00)>>8,
                   (adr&0xff0000)>>16,(adr&0xff000000)>>24];
        self.writecmd(self.MSP430APP,0x02,4,self.data);
        return ord(self.data[0])+(ord(self.data[1])<<8);
    def MSP430poke(self,adr,val):
        """Write the contents of memory at an address."""
        self.data=[adr&0xff, (adr&0xff00)>>8, val&0xff, (val&0xff00)>>8];
        self.writecmd(self.MSP430APP,0x03,4,self.data);
        return ord(self.data[0])+(ord(self.data[1])<<8);
    def MSP430start(self):
        """Start debugging."""
        self.writecmd(self.MSP430APP,0x20,0,self.data);
        self.JTAGID=ord(self.data[0]);
        #print "Identified as %02x." % id;
        if(self.JTAGID==0x89 or self.JTAGID==0x91):
            print "Successfully connected."
        else:
            print "Error, misidentified as %02x." % id;
    
    def MSP430haltcpu(self):
        """Halt the CPU."""
        self.writecmd(self.MSP430APP,0xA0,0,self.data);
    def MSP430releasecpu(self):
        """Resume the CPU."""
        self.writecmd(self.MSP430APP,0xA1,0,self.data);
    def MSP430shiftir8(self,ins):
        """Shift the 8-bit Instruction Register."""
        data=[ins];
        self.writecmd(self.MSP430APP,0x80,1,data);
        return ord(self.data[0]);
    def MSP430shiftdr16(self,dat):
        """Shift the 16-bit Data Register."""
        data=[dat&0xFF,(dat&0xFF00)>>8];
        self.writecmd(self.MSP430APP,0x81,2,data);
        return ord(self.data[0])#+(ord(self.data[1])<<8);
    def MSP430setinstrfetch(self):
        """Set the instruction fetch mode."""
        self.writecmd(self.MSP430APP,0xC1,0,self.data);
        return self.data[0];
    def MSP430ident(self):
        """Grab self-identification word from 0x0FF0 as big endian."""
        if(self.JTAGID==0x89):
            i=self.MSP430peek(0x0ff0);
            ident=((i&0xFF00)>>8)+((i&0xFF)<<8)
        if(self.JTAGID==0x91):
            i=self.MSP430peek(0x1A04);
            ident=((i&0xFF00)>>8)+((i&0xFF)<<8)
        return ident;
    def MSP430test(self):
        """Test MSP430 JTAG.  Requires that a chip be attached."""
        if self.MSP430ident()==0xffff:
            print "Is anything connected?";
        print "Testing RAM.";
        temp=self.MSP430peek(0x0200);
        self.MSP430poke(0x0200,0xdead);
        if(self.MSP430peek(0x0200)!=0xdead):
            print "Poke of 0x0200 did not set to 0xDEAD properly.";
            return;
        self.MSP430poke(0x0200,temp); #restore old value.
    def MSP430flashtest(self):
        self.MSP430masserase();
        i=0x2500;
        while(i<0xFFFF):
            if(self.MSP430peek(i)!=0xFFFF):
                print "ERROR: Unerased flash at %04x."%i;
            self.MSP430writeflash(i,0xDEAD);
            i+=2;
    def MSP430masserase(self):
        """Erase MSP430 flash memory."""
        self.writecmd(self.MSP430APP,0xE3,0,None);
    def MSP430writeflash(self,adr,val):
        """Write a word of flash memory."""
        if(self.MSP430peek(adr)!=0xFFFF):
            print "FLASH ERROR: %04x not clear." % adr;
        data=[adr&0xFF,(adr&0xFF00)>>8,val&0xFF,(val&0xFF00)>>8];
        self.writecmd(self.MSP430APP,0xE1,4,data);
        rval=ord(self.data[0])+(ord(self.data[1])<<8);
        if(val!=rval):
            print "FLASH WRITE ERROR AT %04x.  Found %04x, wrote %04x." % (adr,rval,val);
        
    def MSP430dumpbsl(self):
        self.MSP430dumpmem(0xC00,0xfff);
    def MSP430dumpallmem(self):
        self.MSP430dumpmem(0x200,0xffff);
    def MSP430dumpmem(self,begin,end):
        i=begin;
        while i<end:
            print "%04x %04x" % (i, self.MSP430peek(i));
            i+=2;
