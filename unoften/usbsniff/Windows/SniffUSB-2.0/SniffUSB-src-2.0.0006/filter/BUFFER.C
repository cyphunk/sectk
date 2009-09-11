#include "precomp.h"
#pragma hdrstop

void BufferInit(__in struct Buffer *b)
{
	b->str  = NULL;
	b->len  = 0;
	b->size = 0;
}

/*
	BufferPrintf : like printf() but the output string goes to a buffer
	that is reallocated dynamically. So there is no compile-time limit
	to the length of the output string.

	Implementation details : to avoid reallocation for only few bytes. 'size'
	keeps the size of the allocated buffer. Its size grows by a multiple of
	PAGE_SIZE.
*/

void BufferVprintf(
   __in struct Buffer *b,
   __in const char *format,
   __in va_list marker
   )
{
	int nlen, nsize ;
	char * nstr;

	nlen = MyVsprintfLen(format,marker);
	if (nlen < 0)
	{
		/* The format string is invalid. So we print nothing!	*/
		return ;
	}

	if (b->len + nlen + 1 > b->size)
	{
		nsize = ((b->len + nlen + 1 + PAGE_SIZE -1) / PAGE_SIZE) * PAGE_SIZE;
		nstr = ExAllocatePoolWithTag(NonPagedPool, nsize, LOG_BUFFER_TAG);
		if (nstr == NULL)
		{
			myAllocationFailed ++;
			return ;
		}

		/* copy and free the previous memory block if there was one */
		if (b->str != NULL)
		{
			RtlMoveMemory(nstr, b->str,b->len+1);
			ExFreePool(b->str);
		}

		b->str = nstr;
		b->size = nsize;
	}


	MyVsprintf(b->str + b->len,format,marker);
	b->len += nlen;
}

void BufferPrintf(__in struct Buffer *b, __in const char *format, ...)
{
	va_list marker;

	va_start(marker,format);
	BufferVprintf(b,format,marker);
	va_end(marker);
}

/*
	Implementation detail: since we reinitilize the str, len and size field,
	this function can be called multiple time
*/

void BufferDone(__in struct Buffer *b)
{
	if (b->str != NULL && b->size != 0)
	{
		ExFreePool(b->str);

		b->str  = NULL;
		b->len  = 0;
		b->size = 0;
	}
}
