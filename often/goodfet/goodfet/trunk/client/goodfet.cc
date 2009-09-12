#!/usr/bin/env python

import sys;
import binascii;

from GoodFET import GoodFETCC;
from intelhex import IntelHex;


if(len(sys.argv)==1):
    print "Usage: %s verb [objects]\n" % sys.argv[0];
    print "%s test" % sys.argv[0];
    print "%s info" % sys.argv[0];
    print "%s dumpcode $foo.hex [0x$start 0x$stop]" % sys.argv[0];
    print "%s dumpdata $foo.hex [0x$start 0x$stop]" % sys.argv[0];
    print "%s erase" % sys.argv[0];
    print "%s writedata $foo.hex [0x$start 0x$stop]" % sys.argv[0];
    print "%s verify $foo.hex [0x$start 0x$stop]" % sys.argv[0];
    print "%s peekdata 0x$start [0x$stop]" % sys.argv[0];
    print "%s pokedata 0x$adr 0x$val" % sys.argv[0];
    #print "%s peekcode 0x$start [0x$stop]" % sys.argv[0];
    sys.exit();

#Initailize FET and set baud rate
client=GoodFETCC();
client.serInit()

#Connect to target
client.CCsetup();
client.CCstart();

if(sys.argv[1]=="test"):
    client.CCtest();
if(sys.argv[1]=="dumpcode"):
    f = sys.argv[2];
    start=0x0000;
    stop=0xFFFF;
    if(len(sys.argv)>3):
        start=int(sys.argv[3],16);
    if(len(sys.argv)>4):
        stop=int(sys.argv[4],16);
    
    print "Dumping code from %04x to %04x as %s." % (start,stop,f);
    h = IntelHex(None);
    i=start;
    while i<=stop:
        h[i>>1]=client.CCpeekcodebyte(i);
        if(i%0x100==0):
            print "Dumped %04x."%i;
        i+=1;
    h.write_hex_file(f);
if(sys.argv[1]=="dumpdata"):
    f = sys.argv[2];
    start=0xE000;
    stop=0xFFFF;
    if(len(sys.argv)>3):
        start=int(sys.argv[3],16);
    if(len(sys.argv)>4):
        stop=int(sys.argv[4],16);
    
    print "Dumping data from %04x to %04x as %s." % (start,stop,f);
    h = IntelHex(None);
    i=start;
    while i<=stop:
        h[i]=client.CCpeekdatabyte(i);
        if(i%0x100==0):
            print "Dumped %04x."%i;
        i+=1;
    h.write_hex_file(f);
if(sys.argv[1]=="erase"):
  print "Status: %s" % client.CCstatusstr();
  client.CCchiperase();
  print "Status: %s" %client.CCstatusstr();

# if(sys.argv[1]=="flash"):
#     f=sys.argv[2];
#     start=0;
#     stop=0xFFFF;
#     if(len(sys.argv)>3):
#         start=int(sys.argv[3],16);
#     if(len(sys.argv)>4):
#         stop=int(sys.argv[4],16);
    
#     h = IntelHex(f);
    
#     client.CCchiperase();
#     for i in h._buf.keys():
#         #print "%04x: %04x"%(i,h[i>>1]);
#         if(i>=start and i<=stop  and i&1==0):
#             client.CCwriteflash(i,h[i>>1]);
#             if(i%0x100==0):
#                 print "%04x" % i;
if(sys.argv[1]=="writedata"):
    f=sys.argv[2];
    start=0;
    stop=0xFFFF;
    if(len(sys.argv)>3):
        start=int(sys.argv[3],16);
    if(len(sys.argv)>4):
        stop=int(sys.argv[4],16);
    
    h = IntelHex(f);
    
    for i in h._buf.keys():
        if(i>=start and i<=stop):
            client.CCpokedatabyte(i,h[i]);
            if(i%0x100==0):
                print "%04x" % i;
#if(sys.argv[1]=="flashtest"):
#    client.CCflashtest();
if(sys.argv[1]=="peekdata"):
    start=0x0000;
    if(len(sys.argv)>2):
        start=int(sys.argv[2],16);
    stop=start;
    if(len(sys.argv)>3):
        stop=int(sys.argv[3],16);
    print "Peeking from %04x to %04x." % (start,stop);
    while start<=stop:
        print "%04x: %02x" % (start,client.CCpeekdatabyte(start));
        start=start+1;
if(sys.argv[1]=="peekcode"):
    start=0x0000;
    if(len(sys.argv)>2):
        start=int(sys.argv[2],16);
    stop=start;
    if(len(sys.argv)>3):
        stop=int(sys.argv[3],16);
    print "Peeking from %04x to %04x." % (start,stop);
    while start<=stop:
        print "%04x: %02x" % (start,client.CCpeekcodebyte(start));
        start=start+1;
if(sys.argv[1]=="pokedata"):
    start=0x0000;
    val=0x00;
    if(len(sys.argv)>2):
        start=int(sys.argv[2],16);
    if(len(sys.argv)>3):
        val=int(sys.argv[3],16);
    print "Poking %04x to become %02x." % (start,val);
    client.CCpokedatabyte(start,val);

client.CCstop();
