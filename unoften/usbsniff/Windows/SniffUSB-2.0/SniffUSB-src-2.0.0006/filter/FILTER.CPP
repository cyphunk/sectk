/////////////////////////////////////////////////////////////////////////////
//// INCLUDE FILES

#include "precomp.h"
#pragma hdrstop

#include <usbbusif.h>
#include <hubbusif.h>

#include "Logging.h"
#include "Common.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, FilterAddDevice)
#pragma alloc_text (PAGE, FilterDispatchPnp)
#pragma alloc_text (PAGE, FilterUnload)
#endif

//
// Define IOCTL_INTERFACE in your sources file if you want your application
// to have private interaction with the filter driver. Read KB Q262305
// for more information.
//

#ifdef IOCTL_INTERFACE

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, FilterCreateControlObject)
#pragma alloc_text (PAGE, FilterDeleteControlObject)
#pragma alloc_text (PAGE, FilterDispatchIo)
#endif

FAST_MUTEX ControlMutex;
ULONG InstanceCount = 0;
PDEVICE_OBJECT ControlDeviceObject;

#endif

NPAGED_LOOKASIDE_LIST   g_ContextLookasideList;

NTSTATUS
DriverEntry(
    __in PDRIVER_OBJECT  DriverObject,
    __in PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    Installable driver initialization entry point.
    This entry point is called directly by the I/O system.

Arguments:

    DriverObject - pointer to the driver object

    RegistryPath - pointer to a unicode string representing the path,
                   to driver-specific key in the registry.

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise.

--*/
{
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               ulIndex;
    PDRIVER_DISPATCH  * dispatch;

    UNREFERENCED_PARAMETER (RegistryPath);

    DebugPrint (("Entered the Driver Entry\n"));

    //
    // Allocate Resources for UsbSnoop Logging Mechanism
    //
    LogInit();

    //
    // Allocate IRP stack filtering context lookaside list
    //
    ExInitializeNPagedLookasideList(
       &g_ContextLookasideList,
       NULL,   // Allocate Function
       NULL,   // Free Function
       0,      // Flags. Reserved and must be zero
       sizeof(IRP_STACK_CONTEXT),
       IRP_STACK_CONTEXT_TAG,
       0       // Depth
       );

    //
    // Create dispatch points
    //
    for (ulIndex = 0, dispatch = DriverObject->MajorFunction;
        ulIndex <= IRP_MJ_MAXIMUM_FUNCTION;
        ulIndex++, dispatch++
        )
    {
        *dispatch = FilterDispatchAny;
    }

    DriverObject->MajorFunction[IRP_MJ_PNP]            = FilterDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = FilterDispatchPower;
    DriverObject->DriverExtension->AddDevice           = FilterAddDevice;
    DriverObject->DriverUnload                         = FilterUnload;

#ifdef IOCTL_INTERFACE
    //
    // Set the following dispatch points as we will be doing
    // something useful to these requests instead of just
    // passing them down. 
    // 
    DriverObject->MajorFunction[IRP_MJ_CREATE]     = 
        DriverObject->MajorFunction[IRP_MJ_CLOSE]      = 
        DriverObject->MajorFunction[IRP_MJ_CLEANUP]    = 
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = 
        FilterDispatchIo;

    //
    // Mutex is to synchronize multiple threads creating & deleting 
    // control deviceobjects. 
    //
    ExInitializeFastMutex(&ControlMutex);

#endif

    return status;
}

//
// Add Reference Count on Device 
// -----------------------------
// In particular, insure that DEVICE_EXTENSION memory continues to 
// exist until final dereference.
//
VOID
FilterRefDevice(
   __in PDEVICE_EXTENSION deviceExtension
   )
{
   InterlockedIncrement( &deviceExtension->RefCount );
}

IO_WORKITEM_ROUTINE FilterDeleteDeviceWorkItem;

// Called at IRQL == PASSIVE_LEVEL
VOID
FilterDeleteDeviceWorkItem(
   __in PDEVICE_OBJECT DeviceObject,
   __in PVOID Context
   )
{
   PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION )Context;

   DebugPrint(( "FilterDeleteDeviceWorkItem: %p\n", deviceExtension->Self ));

   ASSERT( deviceExtension );
   ASSERT( DeviceObject == deviceExtension->Self );

   PIO_WORKITEM DeleteWorkItem = deviceExtension->DeleteWorkItem;

   IoDetachDevice(deviceExtension->LowerDeviceObject);
   IoDeleteDevice(deviceExtension->Self);

   IoFreeWorkItem( DeleteWorkItem );
}

//
// Remove Reference Count on Device 
// --------------------------------
// DEVICE_EXTENSION memory will continue to exist until reference count
// is decremented to zero(0).
//
VOID
FilterDerefDevice(
   __in PDEVICE_EXTENSION deviceExtension
   )
{
   if( !deviceExtension )
      return;

   if( InterlockedDecrement( &deviceExtension->RefCount) == 0 )
   {
      DebugPrint(( "FilterDerefDevice: %p\n", deviceExtension->Self ));

      //
      // Use Work Item to Detach and Delete Device
      // -----------------------------------------
      // Must insure that IoDetachDevice is called at IRQL == PASSIVE_LEVEL.
      //
      deviceExtension->DeleteWorkItem = IoAllocateWorkItem( deviceExtension->Self );

      IoQueueWorkItem(
         deviceExtension->DeleteWorkItem,
         FilterDeleteDeviceWorkItem,
         DelayedWorkQueue,
         deviceExtension
         );
   }
}

NTSTATUS
FilterAddDevice(
    __in PDRIVER_OBJECT DriverObject,
    __in PDEVICE_OBJECT PhysicalDeviceObject
    )
