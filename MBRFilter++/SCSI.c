#include "MBRInclude.h"

/*
 * MBRFilterPP_SCSI.c
 * SCSI Passthrough Write Interception
 */

static BOOLEAN
MBRFilterPP_CrackWriteCDB(
    _In_  PUCHAR    Cdb,
    _In_  UCHAR     CdbLength,
    _Out_ PULONG64  StartSector,
    _Out_ PULONG64  SectorCount
)
{
    *StartSector = 0;
    *SectorCount = 0;

    if (Cdb == NULL || CdbLength == 0)
        return FALSE;

    switch (Cdb[0])
    {

    case SCSIOP_WRITE6:
        if (CdbLength < 6) return FALSE;
        *StartSector = ((ULONG64)(Cdb[1] & 0x1F) << 16) |
            ((ULONG64)Cdb[2] << 8) |
            ((ULONG64)Cdb[3]);
        *SectorCount = (Cdb[4] == 0) ? 256 : Cdb[4];
        return TRUE;


    case SCSIOP_WRITE:
    case SCSIOP_WRITE_VERIFY:
        if (CdbLength < 10) return FALSE;
        *StartSector = ((ULONG64)Cdb[2] << 24) |
            ((ULONG64)Cdb[3] << 16) |
            ((ULONG64)Cdb[4] << 8) |
            ((ULONG64)Cdb[5]);
        *SectorCount = ((ULONG64)Cdb[7] << 8) |
            ((ULONG64)Cdb[8]);
        return TRUE;


    case SCSIOP_WRITE12:
    case SCSIOP_WRITE_VERIFY12:
        if (CdbLength < 12) return FALSE;
        *StartSector = ((ULONG64)Cdb[2] << 24) |
            ((ULONG64)Cdb[3] << 16) |
            ((ULONG64)Cdb[4] << 8) |
            ((ULONG64)Cdb[5]);
        *SectorCount = ((ULONG64)Cdb[6] << 24) |
            ((ULONG64)Cdb[7] << 16) |
            ((ULONG64)Cdb[8] << 8) |
            ((ULONG64)Cdb[9]);
        return TRUE;


    case SCSIOP_WRITE16:
    case SCSIOP_WRITE_VERIFY16:
        if (CdbLength < 16) return FALSE;
        *StartSector = ((ULONG64)Cdb[2] << 56) |
            ((ULONG64)Cdb[3] << 48) |
            ((ULONG64)Cdb[4] << 40) |
            ((ULONG64)Cdb[5] << 32) |
            ((ULONG64)Cdb[6] << 24) |
            ((ULONG64)Cdb[7] << 16) |
            ((ULONG64)Cdb[8] << 8) |
            ((ULONG64)Cdb[9]);
        *SectorCount = ((ULONG64)Cdb[10] << 24) |
            ((ULONG64)Cdb[11] << 16) |
            ((ULONG64)Cdb[12] << 8) |
            ((ULONG64)Cdb[13]);
        return TRUE;


    case SCSIOP_WRITE_SAME:
        if (CdbLength < 10) return FALSE;
        *StartSector = ((ULONG64)Cdb[2] << 24) |
            ((ULONG64)Cdb[3] << 16) |
            ((ULONG64)Cdb[4] << 8) |
            ((ULONG64)Cdb[5]);
        *SectorCount = ((ULONG64)Cdb[7] << 8) |
            ((ULONG64)Cdb[8]);
        if (*SectorCount == 0)
            *SectorCount = MAXULONG64 - *StartSector;
        return TRUE;


    case SCSIOP_WRITE_SAME16:
        if (CdbLength < 16) return FALSE;
        *StartSector = ((ULONG64)Cdb[2] << 56) |
            ((ULONG64)Cdb[3] << 48) |
            ((ULONG64)Cdb[4] << 40) |
            ((ULONG64)Cdb[5] << 32) |
            ((ULONG64)Cdb[6] << 24) |
            ((ULONG64)Cdb[7] << 16) |
            ((ULONG64)Cdb[8] << 8) |
            ((ULONG64)Cdb[9]);
        *SectorCount = ((ULONG64)Cdb[10] << 24) |
            ((ULONG64)Cdb[11] << 16) |
            ((ULONG64)Cdb[12] << 8) |
            ((ULONG64)Cdb[13]);
        if (*SectorCount == 0)
            *SectorCount = MAXULONG64 - *StartSector;
        return TRUE;


    case SCSIOP_UNMAP:
        *StartSector = 0;
        *SectorCount = 0;
        return TRUE;

    default:
        return FALSE;
    }
}



