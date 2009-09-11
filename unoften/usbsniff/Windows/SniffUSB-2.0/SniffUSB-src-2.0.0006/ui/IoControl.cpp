// SniffUSBDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "SniffUSB.h"
#include "SniffUSBDlg.h"
#include "devicemgr.h"
#include "multisz.h"
#include "Common.h"

#include <setupapi.h>
#include <winsvc.h>
#include <winioctl.h>
#include "IoControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HANDLE WINAPI
UsbSnoopOpenControlChannel( void )
{
    DWORD   DesiredAccess;
    DWORD   ShareMode;
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes = NULL;

    DWORD   CreationDistribution;
    DWORD   FlagsAndAttributes;
    HANDLE  TemplateFile;
    HANDLE  Handle;

    //
    // Use CreateFile to Open the Handle
    //
    DesiredAccess = GENERIC_READ|GENERIC_WRITE;
    ShareMode = 0;
    CreationDistribution = OPEN_EXISTING;
    FlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
    TemplateFile = NULL;

    Handle = CreateFile(
        USBSNOOP_CREATE_FILE_STRING,
        DesiredAccess,
        ShareMode,
        lpSecurityAttributes,
        CreationDistribution,
        FlagsAndAttributes,
        TemplateFile
        );

    if( Handle == INVALID_HANDLE_VALUE )
    {
        //
        // Special Handling For Accessing Device On Windows 2000 Terminal Server
        // ---------------------------------------------------------------------
        // See Microsoft KB Article 259131
        //
        Handle = CreateFile(
            USBSNOOP_GLOBALS_CREATE_FILE_STRING,
            DesiredAccess,
            ShareMode,
            lpSecurityAttributes,
            CreationDistribution,
            FlagsAndAttributes,
            TemplateFile
            );
    }

    return (Handle);
}

BOOL WINAPI
UsbSnoopIsDriverStarted()
{
    HANDLE hControl = UsbSnoopOpenControlChannel();

    if( hControl == INVALID_HANDLE_VALUE )
        return FALSE;

    CloseHandle( hControl );

    return TRUE;
}

BOOL WINAPI
UsbSnoopQueryLoggingState(
   PULONG  pLoggingState
   )
{
    HANDLE  hControl;
    BOOL    bResult;
    DWORD   BytesReturned;

    hControl = UsbSnoopOpenControlChannel();

    if( hControl == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

    //
    // Use DeviceIoControl to Call The Device
    //
    bResult = DeviceIoControl(
        hControl,
        IOCTL_FILTERIO_QUERY_LOGGING_STATE,
        NULL,
        0,
        pLoggingState,
        sizeof( ULONG ),
        &BytesReturned,
        NULL
        );

    CloseHandle( hControl );

    return( bResult );
}

BOOL WINAPI
UsbSnoopSetLoggingState(
   ULONG  LoggingState
   )
{
    HANDLE  hControl;
    BOOL    bResult;
    DWORD   BytesReturned;

    hControl = UsbSnoopOpenControlChannel();

    if( hControl == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

    //
    // Use DeviceIoControl to Call The Device
    //
    bResult = DeviceIoControl(
        hControl,
        IOCTL_FILTERIO_SET_LOGGING_STATE,
        &LoggingState,
        sizeof( ULONG ),
        NULL,
        0,
        &BytesReturned,
        NULL
        );

    CloseHandle( hControl );

    return( bResult );
}

BOOL WINAPI
UsbSnoopCloseLogFile()
{
    HANDLE  hControl;
    BOOL    bResult;
    DWORD   BytesReturned;

    hControl = UsbSnoopOpenControlChannel();

    if( hControl == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

    //
    // Use DeviceIoControl to Call The Device
    //
    bResult = DeviceIoControl(
        hControl,
        IOCTL_FILTERIO_CLOSE_LOG_FILE,
        NULL,
        0,
        NULL,
        0,
        &BytesReturned,
        NULL
        );

    CloseHandle( hControl );

    return( bResult );
}

BOOL WINAPI
UsbSnoopDeleteLogFile()
{
    HANDLE  hControl;
    BOOL    bResult;
    DWORD   BytesReturned;

    hControl = UsbSnoopOpenControlChannel();

    if( hControl == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

    //
    // Use DeviceIoControl to Call The Device
    //
    bResult = DeviceIoControl(
        hControl,
        IOCTL_FILTERIO_DELETE_LOG_FILE,
        NULL,
        0,
        NULL,
        0,
        &BytesReturned,
        NULL
        );

    CloseHandle( hControl );

    return( bResult );
}