/*++

Routine Description:

    The Plug & Play subsystem is handing us a brand new PDO, for which we
    (by means of INF registration) have been asked to provide a driver.

    We need to determine if we need to be in the driver stack for the device.
    Create a function device object to attach to the stack
    Initialize that device object
    Return status success.

    Remember: We can NOT actually send ANY non pnp IRPS to the given driver
    stack, UNTIL we have received an IRP_MN_START_DEVICE.

Arguments:

    DeviceObject - pointer to a device object.

    PhysicalDeviceObject -  pointer to a device object created by the
                            underlying bus driver.

Return Value:

    NT status code.

--*/
{
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_OBJECT          deviceObject = NULL;
    PDEVICE_EXTENSION       deviceExtension;
    ULONG                   deviceType = FILE_DEVICE_UNKNOWN;

    PAGED_CODE ();

    LogPrintf("UsbSnoop - FilterAddDevice(%p) : DriverObject %p, pdo %p\n",
        FilterAddDevice,DriverObject, PhysicalDeviceObject);

    //    DumpDriverObject(DriverObject);
    //    DumpDeviceObject(PhysicalDeviceObject);

    //
    // IoIsWdmVersionAvailable(1, 0x20) returns TRUE on os after Windows 2000.
    //
    if (!IoIsWdmVersionAvailable(1, 0x20))
    {
        //
        // Win2K system bugchecks if the filter attached to a storage device
        // doesn't specify the same DeviceType as the device it's attaching
        // to. This bugcheck happens in the filesystem when you disable
        // the devicestack whose top level deviceobject doesn't have a VPB.
        // To workaround we will get the toplevel object's DeviceType and
        // specify that in IoCreateDevice.
        //
        deviceObject = IoGetAttachedDeviceReference(PhysicalDeviceObject);
        deviceType = deviceObject->DeviceType;
        ObDereferenceObject(deviceObject);
    }

    //
    // Create a filter device object
    // -----------------------------
    // In WDM terms the newly created filter's deviceObject is the FIDO.
    //
    status = IoCreateDevice (DriverObject,
        sizeof (DEVICE_EXTENSION),
        NULL,  // No Name
        deviceType,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &deviceObject);


    if (!NT_SUCCESS (status))
    {
        //
        // Returning failure here prevents the entire stack from functioning,
        // but most likely the rest of the stack will not be able to create
        // device objects either, so it is still OK.
        //
        return status;
    }

    DebugPrint (("AddDevice PDO (0x%p) FDO (0x%p)\n",
        PhysicalDeviceObject, deviceObject));

    deviceExtension = (PDEVICE_EXTENSION )deviceObject->DeviceExtension;

    deviceExtension->CommonData.Type = FILTER_DEVICE_TYPE_FIDO;

    deviceExtension->RefCount = 1;

    //
    // The IoAttachDeviceToDeviceStack routine attaches the caller's
    // device object to the highest device object in the chain and
    // returns a pointer to the previously highest device object. 
    //
    // This routine sets the AlignmentRequirement in SourceDevice to
    // the value in the next-lower device object and sets the StackSize
    // to the value in the next-lower-object plus one.
    //
    // In WDM terms the LowerDeviceObject returned from this call is the FDO.
    //
    deviceExtension->LowerDeviceObject = IoAttachDeviceToDeviceStack (
        deviceObject,
        PhysicalDeviceObject);

    //
    // Failure for attachment is an indication of a broken plug & play system.
    //
    if (NULL == deviceExtension->LowerDeviceObject)
    {
        IoDeleteDevice(deviceObject);
        return STATUS_UNSUCCESSFUL;
    }

    deviceObject->Flags |= deviceExtension->LowerDeviceObject->Flags &
        (DO_BUFFERED_IO | DO_DIRECT_IO |
        DO_POWER_PAGABLE );

    deviceObject->DeviceType = deviceExtension->LowerDeviceObject->DeviceType;

    deviceObject->Characteristics =
        deviceExtension->LowerDeviceObject->Characteristics;

    deviceExtension->Self = deviceObject;
    deviceExtension->PhysicalDeviceObject = PhysicalDeviceObject;

#ifdef DBG
    DumpDriverObject(DriverObject);
    DumpDeviceObject(deviceExtension->LowerDeviceObject);   // Fdo
    DumpDeviceObject(deviceObject); // fido
    DumpDeviceObject(PhysicalDeviceObject);
#endif // DBG

    {
        PWCHAR deviceName = NULL;
        ULONG nameLength;

        status = IoGetDeviceProperty(PhysicalDeviceObject,
            DevicePropertyPhysicalDeviceObjectName, 0,
            NULL, &nameLength);

        if ((nameLength != 0) && (status == STATUS_BUFFER_TOO_SMALL))
        {
            deviceName = (PWCHAR )ExAllocatePoolWithTag(NonPagedPool, nameLength, POOL_TAG);

            if (deviceName)
            {
                IoGetDeviceProperty(
                    PhysicalDeviceObject,
                    DevicePropertyPhysicalDeviceObjectName, // Example: "\Device\USBPDO-9". NOT Persistent...
                    nameLength,
                    deviceName,
                    &nameLength
                    );

                DebugPrint(("AddDevice: %p to %p->%p (%ws) \n",
                    deviceObject,
                    deviceExtension->LowerDeviceObject,
                    PhysicalDeviceObject,
                    deviceName));

                ExFreePool(deviceName);
            }
        }
    }

    //
    // Hijack the Next Lower Device (FDO) Dispatch Table
    //

    DebugPrint(("Installing FDO Dispatch Table Hook\n"));

    // First Save Pointer To Original Next Lower Device's Driver Object
    deviceExtension->OriginalLowerDriverObject =
        (deviceExtension->LowerDeviceObject)->DriverObject;

    // Make A Copy Of The Next Lower Device's Driver Object
    RtlCopyBytes(
        &deviceExtension->ModifiedLowerDriverObject,
        deviceExtension->OriginalLowerDriverObject,
        sizeof(DRIVER_OBJECT)
        );

    // Make some changes to this copy
    deviceExtension->ModifiedLowerDriverObject.MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = FdoHookDispatchInternalIoctl;
    deviceExtension->ModifiedLowerDriverObject.MajorFunction[IRP_MJ_PNP] = FdoHookDispatchPnp;

    // here is the trick : we save the original DriverObject
    // and next, it points to our modified copy
    InterlockedExchangePointer(
        (PVOID * )&( (deviceExtension->LowerDeviceObject)->DriverObject ),
        &deviceExtension->ModifiedLowerDriverObject
        );

    //
    // Let us use remove lock to keep count of IRPs so that we don't 
    // detach and delete our deviceobject until all pending I/Os in our
    // devstack are completed. Remlock is required to protect us from
    // various race conditions where our driver can get unloaded while we
    // are still running dispatch or completion code.
    //
    IoInitializeRemoveLock (&deviceExtension->RemoveLock , 
        POOL_TAG,
        1, // MaxLockedMinutes 
        100);  // HighWatermark, this parameter is 
    // used only on checked build. Specifies 
    // the maximum number of outstanding 
    // acquisitions allowed on the lock


    //
    // Set the initial state of the Filter DO
    //
    INITIALIZE_PNP_STATE(deviceExtension);

    DebugPrint(("AddDevice: %p to %p->%p \n", deviceObject,
        deviceExtension->LowerDeviceObject,
        PhysicalDeviceObject));

    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;
}


