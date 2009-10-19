#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
extern void *wrapped_lib;
struct FT_Device;

static uint32_t* handle_list[100] = { 0 };
static int nhandles = 0;

static char *
get_handle(uint32_t *ftHandle)
{
    int i;
    static char buf[60];
    for (i = 0; i < nhandles; ++i) {
	if (handle_list[i] == ftHandle) {
	    sprintf(buf, "&ftHandle%d", i);
	    return buf;
	}
    }

    sprintf(buf, "&unknownHandle/* ftHandle=%p -> %u */", ftHandle, *ftHandle);
    return buf;
}

static void
register_handle(uint32_t *ftHandle)
{
    int i;
    for (i = 0; i < nhandles; ++i) {
	if (handle_list[i] == ftHandle)
	    return;
    }
    if (nhandles == ARRAY_SIZE(handle_list)) {
	printf("BUG! Out of handles!\n");
	return;
    }

    if (nhandles == 0)
	printf("uint32_t unknownHandle = 0; /* Just in case */\n");

    printf("uint32_t ftHandle%d; // Allocated handle %d for ftHandle ptr %p\n", nhandles, nhandles, ftHandle);

    handle_list[nhandles++] = ftHandle;
}

static unsigned long (*xFT_Close)(uint32_t * ftHandle);
unsigned long
FT_Close(uint32_t *ftHandle)
{
    unsigned long ret;
    ret = xFT_Close(ftHandle);
    printf("FT_Close(%s); // ret = %lu\n", get_handle(ftHandle), ret);
    return ret;
}

static unsigned long (*xFT_GetDeviceInfo)(uint32_t *ftHandle, struct FT_Device *lpftDevice,
				          uint32_t* lpdwID, char *serialNumber,
				          char *description, void *dummy);
unsigned long
FT_GetDeviceInfo(uint32_t *ftHandle, struct FT_Device *lpftDevice, uint32_t* id,
		 char *serialNumber, char *description, void *dummy)
{
    unsigned long ret;
    ret = xFT_GetDeviceInfo(ftHandle, lpftDevice, id, serialNumber, description, dummy);
    printf("{\n");
    printf("\tchar deviceStruct[200]; //???\n");
    printf("\tuint32_t id;\n");
    printf("\tchar serialNumber[16];\n");
    printf("\tchar description[64];\n");
    printf("\tFT_GetDeviceInfo(%s, deviceStruct, &id /* ret = %#08x */, serialNumber /* ret = %s */, description /* ret = %s */, NULL /* %p */); // ret = %lu\n",
	   get_handle(ftHandle), *id, serialNumber, description, dummy, ret);
    printf("}\n");
    return ret;
}


static unsigned long (*xFT_GetQueueStatus)(uint32_t *ftHandle, uint32_t *dwRxBytes);
unsigned long
FT_GetQueueStatus(uint32_t * ftHandle, uint32_t *dwRxBytes)
{
    unsigned long ret;
    ret = xFT_GetQueueStatus(ftHandle, dwRxBytes);
    printf("{\n");
    printf("\tuint32_t rxBytes;\n");
    printf("\tFT_GetQueueStatus(%s, &rxBytes /* =%u */); // ret = %lu\n", get_handle(ftHandle), *dwRxBytes, ret);
    printf("}\n");
    return ret;
}

static unsigned long (*xFT_ListDevices)(void *pArg1, void *pArg2, uint32_t flags);
unsigned long
FT_ListDevices(void *pArg1, void *pArg2, uint32_t flags)
{
    unsigned long ret;
    ret = xFT_ListDevices(pArg1, pArg2, flags);
    if (flags == 0x80000000 /* FT_LIST_NUMBER_ONLY */) {
	printf("{\n\tuint32_t num;\n");
	printf("\tFT_ListDevices(&num, NULL, %#08x /* flags = FT_LIST_NUMBER_ONLY, returned = %d */); // ret = %lu\n", flags, *(uint32_t*)pArg1, ret);
	printf("}\n");
    }
    else
	printf("FT_ListDevices(%p, %p, %#08x /* flags */); // ret = %lu - ????????\n", pArg1, pArg2, flags, ret);
    return ret;
}


static unsigned long (*xFT_Open)(int deviceNumber, uint32_t **ftHandle);
unsigned long
FT_Open(int deviceNumber, uint32_t **ftHandle)
{
    unsigned long ret;

    ret = xFT_Open(deviceNumber, ftHandle);

    register_handle(*ftHandle);

    printf("FT_Open(%d /* deviceNumber */, %s); // ret = %lu\n", deviceNumber, get_handle(*ftHandle), ret);

    return ret;
}

static unsigned long (*xFT_Read)(uint32_t * ftHandle, unsigned char* lpBuffer,
			         uint32_t nBufferSize, uint32_t* lpBytesReturned);