static NTSTATUS
MBRFilterPP_ExtractCDB_Buffered(
    _In_  PIRP      Irp,
    _In_  PVOID     SystemBuffer,
    _In_  ULONG     InputBufferLength,
    _Out_ PUCHAR* OutCdb,
    _Out_ PUCHAR    OutCdbLength
)
{
#if defined(_WIN64)
    if (IoIs32bitProcess(Irp))
    {
        PSCSI_PASS_THROUGH32 spt32 = (PSCSI_PASS_THROUGH32)SystemBuffer;

        if (InputBufferLength < sizeof(SCSI_PASS_THROUGH32))
            goto InvalidParam;

        /* Sanity: DataBufferOffset must not point outside the buffer */
        if (spt32->DataBufferOffset != 0 && spt32->DataTransferLength != 0)
        {
            if (spt32->DataBufferOffset >= InputBufferLength ||
                spt32->DataTransferLength > InputBufferLength - spt32->DataBufferOffset)
            {
                KdPrint(("MBRFilter++: SPT32 DataBufferOffset overflows buffer\n"));
                goto InvalidParam;
            }
        }

        *OutCdb = spt32->Cdb;
        *OutCdbLength = spt32->CdbLength;
        return STATUS_SUCCESS;
    }
#else
    UNREFERENCED_PARAMETER(Irp);
#endif

    {
        PSCSI_PASS_THROUGH spt = (PSCSI_PASS_THROUGH)SystemBuffer;

        if (InputBufferLength < sizeof(SCSI_PASS_THROUGH))
            goto InvalidParam;

        if (spt->DataBufferOffset != 0 && spt->DataTransferLength != 0)
        {
            if (spt->DataBufferOffset >= InputBufferLength ||
                spt->DataTransferLength > InputBufferLength - spt->DataBufferOffset)
            {
                KdPrint(("MBRFilter++: SPT DataBufferOffset overflows buffer\n"));
                goto InvalidParam;
            }
        }

        *OutCdb = spt->Cdb;
        *OutCdbLength = spt->CdbLength;
        return STATUS_SUCCESS;
    }

InvalidParam:
    Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INVALID_PARAMETER;
}


static NTSTATUS
MBRFilterPP_ExtractCDB_Direct(
    _In_  PIRP      Irp,
    _In_  PVOID     SystemBuffer,
    _In_  ULONG     InputBufferLength,
    _Out_ PUCHAR* OutCdb,
    _Out_ PUCHAR    OutCdbLength
)
{
#if defined(_WIN64)
    if (IoIs32bitProcess(Irp))
    {
        PSCSI_PASS_THROUGH_DIRECT32 sptd32 = (PSCSI_PASS_THROUGH_DIRECT32)SystemBuffer;

        if (InputBufferLength < sizeof(SCSI_PASS_THROUGH_DIRECT32))
            goto InvalidParam;

        if (sptd32->DataBuffer != 0 && sptd32->DataTransferLength != 0)
        {
            PVOID dataPtr = (PVOID)(ULONG_PTR)sptd32->DataBuffer;

            __try
            {
                ProbeForRead(dataPtr, sptd32->DataTransferLength, sizeof(UCHAR));
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                KdPrint(("MBRFilter++: ProbeForRead on SPTD32 DataBuffer faulted\n"));
                goto AccessViolation;
            }
        }

        *OutCdb = sptd32->Cdb;
        *OutCdbLength = sptd32->CdbLength;
        return STATUS_SUCCESS;
    }
#else
    UNREFERENCED_PARAMETER(Irp);
#endif

    {
        PSCSI_PASS_THROUGH_DIRECT sptd = (PSCSI_PASS_THROUGH_DIRECT)SystemBuffer;

        if (InputBufferLength < sizeof(SCSI_PASS_THROUGH_DIRECT))
            goto InvalidParam;

        if (sptd->DataBuffer != NULL && sptd->DataTransferLength != 0)
        {
            __try
            {
                ProbeForRead(sptd->DataBuffer, sptd->DataTransferLength, sizeof(UCHAR));
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                goto AccessViolation;
            }
        }

        *OutCdb = sptd->Cdb;
        *OutCdbLength = sptd->CdbLength;
        return STATUS_SUCCESS;
    }

InvalidParam:
    Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INVALID_PARAMETER;

AccessViolation:
    Irp->IoStatus.Status = STATUS_ACCESS_VIOLATION;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_ACCESS_VIOLATION;
}




