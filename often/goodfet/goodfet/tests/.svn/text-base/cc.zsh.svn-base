#!/bin/zsh

# Testing script for the GoodFET Chipcon Client.

goodfet.cc info
goodfet.cc test

#Verify read/write of RAM.
goodfet.cc pokedata 0xffe0 0xde
goodfet.cc pokedata 0xffe1 0xad
goodfet.cc pokedata 0xffe2 0xbe
goodfet.cc pokedata 0xffe3 0xef
goodfet.cc peekdata 0xffe0 0xffe3