unsigned long
FT_Read(uint32_t * ftHandle, unsigned char* buf, uint32_t nBufferSize,
	uint32_t* lpBytesReturned)
{
    unsigned long ret;
    int i;

    ret = xFT_Read(ftHandle, buf, nBufferSize, lpBytesReturned);

    printf("{\n");
    printf("\tunsigned char buf[%d];\n", nBufferSize);
    printf("\tuint32_t bytesReturned;\n");
    printf("\tFT_Read(%s, buf /* %p */, %u /* buffersize */, &bytesReturned /* *%p = %u */); // ret = %lu\n",
	   get_handle(ftHandle), buf, nBufferSize, lpBytesReturned, *lpBytesReturned, ret);

    int maxRead = *lpBytesReturned;
    if (maxRead > 32)
	maxRead = 32;
#if 1
    printf("\t// char buf[%u] = {", *lpBytesReturned);
    for (i = 0; i < maxRead; ++i) {
	if ((i % 8) == 0)
	    printf("\n\t//\t");
	printf("0x%02x, ", buf[i]);

	if (i == maxRead-1)
	    printf("...");
    }
    printf("\n\t// }\n");
#endif
    printf("}\n");
    return ret;
}

static unsigned long (*xFT_SetBaudRate)(uint32_t *ftHandle, unsigned long baudRate);
unsigned long
FT_SetBaudRate(uint32_t * ftHandle, unsigned long baudRate)
{
    unsigned long ret;
    ret = xFT_SetBaudRate(ftHandle, baudRate);

    printf("FT_SetBaudReate(%s, %lu); // ret = %lu\n", get_handle(ftHandle), baudRate, ret);

    return ret;
}

static unsigned long (*xFT_SetBitMode)(uint32_t * ftHandle, unsigned char mask, unsigned char enable);
unsigned long
FT_SetBitMode(uint32_t * ftHandle, unsigned char mask, unsigned char enable)
{
    unsigned long ret;
    ret = xFT_SetBitMode(ftHandle, mask, enable);

    printf("FT_SetBitMode(%s, %#02x /* mask */, %#02x /* enable */); // ret = %lu\n", get_handle(ftHandle), mask, enable, ret);

    return ret;
}

static unsigned long (*xFT_SetVIDPID)(uint32_t, uint32_t);
unsigned long
FT_SetVIDPID(uint32_t vid, uint32_t pid)
{

    unsigned long ret;

    ret = xFT_SetVIDPID(vid, pid);

    printf("FT_SetVIDPID(%#08x, %#08x); // ret = %lu\n", vid, pid, ret);

    return ret;
}

static unsigned long (*xFT_Write)(void *handle, void *buf, uint32_t bufSize, uint32_t *bytesWritten);
unsigned long
FT_Write(void *handle, unsigned char *buf, uint32_t bufSize, uint32_t *bytesWritten)
{
    unsigned long ret;
    int i;

    ret = xFT_Write(handle, buf, bufSize, bytesWritten);

    printf("{\n");
    printf("\tchar buf[%u] = {", *bytesWritten);
    for (i = 0; i < *bytesWritten; ++i) {
	if ((i % 32) == 0)
	    printf("\n\t\t");
	printf("0x%02x, ", buf[i]);
    }
    printf("\n\t};\n");

    printf("\tuint32_t bytesWritten;\n");
    printf("\tFT_Write(%s, buf /* %p */, %u, &bytesWritten /* *%p = %u */); // ret = %lu\n", get_handle(handle), buf, bufSize, bytesWritten, *bytesWritten, ret);
    printf("}\n");

    return ret;
}

static struct {
    char *symbol;
    void (**function)();
} symbols[] = {
    { "FT_Close", (void*)&xFT_Close },
    { "FT_GetDeviceInfo", (void*)&xFT_GetDeviceInfo },
    { "FT_GetQueueStatus", (void*)&xFT_GetQueueStatus },
    { "FT_ListDevices", (void*)&xFT_ListDevices },
    { "FT_Open", (void*)&xFT_Open },
    { "FT_Read", (void*)&xFT_Read },
    { "FT_SetBaudRate", (void*)&xFT_SetBaudRate },
    { "FT_SetBitMode", (void*)&xFT_SetBitMode },
    { "FT_SetVIDPID", (void*)&xFT_SetVIDPID },
    { "FT_Write", (void*)&xFT_Write },
};


void
_init()
{
    size_t i;

    wrapped_lib = dlopen("/usr/local/lib/libftd2xx.so.0", RTLD_NOW | RTLD_LOCAL);
    if (wrapped_lib == 0) {
	fprintf(stderr, "Could not open wrapped library\n");
	exit(1);
    }

    for (i = 0; i < ARRAY_SIZE(symbols); ++i) {
	if (!(*symbols[i].function = dlsym(wrapped_lib, symbols[i].symbol))) {
	    fprintf(stderr, "Could not resolve symbol: %s\n", symbols[i].symbol);
	    exit(1);
	}
    }
}
