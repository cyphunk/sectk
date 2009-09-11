#ifndef _PRECOMP_H_
#define _PRECOMP_H_

// Supress warning from WDK UsbIoctl.h...
#pragma warning(disable:4200)   // nonstandard extension used : zero-sized array in struct/union

#ifdef __cplusplus
extern "C" {
#endif
	
#include <wdm.h>
#include <wdmsec.h> // for IoCreateDeviceSecure
#include <usbdi.h>
#include <usbioctl.h>
#include <stdarg.h>

#include "Buffer.h"
#include "LogPrint.h"
#include "Common.h"
#include "Filter.h"

#ifdef __cplusplus
}
#endif

#endif // _PRECOMP_H_