NTSTATUS
FilterPassthru (
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
/*++

Routine Description:

    This routine simply sends the IRP down without any modification.

    As we have NO idea which function we are happily passing on, we can make
    NO assumptions about whether or not it will be called at raised IRQL.
    For this reason, this function must be in put into non-paged pool
    (aka the default location).

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

      NT status code

--*/
{
    PDEVICE_EXTENSION           deviceExtension;
    NTSTATUS    status;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS (status))
    {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    IoSkipCurrentIrpStackLocation (Irp);
    status = IoCallDriver (deviceExtension->LowerDeviceObject, Irp);
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp); 
    return status;
}


NTSTATUS
FilterDispatchAny (
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
/*++

Routine Description:

    The default dispatch routine.  If this driver does not recognize the
    IRP, then it should send it down, unmodified.
    If the device holds iris, this IRP must be queued in the device extension
    No completion routine is required.

    For demonstrative purposes only, we will pass all the (non-PnP) Irps down
    on the stack (as we are a filter driver). A real driver might choose to
    service some of these Irps.

    As we have NO idea which function we are happily passing on, we can make
    NO assumptions about whether or not it will be called at raised IRQL.
    For this reason, this function must be in put into non-paged pool
    (aka the default location).

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

      NT status code

--*/
{
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);

    if (irpStack->MajorFunction == IRP_MJ_PNP)
    {
        const char * MinorFunctionName = PnPMinorFunctionString(irpStack->MinorFunction);

        if (MinorFunctionName != NULL)
            LogPrintf("UsbSnoop - FilterDispatchAny(%p) : IRP_MJ_PNP (%s)\n",
            FilterDispatchAny,MinorFunctionName);
        else
            LogPrintf("UsbSnoop - FilterDispatchAny(%p) : IRP_MJ_PNP (0x%x)\n",
            FilterDispatchAny,irpStack->MinorFunction);
    }
    else
    {
       LogPrintf("UsbSnoop - FilterDispatchAny(%p) : %s\n",
          FilterDispatchAny,
          IrpMajorFunctionString( irpStack->MajorFunction )
          );
    }

    // FilterPassthru simply sends the unmodified IRP down...
    return FilterPassthru( DeviceObject, Irp );
}

