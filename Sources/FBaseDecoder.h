/************************************************************************************
 STUNS v0.1
 (c) Andrew Frolov aka FAL, 2004
 http:\\falinc.narod.ru
 falinc@ukr.net
************************************************************************************/

#ifndef FBASEDECODER_H
#define FBASEDECODER_H

#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <assert.h>

class FBaseDecoder {
	static HANDLE hInFile, hInFileM;
	static HANDLE hOutFile, hOutFileM;
public:
	#define TEMP_FILE_NAME "$$$tmp$$$.$$$"
	enum { 
		MAX_OUT_SIZE = 256*1024*1024
	};

	static char OutDir[_MAX_PATH];
	char ClassName[64];

	static unsigned char* InData;
	static unsigned char* OutData;
	static unsigned long InLen, InPos;
	static unsigned long OutLen, OutPos;

	bool Initialize(char* InFileName);
	bool Release(void);
	virtual bool Decompress(void) = 0;
	bool SaveFile(void);

	FBaseDecoder(void)
	{
	}

	~FBaseDecoder(void)
	{
		Release();
	}
};

#endif
