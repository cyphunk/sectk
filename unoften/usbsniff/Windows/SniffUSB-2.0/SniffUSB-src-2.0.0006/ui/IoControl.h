#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef __cplusplus
extern "C" {
#endif

HANDLE WINAPI UsbSnoopOpenControlChannel( void );

BOOL WINAPI UsbSnoopIsDriverStarted();

BOOL WINAPI UsbSnoopQueryLoggingState( PULONG  pLoggingState );

BOOL WINAPI UsbSnoopSetLoggingState( ULONG  LoggingState );

BOOL WINAPI UsbSnoopCloseLogFile();

BOOL WINAPI UsbSnoopDeleteLogFile();

#ifdef __cplusplus
}
#endif

