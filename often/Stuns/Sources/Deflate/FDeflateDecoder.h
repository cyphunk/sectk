/************************************************************************************
 STUNS v0.1
 (c) Andrew Frolov aka FAL, 2004
 http:\\falinc.narod.ru
 falinc@ukr.net
************************************************************************************/

#ifndef FDEFLATEDECODER_H
#define FDEFLATEDECODER_H

#include "..\\FBaseDecoder.h"

class FDeflateDecoder : public FBaseDecoder {
public:
	bool Decompress(void);

	FDeflateDecoder(void) : FBaseDecoder()
	{
		strcpy(ClassName, "Deflate");
	}

	~FDeflateDecoder(void)
	{
	}
};

#endif