NTSTATUS
FilterDispatchPnp (
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
/*++

Routine Description:

    The plug and play dispatch routines.

    Most of these the driver will completely ignore.
    In all cases it must pass on the IRP to the lower driver.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

      NT status code

--*/
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status;
    KEVENT              event;

    PAGED_CODE();

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    DebugPrint(("FilterDO %s IRP:0x%p \n",
        PnPMinorFunctionString(irpStack->MinorFunction), Irp));

    const char * MinorFunctionName = PnPMinorFunctionString(irpStack->MinorFunction);

    if (MinorFunctionName != NULL)
        LogPrintf("UsbSnoop - FilterDispatchPnp(%p) : IRP_MJ_PNP (%s)\n",
            FilterDispatchPnp,MinorFunctionName);
    else
        LogPrintf("UsbSnoop - FilterDispatchPnp(%p) : IRP_MJ_PNP (0x%x)\n",
            FilterDispatchPnp,irpStack->MinorFunction);

    // Provide Additional Information for IRP_MN_QUERY_INTERFACE
    if( irpStack->MinorFunction == IRP_MN_QUERY_INTERFACE )
    {
        if( IsEqualGUIDAligned(
                *irpStack->Parameters.QueryInterface.InterfaceType,
                USB_BUS_INTERFACE_HUB_GUID
                )
            )
        {
            LogPrintf("UsbSnoop - FilterDispatchPnp: Query for Bus interface for USB Hub drivers.\n");
            LogPrintf("\tInterfaceType: USB_BUS_INTERFACE_HUB_GUID\n");
        }
        else if( IsEqualGUIDAligned(
                *irpStack->Parameters.QueryInterface.InterfaceType,
                USB_BUS_INTERFACE_USBDI_GUID
                )
            )
        {
            LogPrintf("UsbSnoop - FilterDispatchPnp: Query for Bus interface for USB Function Drivers.\n");
            LogPrintf("\tInterfaceType: USB_BUS_INTERFACE_USBDI_GUID\n");
        }
        else
        {
            UNICODE_STRING GuidString;
            NTSTATUS ConvertStatus;

            ConvertStatus = RtlStringFromGUID(
                *irpStack->Parameters.QueryInterface.InterfaceType,
                &GuidString
                );

            if( ConvertStatus == STATUS_SUCCESS )
            {
                LogPrintf("UsbSnoop - FilterDispatchPnp: Query for InterfaceType %wZ.\n",&GuidString);

                RtlFreeUnicodeString( &GuidString );
            }
            else
            {
                LogPrintf("UsbSnoop - FilterDispatchPnp: Query for InterfaceType *** UNKNOWN ***.\n");
            }
        }
    }

    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);

    if (!NT_SUCCESS (status))
    {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    switch (irpStack->MinorFunction)
    {
    case IRP_MN_START_DEVICE:

        //
        // The device is starting.
        // We cannot touch the device (send it any non pnp irps) until a
        // start device has been passed down to the lower drivers.
        //
        KeInitializeEvent(&event, NotificationEvent, FALSE);
        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp,
            (PIO_COMPLETION_ROUTINE) FilterStartCompletionRoutine,
            &event,
            TRUE,
            TRUE,
            TRUE);

        status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);

        //
        // Wait for lower drivers to be done with the Irp. Important thing to
        // note here is when you allocate memory for an event in the stack  
        // you must do a KernelMode wait instead of UserMode to prevent 
        // the stack from getting paged out.
        //
        if (status == STATUS_PENDING)
        {
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);          
            status = Irp->IoStatus.Status;
        }

        if (NT_SUCCESS (status))
        {
            //
            // As we are successfully now back, we will
            // first set our state to Started.
            //
            SET_NEW_PNP_STATE(deviceExtension, Started);

            //
            // On the way up inherit FILE_REMOVABLE_MEDIA during Start.
            // This characteristic is available only after the driver stack is started!.
            //
            if (deviceExtension->LowerDeviceObject->Characteristics & FILE_REMOVABLE_MEDIA)
            {
                DeviceObject->Characteristics |= FILE_REMOVABLE_MEDIA;
            }

#ifdef IOCTL_INTERFACE
            //
            // If the PreviousPnPState is stopped then we are being stopped temporarily
            // and restarted for resource rebalance. 
            //
            if (Stopped != deviceExtension->PreviousPnPState)
            {
                //
                // Device is started for the first time.
                //
                FilterCreateControlObject(DeviceObject);
            }
#endif   
        }

        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp); 
        return status;

    case IRP_MN_REMOVE_DEVICE:

        //
        // Wait for all outstanding requests to complete
        //
        DebugPrint(("Waiting for outstanding requests\n"));
        IoReleaseRemoveLockAndWait(&deviceExtension->RemoveLock, Irp);

        // Restore the driver pointer in the FDO
        DebugPrint(("Removing FDO Dispatch Table Hook (1)\n"));

        InterlockedExchangePointer(
            (PVOID * )&( (deviceExtension->LowerDeviceObject)->DriverObject ),
            deviceExtension->OriginalLowerDriverObject
            );

        IoSkipCurrentIrpStackLocation(Irp);

        status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);

        SET_NEW_PNP_STATE(deviceExtension, Deleted);

#ifdef IOCTL_INTERFACE
        FilterDeleteControlObject();
#endif 

        // Should Be The Final Dereference...
        FilterDerefDevice( deviceExtension );

        return status;


    case IRP_MN_QUERY_STOP_DEVICE:
        SET_NEW_PNP_STATE(deviceExtension, StopPending);
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        //
        // Check to see whether you have received cancel-stop
        // without first receiving a query-stop. This could happen if someone
        // above us fails a query-stop and passes down the subsequent
        // cancel-stop.
        //

        if (StopPending == deviceExtension->DevicePnPState)
        {
            //
            // We did receive a query-stop, so restore.
            //
            RESTORE_PREVIOUS_PNP_STATE(deviceExtension);
        }
        status = STATUS_SUCCESS; // We must not fail this IRP.
        break;

    case IRP_MN_STOP_DEVICE:
        SET_NEW_PNP_STATE(deviceExtension, Stopped);
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:

        SET_NEW_PNP_STATE(deviceExtension, RemovePending);
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_SURPRISE_REMOVAL:

        SET_NEW_PNP_STATE(deviceExtension, SurpriseRemovePending);
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        //
        // Check to see whether you have received cancel-remove
        // without first receiving a query-remove. This could happen if
        // someone above us fails a query-remove and passes down the
        // subsequent cancel-remove.
        //
        if (RemovePending == deviceExtension->DevicePnPState)
        {
            //
            // We did receive a query-remove, so restore.
            //
            RESTORE_PREVIOUS_PNP_STATE(deviceExtension);
        }

        status = STATUS_SUCCESS; // We must not fail this IRP.
        break;

    case IRP_MN_DEVICE_USAGE_NOTIFICATION:

        //
        // On the way down, pagable might become set. Mimic the driver
        // above us. If no one is above us, just set pagable.
        //
        if ((DeviceObject->AttachedDevice == NULL) ||
            (DeviceObject->AttachedDevice->Flags & DO_POWER_PAGABLE))
        {
            DeviceObject->Flags |= DO_POWER_PAGABLE;
        }

        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(
            Irp,
            FilterDeviceUsageNotificationCompletionRoutine,
            NULL,
            TRUE,
            TRUE,
            TRUE
            );

        return IoCallDriver(deviceExtension->LowerDeviceObject, Irp);

    default:
        //
        // If you don't handle any IRP you must leave the
        // status as is.
        //
        status = Irp->IoStatus.Status;

        break;
    }

    //
    // Pass the IRP down and forget it.
    //
    Irp->IoStatus.Status = status;
    IoSkipCurrentIrpStackLocation (Irp);
    status = IoCallDriver (deviceExtension->LowerDeviceObject, Irp);
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp); 
    return status;
}

