
                                      STUNS
                             STupid UNcompreSsor v0.1

 This program tries to unpack the given file by application of several algorithms
byte-by-byte. Result of work of the program is the set of files with the unpacked 
data. Many of the produced files are not correct. However, among them  there  can 
be correctly unpacked data. Correctly  unpacked  files  have  mainly  significant 
sizes that distinguishes them from dust.

 This program can be used for semi-automatic definition of a method of packing.
Also addresses on which there are unpacked blocks are shown. Unpacked  files  are
located in corresponding subdirectories.

Supported methods are:

1.	Deflate
2.	PKWare Compression Library format
3.	LZO
4.	UCL nrv2b
5.	UCL nrv2d

                                  PROGRAM USAGE

stuns [options] <filename>

Options are: -deflate[-] Deflate format
             -pkware[-]  PKWare Compression Library format
             -lzo[-]     LZO compression library format
             -ucl2b[-]   UCL nrv2b compression library format
             -ucl2d[-]   UCL nrv2d compression library format
             -Out<dir>   prefix for out directory (default '!Out')
Note: addind '-' after switch turn it off


                                 ADDITIONAL NOTES

1. This program needs not less than 256 mbytes of an empty space on a hard disk!
2. File test.dat included in the archive is a save file from the game "Painkiller".

-------------------------------
(c) Andrew Frolov aka FAL, 2004
http:\\falinc.narod.ru
falinc@ukr.net
