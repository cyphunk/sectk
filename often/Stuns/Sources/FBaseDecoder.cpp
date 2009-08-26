/************************************************************************************
 STUNS v0.1
 (c) Andrew Frolov aka FAL, 2004
 http:\\falinc.narod.ru
 falinc@ukr.net
************************************************************************************/

//-----------------------------------------------------------------------------------
#include "FBaseDecoder.h"
//-----------------------------------------------------------------------------------
HANDLE FBaseDecoder::hInFile = 0, FBaseDecoder::hInFileM = 0;
HANDLE FBaseDecoder::hOutFile = 0, FBaseDecoder::hOutFileM = 0;
unsigned char* FBaseDecoder::InData = NULL;
unsigned char* FBaseDecoder::OutData = NULL;
unsigned long FBaseDecoder::InLen = 0, FBaseDecoder::InPos = 0;
unsigned long FBaseDecoder::OutLen = MAX_OUT_SIZE, FBaseDecoder::OutPos = 0;
char FBaseDecoder::OutDir[_MAX_PATH] = "";
//-----------------------------------------------------------------------------------
bool FBaseDecoder::Initialize(char* InFileName)
{
	if(hInFile) return true;

	// - input file mapping
	hInFile = CreateFile(
		InFileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	
	if(!hInFile) return false;

	hInFileM = CreateFileMapping(
		hInFile,
		NULL,
		PAGE_READONLY,
		0, 0, 
		NULL
	);

	if(!hInFileM)
	{
		CloseHandle(hInFile);
		return false;
	}

	InData = (unsigned char*)MapViewOfFile(
		hInFileM,
		FILE_MAP_READ,
		0, 0, 0
	);

	if(!InData)
	{
	 	CloseHandle(hInFileM);
		CloseHandle(hInFile);
		return false;
	}
	
	InLen = GetFileSize(hInFile, NULL);

	// - output file mapping
	hOutFile = CreateFile(
		TEMP_FILE_NAME,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(!hOutFile) return false;

	hOutFileM = CreateFileMapping(
		hOutFile,
		NULL,
		PAGE_READWRITE,
		0, MAX_OUT_SIZE,
		NULL
	);

	if(!hOutFileM)
	{
		CloseHandle(hOutFile);
		return false;
	}

	OutData = (unsigned char*)MapViewOfFile(
		hOutFileM, 
		FILE_MAP_ALL_ACCESS, 
		0, 0, 0
	);

	if(!OutData)
	{
	 	CloseHandle(hOutFileM);
		CloseHandle(hOutFile);
		return false;
	}

	return true;
}
//-----------------------------------------------------------------------------------
bool FBaseDecoder::Release(void)
{
	if(!hInFile) return true;

	UnmapViewOfFile(InData);
 	CloseHandle(hInFileM);
	CloseHandle(hInFile);

	UnmapViewOfFile(OutData);
	CloseHandle(hOutFileM);
	CloseHandle(hOutFile);

	unlink(TEMP_FILE_NAME);

	hInFile = hInFileM = 0;
	hOutFile = hOutFileM = 0;

	return true;
}
//-----------------------------------------------------------------------------------
bool FBaseDecoder::SaveFile(void)
{
	char OutDir[_MAX_PATH];
	if(!FBaseDecoder::OutDir[0])
		strcpy(FBaseDecoder::OutDir, "!Out");

	sprintf(OutDir, "%s%s", FBaseDecoder::OutDir, ClassName);
	mkdir(OutDir);

	char OutFileName[_MAX_PATH];
	sprintf(OutFileName, "%s\\%04x", OutDir, InPos);
	FILE* out = fopen(OutFileName, "wb");
	fwrite(OutData, OutLen, 1, out);
	fclose(out);
	printf("%s: Succeeded uncompressing %lu bytes at: %u (0x%x)\n", ClassName, OutLen, InPos, InPos);
	return true;
}
//-----------------------------------------------------------------------------------
