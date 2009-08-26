/************************************************************************************
 STUNS v0.1
 (c) Andrew Frolov aka FAL, 2004
 http:\\falinc.narod.ru
 falinc@ukr.net
************************************************************************************/

//-----------------------------------------------------------------------------------
#include "FPKWareDecoder.h"
extern "C" {
#include "blast.h"
};
//-----------------------------------------------------------------------------------
bool FPKWareDecoder::Decompress(void)
{
	OutLen = MAX_OUT_SIZE;
	OutPos = 0;
	int ret = blast(inf, this, outf, this);
	if(ret == 0 && OutLen)
	{
		OutLen = OutPos;
		return true;
	}
	return false;
}
//-----------------------------------------------------------------------------------
