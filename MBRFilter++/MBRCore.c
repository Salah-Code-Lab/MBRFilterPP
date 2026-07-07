/*
 * MBRFilter++ (MBRFilterPP.c)
* Fancy Symbols:÷ + - x * 🂡 (ace of spades)
 * 
 */


#include "MBRInclude.h"
#pragma warning(disable: 4996) 


// Hel;er

static NTSTATUS
MBRFilterPP_SendIoctl(
    _In_      PDEVICE_OBJECT  TargetDevice,
    _In_      ULONG           IoControlCode,
    _In_opt_  PVOID           InputBuffer,
    _In_      ULONG           InputBufferLength,
    _Out_opt_ PVOID           OutputBuffer,
    _In_      ULONG           OutputBufferLength
)
{
    KEVENT          event;
    PIRP            irp;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS        status;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
        IoControlCode,
        TargetDevice,
        InputBuffer,
        InputBufferLength,
        OutputBuffer,
        OutputBufferLength,
        FALSE,
        &event,
        &ioStatus
    );

    if (irp == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    status = IoCallDriver(TargetDevice, irp);

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    return status;
}


VOID MBRFilterPP_Main64(
    _In_ ULONG64 StartSector,
    _In_ ULONG64 EndSector
)
{
    UNICODE_STRING	   msgTitle, msgText;
    ULONG_PTR param[3];
    UNREFERENCED_PARAMETER(StartSector);
    UNREFERENCED_PARAMETER(EndSector);


    RtlInitUnicodeString(&msgTitle, L"MBRFilter++ Critical Sector Protection");


    RtlInitUnicodeString(&msgText,
        L"A program attempted to write to critical boot sectors (sectors 0-63).\n\n"
        L"This operation has been BLOCKED to prevent boot corruption.\n\n"
        L"If you need to Repartition the Drive or Partition\n"
        L"Retry the operation from Safe Mode\n\n"
        L"If a another app was ran as Administrator Turn off your Internet Now and notify your Admin immediately.\n\n"
        L"Press OK to continue.");



    param[0] = (ULONG_PTR)&msgText;
    param[1] = (ULONG_PTR)&msgTitle;
    param[2] = 0x40;

    ULONG response = 0;
    ExRaiseHardError(STATUS_SERVICE_NOTIFICATION, 3, 3, param, 1, &response);
}






VOID MBRFilterPP_Last64(
    _In_ ULONG64 StartSector,
    _In_ ULONG64 EndSector

)
{
    UNREFERENCED_PARAMETER(StartSector);
    UNREFERENCED_PARAMETER(EndSector);
    ULONG_PTR		   param[3];
    UNICODE_STRING	   msgTitle, msgText;

    RtlInitUnicodeString(&msgTitle, L"MBRFilter++ Critical Sector Protection");


    RtlInitUnicodeString(&msgText,
        L"A program attempted to write to critical Backup sectors (Last 64 sectors of your disk Or partition).\n\n"
        L"This operation has been BLOCKED to prevent Boot backup corruption.\n\n"
        L"If a another app was ran as Administrator Turn off your Internet Now and notify your Admin immediately.\n\n"
        L"Press OK to continue.");



    param[0] = (ULONG_PTR)&msgText;
    param[1] = (ULONG_PTR)&msgTitle;
    param[2] = 0x40;

    ULONG response = 0;
    ExRaiseHardError(STATUS_SERVICE_NOTIFICATION, 3, 3, param, 1, &response);
}


VOID MBRFilterPP_Mid64_5119(
    _In_ ULONG64 StartSector,
    _In_ ULONG64 EndSector
)
{
    UNICODE_STRING	   msgTitle, msgText;
    ULONG_PTR param[3];
    UNREFERENCED_PARAMETER(StartSector);
    UNREFERENCED_PARAMETER(EndSector);


    RtlInitUnicodeString(&msgTitle, L"MBRFilter++ Critical Sector Protection");

    RtlInitUnicodeString(&msgText,
        L"A program attempted to write to critical sectors (Sectors64-5119 of your disk Or partition).\n\n"
        L"This operation has been BLOCKED to prevent Boot corruption.\n\n"
        L"If you ran a repartitioning tool boot to safe mode and retry the repartition operation.\n\n"
        L"If another app was ran as Administrator Turn off your Internet Now and notify your Admin immediately.\n\n"
        L"Press OK to continue.");

    param[0] = (ULONG_PTR)&msgText;
    param[1] = (ULONG_PTR)&msgTitle;
    param[2] = 0x40;

    ULONG response = 0;
    ExRaiseHardError(STATUS_SERVICE_NOTIFICATION, 3, 3, param, 1, &response);
}










