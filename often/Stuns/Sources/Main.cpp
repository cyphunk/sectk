/************************************************************************************
 STUNS v0.1
 (c) Andrew Frolov aka FAL, 2004
 http:\\falinc.narod.ru
 falinc@ukr.net
************************************************************************************/

#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <io.h>
#include <assert.h>

#include "Deflate\\FDeflateDecoder.h"
#include "PKWare\\FPKWareDecoder.h"
#include "LZO\\FLZODecoder.h"
#include "UCL\\FUCL2BDecoder.h"
#include "UCL\\FUCL2DDecoder.h"

void Usage(void)
{
	printf("STUNS - STupid UNcompreSsor v0.1\n"
			 "(c) Andrew Frolov aka FAL, 2004\n"
			 "http:\\\\falinc.narod.ru\n"
			 "falinc@ukr.net\n"
			 "Usage: stuns [options] <filename>\n\n"
			 "Options are: -deflate[-] Deflate format\n"
			 "             -pkware[-]  PKWare Compression Library format\n"
			 "             -lzo[-]     LZO compression library format\n"
			 "             -ucl2b[-]   UCL nrv2b compression library format\n"
			 "             -ucl2d[-]   UCL nrv2d compression library format\n"
			 "             -Out<dir>   prefix for out directory (default '!Out')\n"
			 "Note: addind '-' after switch turn it off\n"
	);
}

struct DECODER {
	bool isActive;
	FBaseDecoder* decoder;
	DECODER()
	{
		isActive = true;
		decoder = NULL;
	}
	~DECODER()
	{
		isActive = true;
		if(decoder) delete decoder;
	}
} Decoders[64];

int main(int argc, char **argv)
{
	// - input file name
	char* InFileName = NULL;

	// - command line parsing
	for(int i = 1; i < argc; i++)
	{
		if(*argv[i] == '-')
		{
			if(strlen(argv[i]) > strlen("-Out") && memicmp(argv[i], "-Out", strlen("-Out")) == 0)
				strcpy(FBaseDecoder::OutDir, argv[i]+strlen("-Out"));
			else
			if(stricmp(argv[i], "-deflate") == 0)
				Decoders[0].isActive = true;
			else
			if(stricmp(argv[i], "-deflate-") == 0)
				Decoders[0].isActive = false;
			else
			if(stricmp(argv[i], "-pkware") == 0)
				Decoders[1].isActive = true;
			else
			if(stricmp(argv[i], "-pkware-") == 0)
				Decoders[1].isActive = false;
			else
			if(stricmp(argv[i], "-lzo") == 0)
				Decoders[2].isActive = true;
			else
			if(stricmp(argv[i], "-lzo-") == 0)
				Decoders[2].isActive = false;
			else
			if(stricmp(argv[i], "-ucl2b") == 0)
				Decoders[3].isActive = true;
			else
			if(stricmp(argv[i], "-ucl2b-") == 0)
				Decoders[3].isActive = false;
			else
			if(stricmp(argv[i], "-ucl2d") == 0)
				Decoders[4].isActive = true;
			else
			if(stricmp(argv[i], "-ucl2d-") == 0)
				Decoders[4].isActive = false;
		}
		else
			InFileName = argv[i];
	}

	if(!InFileName || access(InFileName, 0) != 0)
	{
		Usage();
		exit(-1);
	}

	// - create decompressors
	Decoders[0].decoder = new FDeflateDecoder();
	Decoders[1].decoder = new FPKWareDecoder();
	Decoders[2].decoder = new FLZODecoder();
	Decoders[3].decoder = new FUCL2BDecoder();
	Decoders[4].decoder = new FUCL2DDecoder();
	Decoders[5].decoder = NULL;

	// - initialize decompressors
	i = 0;
	while(Decoders[i].decoder)
	{
		Decoders[i].decoder->Initialize(InFileName);
		i++;
	}

	// - unpacking
	for(FBaseDecoder::InPos = 0; FBaseDecoder::InPos < FBaseDecoder::InLen; FBaseDecoder::InPos++)
	{
		int i = 0;
		while(Decoders[i].decoder)
		{
			if(Decoders[i].isActive && Decoders[i].decoder->Decompress()) 
				Decoders[i].decoder->SaveFile();
			i++;
		}
	}

	return 0;
}