NTSTATUS
FilterStartCompletionRoutine(
    __in PDEVICE_OBJECT   DeviceObject,
    __in PIRP             Irp,
    __in PVOID            Context
    )
/*++
Routine Description:
    A completion routine for use when calling the lower device objects to
    which our filter deviceobject is attached.

Arguments:

    DeviceObject - Pointer to deviceobject
    Irp          - Pointer to a PnP Irp.
    Context      - NULL
Return Value:

    NT Status is returned.

--*/

{
    PKEVENT             event = (PKEVENT)Context;

    UNREFERENCED_PARAMETER (DeviceObject);

    //
    // If the lower driver didn't return STATUS_PENDING, we don't need to 
    // set the event because we won't be waiting on it. 
    // This optimization avoids grabbing the dispatcher lock, and improves perf.
    //
    if (Irp->PendingReturned == TRUE)
    {
        KeSetEvent (event, IO_NO_INCREMENT, FALSE);
    }

    //
    // The dispatch routine will have to call IoCompleteRequest
    //

    return STATUS_MORE_PROCESSING_REQUIRED;

}

NTSTATUS
FilterDeviceUsageNotificationCompletionRoutine(
    __in PDEVICE_OBJECT   DeviceObject,
    __in PIRP             Irp,
    __in PVOID            Context
    )
/*++
Routine Description:
    A completion routine for use when calling the lower device objects to
    which our filter deviceobject is attached.

Arguments:

    DeviceObject - Pointer to deviceobject
    Irp          - Pointer to a PnP Irp.
    Context      - NULL
Return Value:

    NT Status is returned.

--*/

{
    PDEVICE_EXTENSION       deviceExtension;

    UNREFERENCED_PARAMETER(Context);

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if (Irp->PendingReturned)
    {
        IoMarkIrpPending(Irp);
    }

    //
    // On the way up, pagable might become clear. Mimic the driver below us.
    //
    if (!(deviceExtension->LowerDeviceObject->Flags & DO_POWER_PAGABLE))
    {
        DeviceObject->Flags &= ~DO_POWER_PAGABLE;
    }

    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp); 

    return STATUS_CONTINUE_COMPLETION;

}

NTSTATUS
FilterDispatchPower(
    __in PDEVICE_OBJECT    DeviceObject,
    __in PIRP              Irp
    )
/*++

Routine Description:

    This routine is the dispatch routine for power irps.

Arguments:

    DeviceObject - Pointer to the device object.

    Irp - Pointer to the request packet.

Return Value:

    NT Status code
--*/
{
    PDEVICE_EXTENSION   deviceExtension;
    NTSTATUS    status;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) // may be device is being removed.
    {
        Irp->IoStatus.Status = status;
        PoStartNextPowerIrp(Irp);
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    status = PoCallDriver(deviceExtension->LowerDeviceObject, Irp);
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp); 
    return status;
}


VOID
FilterUnload(
    __in PDRIVER_OBJECT DriverObject
    )
/*++

Routine Description:

    Free all the allocated resources in DriverEntry, etc.

Arguments:

    DriverObject - pointer to a driver object.

Return Value:

    VOID.

--*/
{
    PAGED_CODE ();

    //
    // The device object(s) should be NULL now
    // (since we unload, all the devices objects associated with this
    // driver must be deleted.
    //
    ASSERT(DriverObject->DeviceObject == NULL);

    //
    // We should not be unloaded until all the devices we control
    // have been removed from our queue.
    //
    DebugPrint (("==> FilterUnload\n"));

    //
    // Free Resources for UsbSnoop Logging Mechanism
    //
    LogDone();

    //
    // Free IRP stack filtering context lookaside list
    //
    ExDeleteNPagedLookasideList( &g_ContextLookasideList );

    DebugPrint (("<== FilterUnload\n"));

    return;
}

/////////////////////////////////////////////////////////////////////////////
//                       Private IOCTL Interface Routines                  //
/////////////////////////////////////////////////////////////////////////////

#ifdef IOCTL_INTERFACE

