#include <initguid.h>
#include <dontuse.h>

// WAS       : {41966169-3FD7-4392-AFE4-E6A9D0A92C72}  - generated using guidgen.exe
// CHANGED TO: {9FFF154F-2FE5-4341-9799-BDACF40A90E6}  - generated using guidgen.exe
DEFINE_GUID (GUID_SD_FILTER_CONTROL_OBJECT,
        0x9fff154f, 0x2fe5, 0x4341, 0x97, 0x99, 0xbd, 0xac, 0xf4, 0xa, 0x90, 0xe6);

//
// GUID definition are required to be outside of header inclusion pragma to avoid
// error during precompiled headers.
//

#if !defined(_FILTER_H_INCLUDED_)
#define _FILTER_H_INCLUDED_

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

#define DRIVERNAME "UsbSnoop: "

#if DBG
#define DebugPrint(_x_) \
               DbgPrint (DRIVERNAME); \
               DbgPrint _x_;

#define TRAP() DbgBreakPoint()

#else
#define DebugPrint(_x_)
#define TRAP()
#endif


#ifndef  STATUS_CONTINUE_COMPLETION //required to build driver in Win2K and XP build environment
//
// This value should be returned from completion routines to continue
// completing the IRP upwards. Otherwise, STATUS_MORE_PROCESSING_REQUIRED
// should be returned.
//
#define STATUS_CONTINUE_COMPLETION      STATUS_SUCCESS

#endif

#define POOL_TAG   'liFT'

//
// These are the states Filter transition to upon
// receiving a specific PnP Irp. Refer to the PnP Device States
// diagram in DDK documentation for better understanding.
//

typedef enum _DEVICE_PNP_STATE {

    NotStarted = 0,         // Not started yet
    Started,                // Device has received the START_DEVICE IRP
    StopPending,            // Device has received the QUERY_STOP IRP
    Stopped,                // Device has received the STOP_DEVICE IRP
    RemovePending,          // Device has received the QUERY_REMOVE IRP
    SurpriseRemovePending,  // Device has received the SURPRISE_REMOVE IRP
    Deleted                 // Device has received the REMOVE_DEVICE IRP

} DEVICE_PNP_STATE;

#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DevicePnPState =  NotStarted;\
        (_Data_)->PreviousPnPState = NotStarted;

#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PreviousPnPState =  (_Data_)->DevicePnPState;\
        (_Data_)->DevicePnPState = (_state_);

#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DevicePnPState =   (_Data_)->PreviousPnPState;\

typedef enum
_FILTER_DEVICE_TYPE
{
    FILTER_DEVICE_TYPE_INVALID = 0,         // Invalid Type;
    FILTER_DEVICE_TYPE_FIDO,                // Device is a filter device.
    FILTER_DEVICE_TYPE_CDO,                 // Device is a control device.
}
   FILTER_DEVICE_TYPE;

//
// A common header for the device extensions of the Filter and control
// device objects
//

typedef struct _COMMON_DEVICE_DATA
{
    FILTER_DEVICE_TYPE Type;
}
   COMMON_DEVICE_DATA, *PCOMMON_DEVICE_DATA;


typedef struct _DEVICE_EXTENSION
{
    COMMON_DEVICE_DATA  CommonData;

    //
    // A back pointer to the device object.
    //
    PDEVICE_OBJECT   Self;

    //
    // The top of the stack before this filter was added.
    //
    PDEVICE_OBJECT   LowerDeviceObject;

    //
    // current PnP state of the device
    //
    DEVICE_PNP_STATE DevicePnPState;

    //
    // Remembers the previous pnp state
    //
    DEVICE_PNP_STATE PreviousPnPState;

    //
    // Removelock to track IRPs so that device can be removed and
    // the driver can be unloaded safely.
    //
    IO_REMOVE_LOCK   RemoveLock;

    LONG             RefCount;
    PIO_WORKITEM     DeleteWorkItem;   // For Final Dereference...

    PDEVICE_OBJECT   PhysicalDeviceObject;
    ULONG		      uSequenceNumber;
    
    PDRIVER_OBJECT   OriginalLowerDriverObject;
    DRIVER_OBJECT    ModifiedLowerDriverObject;
}
   DEVICE_EXTENSION, *PDEVICE_EXTENSION;

// For our IRP stack context information
#define IRP_STACK_CONTEXT_TAG  'xtCz'      // "zCtx"

typedef struct _IRP_STACK_CONTEXT
{
   PDEVICE_EXTENSION   deviceExtension;   // Of Filter Handling IRP...

   // information for the old CompletionRoutine
   PIO_COMPLETION_ROUTINE CompletionRoutine;
   PVOID Context;
   UCHAR Control;

   // what we need : the saved PURB & its number
   URB   OriginalUrb;
   PURB  pUrb;
   ULONG uSequenceNumber;

   // the stack where we should restore information
   PIO_STACK_LOCATION Stack;
}
   IRP_STACK_CONTEXT, * PIRP_STACK_CONTEXT;


#ifdef __cplusplus
extern "C" {
#endif

DRIVER_INITIALIZE DriverEntry;

DRIVER_ADD_DEVICE FilterAddDevice;

//
// Dispatch Routines for This Device (FIDO)
//
DRIVER_DISPATCH FilterDispatchPnp;
DRIVER_DISPATCH FilterDispatchPower;
DRIVER_DISPATCH FilterDispatchAny;
DRIVER_UNLOAD FilterUnload;

IO_COMPLETION_ROUTINE FilterDeviceUsageNotificationCompletionRoutine;

IO_COMPLETION_ROUTINE FilterStartCompletionRoutine;

//
// Hooked Routines in Next Lower Device (FDO) Dispatch Table
//
DRIVER_DISPATCH FdoHookDispatchInternalIoctl;
DRIVER_DISPATCH FdoHookDispatchPnp;

NTSTATUS
FilterStartCompletionRoutine(
    __in PDEVICE_OBJECT   DeviceObject,
    __in PIRP             Irp,
    __in PVOID            Context
    );

#ifdef __cplusplus
}
#endif


// Implemented in Strings.c
PCHAR IrpMajorFunctionString( UCHAR MajorFunction );
PCHAR PnPMinorFunctionString( UCHAR MinorFunction );

#ifdef IOCTL_INTERFACE

typedef struct _CONTROL_DEVICE_EXTENSION {

    COMMON_DEVICE_DATA CommonData;

    ULONG   Deleted; // False if the deviceobject is valid, TRUE if it's deleted

    PVOID   ControlData; // Store your control data here

} CONTROL_DEVICE_EXTENSION, *PCONTROL_DEVICE_EXTENSION;

#ifdef __cplusplus
extern "C" {
#endif

NTSTATUS
FilterCreateControlObject(
    __in PDEVICE_OBJECT    DeviceObject
);

VOID
FilterDeleteControlObject(
    );

DRIVER_DISPATCH FilterDispatchIo;

#ifdef __cplusplus
}
#endif

#endif // IOCTL_INTERFACE

#endif // _FILTER_H_INCLUDED_


