/************************************************************************************
 STUNS v0.1
 (c) Andrew Frolov aka FAL, 2004
 http:\\falinc.narod.ru
 falinc@ukr.net
************************************************************************************/

//-----------------------------------------------------------------------------------
#include "FUCL2BDecoder.h"
extern "C" {
#include "ucl\\ucl.h"
};
//-----------------------------------------------------------------------------------
bool FUCL2BDecoder::Decompress(void)
{
	unsigned int outLen = MAX_OUT_SIZE;
	int r = ucl_nrv2b_decompress_safe_8(InData+InPos, InLen-InPos, OutData, &outLen, NULL);
	OutLen = outLen;
	if(r == UCL_E_OK || r == UCL_E_INPUT_NOT_CONSUMED && OutLen)
		return true;
	return false;
}
//-----------------------------------------------------------------------------------
