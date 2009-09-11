#ifndef _LOGPRINT_H_
#define _LOGPRINT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* maximum value our semaphore can reach without crashing the machine */
#define MAX_SEMAPHORE 10000

int  MyVsprintfLen(        const char * format,va_list marker);
void MyVsprintf(char *dst, const char * format,va_list marker);
void MySprintf (char *dst, const char * format, ...);

/* some global variable that we share */
extern int myAllocationFailed;

/* following functions are for general use */

extern BOOLEAN g_bLoggingEnabled;
extern HANDLE  g_hLogFile;

void LogInit();
void LogDone();
NTSTATUS LogCloseFile();
NTSTATUS LogDeleteFile();
void LogPrintf(const char * format, ...);
void LogBuffer(struct Buffer * b);

#ifdef __cplusplus
};
#endif

#endif // _LOGPRINT_H_
