/************************************************************************************
 STUNS v0.1
 (c) Andrew Frolov aka FAL, 2004
 http:\\falinc.narod.ru
 falinc@ukr.net
************************************************************************************/

#ifndef FLZODECODER_H
#define FLZODECODER_H

#include "..\\FBaseDecoder.h"

class FLZODecoder : public FBaseDecoder {
public:
	bool Decompress(void);

	FLZODecoder(void) : FBaseDecoder()
	{
		strcpy(ClassName, "LZO");
	}

	~FLZODecoder(void)
	{
	}
};

#endif