NTSTATUS
FilterCreateControlObject(
    __in PDEVICE_OBJECT    DeviceObject
    )
{
    UNICODE_STRING      ntDeviceName;
    UNICODE_STRING      symbolicLinkName;
    PCONTROL_DEVICE_EXTENSION   deviceExtension;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    UNICODE_STRING  sddlString;    

    PAGED_CODE();    

    //
    // Using unsafe function so that the IRQL remains at PASSIVE_LEVEL.
    // IoCreateDeviceSecure & IoCreateSymbolicLink must be called at
    // PASSIVE_LEVEL.
    //
    ExAcquireFastMutexUnsafe(&ControlMutex);

    //
    // If this is a first instance of the device, then create a controlobject
    // and register dispatch points to handle ioctls.
    //
    if (1 == ++InstanceCount)
    {
        //
        // Initialize the unicode strings
        //
        RtlInitUnicodeString(&ntDeviceName, USBSNOOP_NTDEVICE_NAME_W);
        RtlInitUnicodeString(&symbolicLinkName, USBSNOOP_LINKNAME_W);

        //
        // Initialize a security descriptor string. Refer to SDDL docs in the SDK
        // for more info.
        //
        RtlInitUnicodeString( &sddlString, L"D:P(A;;GA;;;SY)(A;;GA;;;BA)");

        //
        // Create a named deviceobject so that applications or drivers
        // can directly talk to us without going throuhg the entire stack.
        // This call could fail if there are not enough resources or
        // another deviceobject of same name exists (name collision).
        // Let us use the new IoCreateDeviceSecure and specify a security
        // descriptor (SD) that allows only System and Admin groups to access the 
        // control device. Let us also specify a unique guid to allow administrators 
        // to change the SD if he desires to do so without changing the driver. 
        // The SD will be stored in 
        // HKLM\SYSTEM\CCSet\Control\Class\<GUID>\Properties\Security.
        // An admin can override the SD specified in the below call by modifying
        // the registry.
        //
        status = IoCreateDeviceSecure(DeviceObject->DriverObject,
            sizeof(CONTROL_DEVICE_EXTENSION),
            &ntDeviceName,
            FILE_DEVICE_USBSNOOP,   // Must match definition in Common.h
            FILE_DEVICE_SECURE_OPEN,
            FALSE, 
            &sddlString,
            (LPCGUID)&GUID_SD_FILTER_CONTROL_OBJECT,
            &ControlDeviceObject);

        if (NT_SUCCESS( status ))
        {
            ControlDeviceObject->Flags |= DO_BUFFERED_IO;

            status = IoCreateSymbolicLink( &symbolicLinkName, &ntDeviceName );

            if ( !NT_SUCCESS( status ) )
            {
                IoDeleteDevice(ControlDeviceObject);
                DebugPrint(("IoCreateSymbolicLink failed %x\n", status));
                goto End;
            }

            deviceExtension = (PCONTROL_DEVICE_EXTENSION )ControlDeviceObject->DeviceExtension;
            deviceExtension->CommonData.Type = FILTER_DEVICE_TYPE_CDO;
            deviceExtension->ControlData = NULL;
            deviceExtension->Deleted = FALSE;

            ControlDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

        }
        else
        {
            DebugPrint(("IoCreateDevice failed %x\n", status));
        }
    }

End:

    ExReleaseFastMutexUnsafe(&ControlMutex); 
    return status;
}

VOID
FilterDeleteControlObject()
{
    UNICODE_STRING      symbolicLinkName;
    PCONTROL_DEVICE_EXTENSION   deviceExtension;

    PAGED_CODE();    

    ExAcquireFastMutexUnsafe (&ControlMutex);

    //
    // If this is the last instance of the device then delete the controlobject
    // and symbolic link to enable the pnp manager to unload the driver.
    //

    if (!(--InstanceCount) && ControlDeviceObject)
    {
        RtlInitUnicodeString(&symbolicLinkName, USBSNOOP_LINKNAME_W);
        deviceExtension = (PCONTROL_DEVICE_EXTENSION )ControlDeviceObject->DeviceExtension;
        deviceExtension->Deleted = TRUE;
        IoDeleteSymbolicLink(&symbolicLinkName);
        IoDeleteDevice(ControlDeviceObject);
        ControlDeviceObject = NULL;
    }

    ExReleaseFastMutexUnsafe (&ControlMutex); 
}


NTSTATUS
FilterDispatchIo(
    __in PDEVICE_OBJECT    DeviceObject,
    __in PIRP              Irp
    )
