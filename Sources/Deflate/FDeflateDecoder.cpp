/************************************************************************************
 STUNS v0.1
 (c) Andrew Frolov aka FAL, 2004
 http:\\falinc.narod.ru
 falinc@ukr.net
************************************************************************************/

//-----------------------------------------------------------------------------------
#include "FDeflateDecoder.h"
extern "C" {
#include "puff.h"
};
//-----------------------------------------------------------------------------------
bool FDeflateDecoder::Decompress(void)
{
	unsigned long inLen = InLen-InPos;
	OutLen = MAX_OUT_SIZE;
	int ret = puff(OutData, &OutLen, InData+InPos, &inLen);
	if(ret == 0 && OutLen)
		return true;
	return false;
}
//-----------------------------------------------------------------------------------