// Query Physical Sectors I should had used MmIoMapToSpace Yeah sure sure (Sarcastic piece of shi)

static NTSTATUS
MBRFilterPP_QuerySectorSize(
    _In_  PDEVICE_OBJECT    LowerDevice,
    _Out_ PULONG64          BytesPerSector
)
{
    NTSTATUS                            status;
    STORAGE_PROPERTY_QUERY              query;
    STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR alignDesc;
    DISK_GEOMETRY_EX                    geoEx;

    *BytesPerSector = 0;

    RtlZeroMemory(&query, sizeof(query));
    RtlZeroMemory(&alignDesc, sizeof(alignDesc));

    query.PropertyId = StorageAccessAlignmentProperty;
    query.QueryType = PropertyStandardQuery;

    status = MBRFilterPP_SendIoctl(
        LowerDevice,
        IOCTL_STORAGE_QUERY_PROPERTY,
        &query, sizeof(query),
        &alignDesc, sizeof(alignDesc)
    );

    if (NT_SUCCESS(status) && alignDesc.BytesPerLogicalSector != 0)
    {
        ULONG64 bps = (ULONG64)alignDesc.BytesPerLogicalSector;

        if (bps != 0 && (bps & (bps - 1)) == 0)
        {
            *BytesPerSector = bps;
            return STATUS_SUCCESS;
        }

        KdPrint(("MBRFilter++: Bogus or Hot Garbage BytesPerLogicalSector %llu, falling back\n", bps));
    }

    KdPrint(("MBRFilter++: StorageAccessAlignmentProperty failed (0x%08X), "
        "falling back to DISK_GEOMETRY_EX\n", status));

    RtlZeroMemory(&geoEx, sizeof(geoEx));

    status = MBRFilterPP_SendIoctl(
        LowerDevice,
        IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
        NULL, 0,
        &geoEx, sizeof(geoEx)
    );

    if (!NT_SUCCESS(status))
        return status;

    if (geoEx.Geometry.BytesPerSector == 0)
        return STATUS_DEVICE_DATA_ERROR;

    *BytesPerSector = (ULONG64)geoEx.Geometry.BytesPerSector;
    return STATUS_SUCCESS;
}

// Query sector amount

static NTSTATUS
MBRFilterPP_QueryTotalSectors(
    _In_  PDEVICE_OBJECT    LowerDevice,
    _In_  ULONG64           BytesPerSector,
    _Out_ PULONG64          TotalSectors
)
{
    NTSTATUS            status;
    DISK_GEOMETRY_EX    geoEx;

    *TotalSectors = 0;

    RtlZeroMemory(&geoEx, sizeof(geoEx));

    status = MBRFilterPP_SendIoctl(
        LowerDevice,
        IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
        NULL, 0,
        &geoEx, sizeof(geoEx)
    );

    if (!NT_SUCCESS(status))
        return status;

    if (geoEx.DiskSize.QuadPart <= 0)
        return STATUS_DEVICE_DATA_ERROR;

    *TotalSectors = (ULONG64)geoEx.DiskSize.QuadPart / BytesPerSector;
    return STATUS_SUCCESS;
}

static NTSTATUS
MBRFilterPP_DetectGPT(
    _In_  PDEVICE_OBJECT    LowerDevice,
    _Out_ PBOOLEAN          IsGPT
)
{
    NTSTATUS                    status;
    KEVENT                      event;
    PIRP                        irp;
    IO_STATUS_BLOCK             ioStatus;
    LARGE_INTEGER               offset;
    PUCHAR                      buffer = NULL;
    USHORT                      mbrSig;
    MBR_PARTITION_ENTRY UNALIGNED* partEntry;

    *IsGPT = FALSE;

    buffer = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, 512, MBRFPP_TAG);
    if (buffer == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    offset.QuadPart = 0;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_READ,
        LowerDevice,
        buffer,
        512,
        &offset,
        &event,
        &ioStatus
    );

    if (irp == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    status = IoCallDriver(LowerDevice, irp);

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    if (!NT_SUCCESS(status))
    {
        KdPrint(("MBRFilter++: Sector 0 read failed (0x%08X) defaulting to MBR\n",
            status));
        status = STATUS_SUCCESS;
        goto Cleanup;
    }

    mbrSig = *(USHORT UNALIGNED*)(buffer + MBR_SIGNATURE_OFFSET);

    if (mbrSig != MBR_SIGNATURE_VALUE)
    {
        KdPrint(("MBRFilter++: No MBR signature raw/uninitialized disk, MBR Partioned maybe i don't know better safe then Sorry\n"));
        goto Cleanup;
    }

    partEntry = (MBR_PARTITION_ENTRY UNALIGNED*)(buffer + MBR_PARTITION_TABLE_OFFSET);

    if (partEntry->PartitionType == GPT_PROTECTIVE_TYPE)
    {
        KdPrint(("MBRFilter++: GPT protective MBR detected tail protection IS ARMED\n"));
        *IsGPT = TRUE;
    }
    else
    {
        KdPrint(("MBRFilter++: MBR partition scheme tail protection IS INACTIVE\n"));
    }

    status = STATUS_SUCCESS;

Cleanup:
    ExFreePoolWithTag(buffer, MBRFPP_TAG);
    return status;
}