/*++

Routine Description:

    This routine is the dispatch routine for non passthru irps.
    We will check the input device object to see if the request
    is meant for the control device object. If it is, we will
    handle and complete the IRP, if not, we will pass it down to 
    the lower driver.
    
Arguments:

    DeviceObject - Pointer to the device object.

    Irp - Pointer to the request packet.

Return Value:

    NT Status code
--*/
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status;
    PCONTROL_DEVICE_EXTENSION   deviceExtension;
    PCOMMON_DEVICE_DATA commonData;

    PAGED_CODE();

    commonData = (PCOMMON_DEVICE_DATA)DeviceObject->DeviceExtension;

    //
    // Please note that this is a common dispatch point for controlobject and
    // filter deviceobject attached to the pnp stack. 
    //
    if (commonData->Type == FILTER_DEVICE_TYPE_FIDO)
    {
        //
        // We will just  the request down as we are not interested in handling
        // requests that come on the PnP stack.
        //
        return FilterDispatchAny(DeviceObject, Irp);    
    }

    ASSERT(commonData->Type == FILTER_DEVICE_TYPE_CDO);

    deviceExtension = (PCONTROL_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    //
    // Else this is targeted at our control deviceobject so let's handle it.
    // Here we will handle the IOCTL requests that come from the app.
    // We don't have to worry about acquiring remlocks for I/Os that come 
    // on our control object because the I/O manager takes reference on our 
    // deviceobject when it initiates a request to our device and that keeps
    // our driver from unloading when we have pending I/Os. But we still
    // have to watch out for a scenario where another driver can send 
    // requests to our deviceobject directly without opening an handle.
    //
    if (!deviceExtension->Deleted)  //if not deleted
    {
        status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;
        irpStack = IoGetCurrentIrpStackLocation (Irp);

        switch (irpStack->MajorFunction)
        {
        case IRP_MJ_CREATE:
            DebugPrint(("Create \n"));
            break;

        case IRP_MJ_CLOSE:
            DebugPrint(("Close \n"));
            break;

        case IRP_MJ_CLEANUP:
            DebugPrint(("Cleanup \n"));
            break;

        case  IRP_MJ_DEVICE_CONTROL:
            DebugPrint(("DeviceIoControl\n"));
            switch (irpStack->Parameters.DeviceIoControl.IoControlCode)
            {
            case IOCTL_FILTERIO_QUERY_LOGGING_STATE:
               {
                  ULONG outputBufferLength;

                  outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

                  ASSERT( outputBufferLength == sizeof( ULONG ) );

                  if( outputBufferLength != sizeof( ULONG ) )
                  {
                     status = STATUS_INVALID_PARAMETER;
                  }
                  else
                  {
                     PULONG pLoggingState;

                     pLoggingState = (PULONG )Irp->AssociatedIrp.SystemBuffer;

                     *pLoggingState = 0;

                     if( g_bLoggingEnabled )
                     {
                        *pLoggingState |= LoggingEnabled;
                     }

                     if( g_hLogFile != (HANDLE )NULL )
                     {
                        *pLoggingState |= LogFileOpen;
                     }

                     Irp->IoStatus.Information = sizeof( ULONG );
                  }
               }
               break;

            case IOCTL_FILTERIO_SET_LOGGING_STATE:
               {
                  ULONG inputBufferLength;

                  inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;

                  ASSERT( inputBufferLength == sizeof( ULONG ) );

                  if( inputBufferLength != sizeof( ULONG ) )
                  {
                     status = STATUS_INVALID_PARAMETER;
                  }
                  else
                  {
                     PULONG pLoggingState;

                     pLoggingState = (PULONG )Irp->AssociatedIrp.SystemBuffer;

                     if( *pLoggingState & LoggingEnabled )
                     {
                        g_bLoggingEnabled = TRUE;
                     }
                     else
                     {
                        g_bLoggingEnabled = FALSE;
                     }
                  }
               }
               break;

            case IOCTL_FILTERIO_CLOSE_LOG_FILE:
               {
                  status = LogCloseFile();
               }
               break;

            case IOCTL_FILTERIO_DELETE_LOG_FILE:
               {
                  status = LogDeleteFile();
               }
               break;

            default:
                status = STATUS_NOT_SUPPORTED;
                break;
            }
        default:
            break;
        }
    }
    else
    {
        ASSERTMSG(FALSE, "Requests being sent to a dead device\n");
        status = STATUS_DEVICE_REMOVED;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return status;
}

#endif 

/////////////////////////////////////////////////////////////////////////////
//                          FDO Hooking Routines                           //
/////////////////////////////////////////////////////////////////////////////

IO_COMPLETION_ROUTINE MyInternalIOCTLCompletion;

NTSTATUS MyInternalIOCTLCompletion(__in PDEVICE_OBJECT fido, __in PIRP Irp, __in PVOID inContext)
{
    PDEVICE_EXTENSION   deviceExtension;
    struct Buffer       b;

    LogPrintf("UsbSnoop - MyInternalIOCTLCompletion(%p) : fido=%p, Irp=%p, Context=%p, IRQL=%d\n",
        MyInternalIOCTLCompletion,fido,Irp,inContext,KeGetCurrentIrql());

    PIRP_STACK_CONTEXT Context = (PIRP_STACK_CONTEXT )inContext;
    //	DumpContext(Context);

   // restoring information.
    ASSERT( Context );
    ASSERT( Context->Stack );

    deviceExtension = Context->deviceExtension;

    Context->Stack->CompletionRoutine = Context->CompletionRoutine;
    Context->Stack->Context           = Context->Context;
    Context->Stack->Control           = Context->Control;

    // dumping URB 
    BufferInit(&b);
    BufferPrintf(&b," <<<  URB %d coming back  <<< \n",Context->uSequenceNumber);
    DumpURB(&b,Context->pUrb, TRUE);

    LogBuffer(&b);
    BufferDone(&b);

    // saving some field before freeing the structure
    PVOID OldContext = Context->Context;
    PIO_COMPLETION_ROUTINE OldCompletionRoutine = Context->CompletionRoutine;

    // freeing the allocated structure
    ExFreeToNPagedLookasideList( &g_ContextLookasideList, Context );

    // calling the old CompletionRoutine, if there was one

    FilterDerefDevice( deviceExtension );

    if (OldCompletionRoutine != NULL)
        return OldCompletionRoutine(fido,Irp,OldContext);

    return STATUS_SUCCESS;
}

NTSTATUS
FdoHookDispatchInternalIoctl(
    __in PDEVICE_OBJECT DeviceObject,  // The FDO Device Object
    __in PIRP Irp
    )
/*++

Routine Description:

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

      NT status code

--*/
{
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);

    LogPrintf("UsbSnoop - FdoHookDispatchInternalIoctl(%p) : fdo=%p, Irp=%p, IRQL=%d\n",
        FdoHookDispatchInternalIoctl,DeviceObject,Irp,KeGetCurrentIrql());

    //
    // Locate _OUR_ FIDO DEVICE_EXTENSION in this convoluted scheme...
    // ---------------------------------------------------------------
    // This hook function has replaced the IRP_MJ_INTERNAL_DEVICE_CONTROL
    // callback of the FDO. So, when it is called the DeviceObject is
    // that of the FDO - _NOT_ our FIDO.
    //
    // The FDO AttachedDevice field can be used to recover our filter's
    // device object and the filter's device extension.
    //
    PDEVICE_OBJECT      FilterDeviceObject = DeviceObject->AttachedDevice;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION )FilterDeviceObject->DeviceExtension;

    if( irpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_INTERNAL_USB_SUBMIT_URB)
    {
        PURB pUrb = (PURB) irpStack->Parameters.Others.Argument1;

        if( !pUrb )
        {
            DebugPrint(("*** URB is NULL!!! ***\n" ));
            goto CallLowerDevice;
        }

        struct Buffer b;
        ULONG uSequenceNumber = InterlockedIncrement((PLONG)&deviceExtension->uSequenceNumber);

        BufferInit(&b);

        BufferPrintf(&b," >>>  URB %d going down  >>> \n", uSequenceNumber);

        DumpURB(&b,pUrb,FALSE);

        PIRP_STACK_CONTEXT Context = (PIRP_STACK_CONTEXT )ExAllocateFromNPagedLookasideList( &g_ContextLookasideList );

        if (Context != NULL)
        {
            // Memory is Uninitialized...
            RtlZeroMemory( Context, sizeof( IRP_STACK_CONTEXT ) );

            if( pUrb->UrbHeader.Length <= sizeof( URB ) )
            {
               RtlCopyBytes( &Context->OriginalUrb, pUrb, pUrb->UrbHeader.Length );
            }
            else
            {
               RtlCopyBytes( &Context->OriginalUrb, pUrb, sizeof( URB ) );
            }

            // Add Reference to Device
            FilterRefDevice( deviceExtension );
            Context->deviceExtension   = deviceExtension;

            Context->CompletionRoutine = irpStack->CompletionRoutine;
            Context->Context           = irpStack->Context;
            Context->Control           = irpStack->Control;
            Context->pUrb              = pUrb;
            Context->uSequenceNumber   = uSequenceNumber;
            Context->Stack             = irpStack;

            irpStack->CompletionRoutine = MyInternalIOCTLCompletion;
            irpStack->Context = Context;
            irpStack->Control = SL_INVOKE_ON_SUCCESS | SL_INVOKE_ON_ERROR | SL_INVOKE_ON_CANCEL;
        }
        else
            LogPrintf("  ExAllocatePool failed! Can't redirect CompletionRoutine\n");

        LogBuffer(&b);
        BufferDone(&b);
    }

