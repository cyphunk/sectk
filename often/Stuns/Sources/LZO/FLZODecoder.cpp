/************************************************************************************
 STUNS v0.1
 (c) Andrew Frolov aka FAL, 2004
 http:\\falinc.narod.ru
 falinc@ukr.net
************************************************************************************/

//-----------------------------------------------------------------------------------
#include "FLZODecoder.h"
extern "C" {
#include "minilzo.h"
};
//-----------------------------------------------------------------------------------
bool FLZODecoder::Decompress(void)
{
	unsigned int outLen = MAX_OUT_SIZE;
	int r = lzo1x_decompress_safe(InData+InPos, InLen-InPos, OutData, &outLen, NULL);
	OutLen = outLen;
	if(r == LZO_E_OK || r == LZO_E_INPUT_NOT_CONSUMED && OutLen)
		return true;
	return false;
}
//-----------------------------------------------------------------------------------
