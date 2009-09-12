Linking script notes, by Travis Goodspeed.

msp430f1612.x is a neighborly little linking script, the purpose of which is to 
link an application for use on an MSP430F1611 or MSP430F1612.  These
chips differ in that five kilobytes at 0x2500 are RAM in the 1611,
Flash in the 1612.  All other scripts in this directory should be
similarly modified, but the GCC script seems to be used by default if not.




