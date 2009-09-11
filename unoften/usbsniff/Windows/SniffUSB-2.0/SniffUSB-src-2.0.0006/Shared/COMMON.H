#ifndef _COMMON_H_
#define _COMMON_H_

//#if defined(_WINDOWS) || defined (_CONSOLE) || defined (_LIB)
//#include <WinIoctl.h>   // Compiling Win32 Applications Or DLL's
//#endif // _WINDOWS

// Copyright And Configuration Management ----------------------------------
//
//               User-Mode/Kernel-Mode Common Header - common.h
//
// Copyright (c) 2006-2007 Printing Communications Associates, Inc. (PCAUSA)
//                          http://www.pcausa.com
//
// End ---------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
//// Common String Definitions
//

#define USBSNOOP_NTDEVICE_NAME_A                "\\Device\\UsbSnoopFilter"
#define USBSNOOP_NTDEVICE_NAME_W                L"\\Device\\UsbSnoopFilter"

#define USBSNOOP_SERVICE_NAME_A                 "UsbSnoop"
#define USBSNOOP_SERVICE_NAME_W                 L"UsbSnoop"

#define USBSNOOP_LINKNAME_A                     "\\DosDevices\\UsbSnoopFilter"
#define USBSNOOP_LINKNAME_W                     L"\\DosDevices\\UsbSnoopFilter"

#define USBSNOOP_CREATE_FILE_STRING_A           "\\\\.\\UsbSnoopFilter"
#define USBSNOOP_CREATE_FILE_STRING_W           L"\\\\.\\UsbSnoopFilter"

#define USBSNOOP_GLOBALS_CREATE_FILE_STRING_A   "\\\\.\\Globals\\UsbSnoopFilter"
#define USBSNOOP_GLOBALS_CREATE_FILE_STRING_W   L"\\\\.\\Globals\\UsbSnoopFilter"

#define USBSNOOP_LOGFILE_NAME_A                 "UsbSnoop.log"
#define USBSNOOP_LOGFILE_NAME_W                 L"UsbSnoop.log"

#define USBSNOOP_LOGFILE_PATH_A                 "\\SystemRoot\\UsbSnoop.log"
#define USBSNOOP_LOGFILE_PATH_W                 L"\\SystemRoot\\UsbSnoop.log"

#ifdef _UNICODE
#define USBSNOOP_NTDEVICE_NAME                USBSNOOP_NTDEVICE_NAME_W
#define USBSNOOP_SERVICE_NAME                 USBSNOOP_SERVICE_NAME_W
#define USBSNOOP_LINKNAME                     USBSNOOP_LINKNAME_W
#define USBSNOOP_CREATE_FILE_STRING           USBSNOOP_CREATE_FILE_STRING_W
#define USBSNOOP_GLOBALS_CREATE_FILE_STRING   USBSNOOP_CREATE_FILE_STRING_W
#define USBSNOOP_LOGFILE_NAME                 USBSNOOP_LOGFILE_NAME_W
#define USBSNOOP_LOGFILE_PATH                 USBSNOOP_LOGFILE_PATH_W
#else
#define USBSNOOP_NTDEVICE_NAME                USBSNOOP_NTDEVICE_NAME_A
#define USBSNOOP_SERVICE_NAME                 USBSNOOP_SERVICE_NAME_A
#define USBSNOOP_LINKNAME                     USBSNOOP_LINKNAME_A
#define USBSNOOP_CREATE_FILE_STRING           USBSNOOP_CREATE_FILE_STRING_A
#define USBSNOOP_GLOBALS_CREATE_FILE_STRING   USBSNOOP_CREATE_FILE_STRING_A
#define USBSNOOP_LOGFILE_NAME                 USBSNOOP_LOGFILE_NAME_A
#define USBSNOOP_LOGFILE_PATH                 USBSNOOP_LOGFILE_PATH_A
#endif


/////////////////////////////////////////////////////////////////////////////
//// Log File Control
//

// Bitmask
typedef
enum _USBSNOOP_LOGGING_STATE
{
   LogFileOpen = 0x80000000,        // Log File Open in Driver
   LoggingEnabled = 0x00000001,     // Logging Enabled in Driver
}
   USBSNOOP_LOGGING_STATE, *PUSBSNOOP_LOGGING_STATE;

/////////////////////////////////////////////////////////////////////////////
//// IOCTL Code Definitions
//

// ATTENTION!!! Must match parameter passed to IoCreateDeviceSecure
#define FILE_DEVICE_USBSNOOP    0x8000

#define USBSNOOP_FCN_CODE_BASE  0x0800

#define _FILTERIO_CTL_CODE(_Function, _Method, _Access)  \
            CTL_CODE(FILE_DEVICE_USBSNOOP, (USBSNOOP_FCN_CODE_BASE + _Function), _Method, _Access)

#define IOCTL_FILTERIO_SET_LOGGING_STATE   \
            _FILTERIO_CTL_CODE(1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FILTERIO_QUERY_LOGGING_STATE   \
            _FILTERIO_CTL_CODE(2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FILTERIO_CLOSE_LOG_FILE   \
            _FILTERIO_CTL_CODE(3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FILTERIO_DELETE_LOG_FILE   \
            _FILTERIO_CTL_CODE(4, METHOD_BUFFERED, FILE_ANY_ACCESS)
    
#endif // _COMMON_H_

