/************************************************************************************
 STUNS v0.1
 (c) Andrew Frolov aka FAL, 2004
 http:\\falinc.narod.ru
 falinc@ukr.net
************************************************************************************/

#ifndef FUCL2BDECODER_H
#define FUCL2BDECODER_H

#include "..\\FBaseDecoder.h"

class FUCL2BDecoder : public FBaseDecoder {
public:
	bool Decompress(void);

	FUCL2BDecoder(void) : FBaseDecoder()
	{
		strcpy(ClassName, "UCL2B");
	}

	~FUCL2BDecoder(void)
	{
	}
};

#endif
