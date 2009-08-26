/************************************************************************************
 STUNS v0.1
 (c) Andrew Frolov aka FAL, 2004
 http:\\falinc.narod.ru
 falinc@ukr.net
************************************************************************************/

#ifndef FUCL2DDECODER_H
#define FUCL2DDECODER_H

#include "..\\FBaseDecoder.h"

class FUCL2DDecoder : public FBaseDecoder {
public:
	bool Decompress(void);

	FUCL2DDecoder(void) : FBaseDecoder()
	{
		strcpy(ClassName, "UCL2D");
	}

	~FUCL2DDecoder(void)
	{
	}
};

#endif