CallLowerDevice:
    // Call The FDO's Original IRP_MJ_INTERNAL_DEVICE_CONTROL Callback
    return (deviceExtension->OriginalLowerDriverObject)->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]( DeviceObject,Irp );
}

NTSTATUS
FdoHookDispatchPnp(
    __in PDEVICE_OBJECT DeviceObject,  // The FDO Device Object
    __in PIRP Irp
    )
/*++

Routine Description:

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

      NT status code

--*/
{
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    const char * MinorFunctionName = PnPMinorFunctionString(irpStack->MinorFunction);

    if (MinorFunctionName != NULL)
        LogPrintf("UsbSnoop - FdoHookDispatchPnp(%p) : IRP_MJ_PNP (%s)\n",
            FilterDispatchPnp,MinorFunctionName);
    else
        LogPrintf("UsbSnoop - FdoHookDispatchPnp(%p) : IRP_MJ_PNP (0x%x)\n",
            FilterDispatchPnp,irpStack->MinorFunction);

    //
    // Locate _OUR_ FIDO DEVICE_EXTENSION in this convoluted scheme...
    // ---------------------------------------------------------------
    // This hook function has replaced the IRP_MJ_INTERNAL_DEVICE_CONTROL
    // callback of the FDO. So, when it is called the DeviceObject is
    // that of the FDO - _NOT_ our FIDO.
    //
    // The FDO AttachedDevice field can be used to recover our filter's
    // device object and the filter's device extension.
    //
    PDEVICE_OBJECT      FilterDeviceObject = DeviceObject->AttachedDevice;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION )FilterDeviceObject->DeviceExtension;

    // Provide Additional Information for IRP_MN_QUERY_INTERFACE
    if( irpStack->MinorFunction == IRP_MN_QUERY_INTERFACE )
    {
        if( IsEqualGUIDAligned(
                *irpStack->Parameters.QueryInterface.InterfaceType,
                USB_BUS_INTERFACE_HUB_GUID
                )
            )
        {
            LogPrintf("UsbSnoop - FdoHookDispatchPnp: Query for Bus interface for USB Hub drivers.\n");
            LogPrintf("\tInterfaceType: USB_BUS_INTERFACE_HUB_GUID\n");
        }
        else if( IsEqualGUIDAligned(
                *irpStack->Parameters.QueryInterface.InterfaceType,
                USB_BUS_INTERFACE_USBDI_GUID
                )
            )
        {
            LogPrintf("UsbSnoop - FdoHookDispatchPnp: Query for Bus interface for USB Function Drivers.\n");
            LogPrintf("\tInterfaceType: USB_BUS_INTERFACE_USBDI_GUID\n");
        }
        else
        {
            UNICODE_STRING GuidString;
            NTSTATUS ConvertStatus;

            ConvertStatus = RtlStringFromGUID(
                *irpStack->Parameters.QueryInterface.InterfaceType,
                &GuidString
                );

            if( ConvertStatus == STATUS_SUCCESS )
            {
                LogPrintf("UsbSnoop - FdoHookDispatchPnp: Query for InterfaceType %wZ.\n",&GuidString);

                RtlFreeUnicodeString( &GuidString );
            }
            else
            {
                LogPrintf("UsbSnoop - FdoHookDispatchPnp: Query for InterfaceType *** UNKNOWN ***.\n");
            }
        }
    }

    if( irpStack->MinorFunction == IRP_MN_REMOVE_DEVICE )
    {
        // Restore the driver pointer in the FDO
        DebugPrint(("Removing FDO Dispatch Table Hook (2)\n"));

        InterlockedExchangePointer(
            (PVOID * )&( (deviceExtension->LowerDeviceObject)->DriverObject ),
            deviceExtension->OriginalLowerDriverObject
            );
    }

    // Call The FDO's Original IRP_MJ_PNP Callback
    return (deviceExtension->OriginalLowerDriverObject)->MajorFunction[IRP_MJ_PNP]( DeviceObject,Irp );
}