NTSTATUS
MBRFilterPP_DispatchDeviceControl(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP           Irp
)
{
    PMBRFILTERPP_DEVICE_EXTENSION   devExt;
    PIO_STACK_LOCATION              stack;
    ULONG                           ioControlCode;
    ULONG                           inputBufferLength;
    PVOID                           systemBuffer;
    PUCHAR                          cdb = NULL;
    UCHAR                           cdbLength = 0;
    ULONG64                         startSector = 0;
    ULONG64                         sectorCount = 0;
    ULONG64                         endSector = 0;
    BOOLEAN                         isWrite;
    NTSTATUS                        status;

    devExt = (PMBRFILTERPP_DEVICE_EXTENSION)DeviceObject->DeviceExtension;


    if (!devExt->ProtectionEnabled)
    {
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(devExt->LowerDeviceObject, Irp);
    }

    stack = IoGetCurrentIrpStackLocation(Irp);
    ioControlCode = stack->Parameters.DeviceIoControl.IoControlCode;
    inputBufferLength = stack->Parameters.DeviceIoControl.InputBufferLength;
    systemBuffer = Irp->AssociatedIrp.SystemBuffer;


    if (ioControlCode != IOCTL_SCSI_PASS_THROUGH &&
        ioControlCode != IOCTL_SCSI_PASS_THROUGH_DIRECT)
    {
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(devExt->LowerDeviceObject, Irp);
    }


    if (systemBuffer == NULL)
    {
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(devExt->LowerDeviceObject, Irp);
    }

    /* Extract CDB */
    if (ioControlCode == IOCTL_SCSI_PASS_THROUGH)
    {
        status = MBRFilterPP_ExtractCDB_Buffered(
            Irp, systemBuffer, inputBufferLength, &cdb, &cdbLength);
    }
    else
    {
        status = MBRFilterPP_ExtractCDB_Direct(
            Irp, systemBuffer, inputBufferLength, &cdb, &cdbLength);
    }

    if (!NT_SUCCESS(status))
        return status;

    isWrite = MBRFilterPP_CrackWriteCDB(cdb, cdbLength, &startSector, &sectorCount);

    if (!isWrite || sectorCount == 0)
    {
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(devExt->LowerDeviceObject, Irp);
    }

    /* Overflow guard */
    if (sectorCount > MAXULONG64 - startSector)
        endSector = MAXULONG64;
    else
        endSector = startSector + sectorCount - 1;

    /* Range check */
    status = MBRFilterPP_CheckSectorRange(devExt, startSector, endSector, DeviceObject);

    if (!NT_SUCCESS(status))
    {
        KdPrint((
            "MBRFilter++: SCSI passthrough write BLOCKED "
            "sectors [%llu, %llu] opcode 0x%02X\n",
            startSector, endSector, cdb[0]
            ));
        KeBugCheckEx(
            BUGCHECK_FORBIDDEN_SECTOR_WRITE,
            (ULONG_PTR)startSector,
            (ULONG_PTR)endSector,
            (ULONG_PTR)cdb[0],
            (ULONG_PTR)DeviceObject
        );
    }

    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(devExt->LowerDeviceObject, Irp);
}