static NTSTATUS
MBRFilterPP_PnPCompletion(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP           Irp,
    _In_ PVOID          Context
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

static NTSTATUS
MBRFilterPP_StartDeviceCompletion(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP           Irp,
    _In_ PVOID          Context
)
{
    PMBRFILTERPP_DEVICE_EXTENSION devExt =
        (PMBRFILTERPP_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    UNREFERENCED_PARAMETER(Context);

    if (NT_SUCCESS(Irp->IoStatus.Status) && !devExt->ProtectionEnabled)
    {
        ULONG64 bytesPerSector = 0;
        ULONG64 totalSectors = 0;
        BOOLEAN isGPT = FALSE;

        if (NT_SUCCESS(MBRFilterPP_QuerySectorSize(devExt->LowerDeviceObject, &bytesPerSector))
            && bytesPerSector != 0)
        {
            devExt->BytesPerSector = bytesPerSector;

            if (NT_SUCCESS(MBRFilterPP_QueryTotalSectors(devExt->LowerDeviceObject, bytesPerSector, &totalSectors))
                && totalSectors >= MIN_ARMABLE_SECTORS)
            {
                ULONG64 maxDiskBytes = 0;

                // Mathematically verify that Sectors * SectorSize fits inside the disk not garbage writes to bypass
                if (!NT_SUCCESS(RtlULongLongMult(totalSectors, bytesPerSector, &maxDiskBytes)))
                {
                    KdPrint(("MBRFilter++: CRITICAL Spoofed or insane disk size overflow detected! Refusing to arm.\n"));
                    devExt->ProtectionEnabled = FALSE;
                    return STATUS_CONTINUE_COMPLETION;
                }
              

                devExt->TotalSectors = totalSectors;
                devExt->TailStartSector = totalSectors - TAIL_PROTECTED_SECTOR_COUNT;

                if (!NT_SUCCESS(MBRFilterPP_DetectGPT(devExt->LowerDeviceObject, &isGPT)))
                    isGPT = FALSE;

                devExt->IsGPT = isGPT;
                devExt->ProtectionEnabled = TRUE;

                KdPrint((
                    "MBRFilter++: ARMED\n"
                    "   BytesPerSector : %llu\n"
                    "   TotalSectors   : %llu\n"
                    "   TailStartSector: %llu\n"
                    "   IsGPT          : %s\n",
                    bytesPerSector, totalSectors,
                    devExt->TailStartSector,
                    isGPT ? "YES" : "NO"
                    ));
            }
        }

        if (!devExt->ProtectionEnabled)
            KdPrint(("MBRFilter++: Not armed — staying passive\n"));
    }

    if (Irp->PendingReturned)
        IoMarkIrpPending(Irp);

    return STATUS_CONTINUE_COMPLETION;
}

NTSTATUS
MBRFilterPP_DispatchPnP(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP           Irp
)
{
    PMBRFILTERPP_DEVICE_EXTENSION   devExt;
    PIO_STACK_LOCATION              stack;
    UCHAR                           minorFunction;
    NTSTATUS                        status;

    devExt = (PMBRFILTERPP_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation(Irp);
    minorFunction = stack->MinorFunction;

    switch (minorFunction)
    {
    case IRP_MN_START_DEVICE:
        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(
            Irp,
            MBRFilterPP_StartDeviceCompletion,
            NULL,
            TRUE, TRUE, TRUE
        );
        return IoCallDriver(devExt->LowerDeviceObject, Irp);

    case IRP_MN_REMOVE_DEVICE:
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(devExt->LowerDeviceObject, Irp);
        IoDetachDevice(devExt->LowerDeviceObject);
        IoDeleteDevice(DeviceObject);

        return status;

    default:
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(devExt->LowerDeviceObject, Irp);

    }

}

NTSTATUS
MBRFilterPP_DispatchPower(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP           Irp
)
{
    PMBRFILTERPP_DEVICE_EXTENSION devExt =
        (PMBRFILTERPP_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(devExt->LowerDeviceObject, Irp);
}


 NTSTATUS
MBRFilterPP_DispatchPassThrough(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP           Irp
)
{
    PMBRFILTERPP_DEVICE_EXTENSION devExt =
        (PMBRFILTERPP_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(devExt->LowerDeviceObject, Irp);
}

// Add Da Device

 NTSTATUS
     MBRFilterPP_AddDevice(
         _In_ PDRIVER_OBJECT DriverObject,
         _In_ PDEVICE_OBJECT PhysicalDeviceObject
     )
 {
     NTSTATUS                        status;
     PDEVICE_OBJECT                  filterDevice = NULL;
     PMBRFILTERPP_DEVICE_EXTENSION   devExt = NULL;
     PDEVICE_OBJECT                  lowerDevice = NULL;

     status = IoCreateDevice(
         DriverObject,
         sizeof(MBRFILTERPP_DEVICE_EXTENSION),
         NULL,
         FILE_DEVICE_DISK,
         FILE_DEVICE_SECURE_OPEN,
         FALSE,
         &filterDevice
     );

     if (!NT_SUCCESS(status))
         return status;

     devExt = (PMBRFILTERPP_DEVICE_EXTENSION)filterDevice->DeviceExtension;
     RtlZeroMemory(devExt, sizeof(MBRFILTERPP_DEVICE_EXTENSION));
     devExt->ProtectionEnabled = FALSE;

     lowerDevice = IoAttachDeviceToDeviceStack(filterDevice, PhysicalDeviceObject);

     if (lowerDevice == NULL)
     {
         IoDeleteDevice(filterDevice);
         return STATUS_NO_SUCH_DEVICE;
     }

     devExt->LowerDeviceObject = lowerDevice;

     filterDevice->Flags |= lowerDevice->Flags & (DO_BUFFERED_IO | DO_DIRECT_IO);
     filterDevice->Flags &= ~DO_DEVICE_INITIALIZING;
     filterDevice->Characteristics = lowerDevice->Characteristics;

     // NO QUERIES. Stack isn't ready yet.
     return STATUS_SUCCESS;
 }





 NTSTATUS
     MBRFilterPP_CheckSectorRange(
         _In_ PMBRFILTERPP_DEVICE_EXTENSION  DevExt,
         _In_ ULONG64                        StartSector,
         _In_ ULONG64                        EndSector,
         _In_ PDEVICE_OBJECT                 DeviceObject
     )
 {
     UNREFERENCED_PARAMETER(DeviceObject);
     if (ExGetPreviousMode() == KernelMode) {
         if (StartSector <= CRITICAL_ZONE_END)
         {
             return STATUS_ACCESS_DENIED;
         }

         if (DevExt->IsGPT && (EndSector >= DevExt->TailStartSector))
         {
             return STATUS_ACCESS_DENIED;
         }

         if ((EndSector >= 64ULL) && (StartSector <= HEAD_PROTECT_END))
         {
             BOOLEAN fullyInExempt = (StartSector >= EXEMPT_MFTMIRR_START) &&
                 (EndSector <= EXEMPT_MFTMIRR_END);

             if (!fullyInExempt)
             {
                 return STATUS_ACCESS_DENIED;
             }
         }
        
     }
     else
     {
         if (StartSector <= CRITICAL_ZONE_END)
         {
             MBRFilterPP_Main64(StartSector, EndSector);
             return STATUS_ACCESS_DENIED;
         }

         if (DevExt->IsGPT && (EndSector >= DevExt->TailStartSector))
         {
             MBRFilterPP_Last64(StartSector, EndSector);
             return STATUS_ACCESS_DENIED;
         }

         if ((EndSector >= 64ULL) && (StartSector <= HEAD_PROTECT_END))
         {
             BOOLEAN fullyInExempt = (StartSector >= EXEMPT_MFTMIRR_START) &&
                 (EndSector <= EXEMPT_MFTMIRR_END);

             if (!fullyInExempt)
             {
                 MBRFilterPP_Mid64_5119(StartSector, EndSector);
                 return STATUS_ACCESS_DENIED;
             }
         }

       
     }
     return STATUS_SUCCESS;
 }


 NTSTATUS
     MBRFilterPP_DispatchWrite(
         _In_ PDEVICE_OBJECT DeviceObject,
         _In_ PIRP           Irp
     )
 {
     PMBRFILTERPP_DEVICE_EXTENSION   devExt;
     PIO_STACK_LOCATION              stack;
     ULONG64                         byteOffset;
     ULONG64                         writeLength;
     ULONG64                         startSector;
     ULONG64                         endSector;
     ULONG64                         bps;
     NTSTATUS                        status;

     devExt = (PMBRFILTERPP_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

     if (!devExt->ProtectionEnabled)
     {
         IoSkipCurrentIrpStackLocation(Irp);
         return IoCallDriver(devExt->LowerDeviceObject, Irp);
     }

     stack = IoGetCurrentIrpStackLocation(Irp);
     byteOffset = (ULONG64)stack->Parameters.Write.ByteOffset.QuadPart;
     writeLength = (ULONG64)stack->Parameters.Write.Length;
     bps = devExt->BytesPerSector;

   
     if (writeLength == 0)
     {
         IoSkipCurrentIrpStackLocation(Irp);
         return IoCallDriver(devExt->LowerDeviceObject, Irp);
     }

   
     if ((byteOffset % bps) != 0)
     {
         KdPrint((
             "MBRFilter++: IRP_MJ_WRITE rejected — "
             "misaligned byte offset %llu (BytesPerSector=%llu)\n",
             byteOffset, bps
             ));
         Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
         Irp->IoStatus.Information = 0;
         IoCompleteRequest(Irp, IO_NO_INCREMENT);
         return STATUS_INVALID_PARAMETER;
     }

     ULONG64 maxDiskBytes = devExt->TotalSectors * bps;

     if (byteOffset >= maxDiskBytes || writeLength > (maxDiskBytes - byteOffset))
     {
         KdPrint(("MBRFilter++: Rejected massive or out-of-bounds IRP_MJ_WRITE length!\n"));
         Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
         Irp->IoStatus.Information = 0;
         IoCompleteRequest(Irp, IO_NO_INCREMENT);
         return STATUS_INVALID_PARAMETER;
     }
    

     startSector = byteOffset / bps;

     endSector = (byteOffset + writeLength - 1) / bps;

     status = MBRFilterPP_CheckSectorRange(devExt, startSector, endSector, DeviceObject);

     if (!NT_SUCCESS(status))
     {
         Irp->IoStatus.Status = status;
         Irp->IoStatus.Information = 0;
         IoCompleteRequest(Irp, IO_NO_INCREMENT);
         return status;
     }

     IoSkipCurrentIrpStackLocation(Irp);
     return IoCallDriver(devExt->LowerDeviceObject, Irp);
 }

// Driver Unload
 VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject)
 {
     PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;

     while (deviceObject != NULL) {
         // Use your actual struct name
         PMBRFILTERPP_DEVICE_EXTENSION devExt = (PMBRFILTERPP_DEVICE_EXTENSION)deviceObject->DeviceExtension;

         // Change 'TargetDeviceObject' to 'LowerDeviceObject' here
         if (devExt->LowerDeviceObject) {
             IoDetachDevice(devExt->LowerDeviceObject);
         }

         PDEVICE_OBJECT nextDeviceObject = deviceObject->NextDevice;

         IoDeleteDevice(deviceObject);

         deviceObject = nextDeviceObject;
     }
 }

// Driver Entry


 NTSTATUS
     DriverEntry(
         _In_ PDRIVER_OBJECT  DriverObject,
         _In_ PUNICODE_STRING RegistryPath
     )
 {
     ULONG i;
     UNREFERENCED_PARAMETER(RegistryPath);
     if (*InitSafeBootMode > 0) {
         DriverObject->DriverUnload = DriverUnload;
         return STATUS_SUCCESS;
     }
     else {
         DriverObject->DriverUnload = NULL;
     }

     KdPrint(("MBRFilter++: DriverEntry\n"));

     for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
         DriverObject->MajorFunction[i] = MBRFilterPP_DispatchPassThrough;

     DriverObject->MajorFunction[IRP_MJ_WRITE] = MBRFilterPP_DispatchWrite;
     DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MBRFilterPP_DispatchDeviceControl;
     DriverObject->MajorFunction[IRP_MJ_PNP] = MBRFilterPP_DispatchPnP;
     DriverObject->MajorFunction[IRP_MJ_POWER] = MBRFilterPP_DispatchPower;
     DriverObject->DriverExtension->AddDevice = MBRFilterPP_AddDevice;
 
    

    KdPrint(("MBRFilter++: DriverEntry complete\n"));
    return STATUS_SUCCESS;
};
