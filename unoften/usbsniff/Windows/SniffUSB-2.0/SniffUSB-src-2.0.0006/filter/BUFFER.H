#ifndef BUFFER_H
#define BUFFER_H

#define LOG_BUFFER_TAG  'fuBz'      // "zBuf"

struct Buffer
{
	char * str;

	int len;  /* current length of the string str (without the null terminator) */
	int size; /* number of bytes allocated for str. size > len */
};

void BufferInit( __in struct Buffer *b);
void BufferVprintf(
   __in struct Buffer *b,
   __in const char * format,
   __in va_list marker
   );
void BufferPrintf( __in struct Buffer *b, __in const char * format, ...);
void BufferDone(__in struct Buffer *b);

#endif
