#ifndef _LOGGING_H_
#define _LOGGING_H_

void DumpDriverObject(__in PDRIVER_OBJECT p);
void DumpDeviceObject(__in PDEVICE_OBJECT p);
void DumpURB(__in struct Buffer *b, __in PURB pUrb, __in BOOLEAN bReturnedFromHCD);

#endif // _LOGGING_H_
