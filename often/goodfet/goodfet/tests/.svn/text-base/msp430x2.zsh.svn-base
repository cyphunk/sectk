#!/bin/zsh

# Testing script for the GoodFET Chipcon Client.

goodfet.msp430 info
#goodfet.msp430 test

#Verify read/write of RAM.
goodfet.msp430 poke 0x3000 0xdead
goodfet.msp430 poke 0x3002 0xbeef
goodfet.msp430 peek 0x3000 0x3002
