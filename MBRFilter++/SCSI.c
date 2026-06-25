#include "MBRInclude.h"

/*
 * MBRFilterPP SCSI.c
 */


VOID MBRFilterPPFailure(
    _In_ ULONG64 StartSector,
    _In_ ULONG64 EndSector
)
{
    UNREFERENCED_PARAMETER(StartSector);
    UNREFERENCED_PARAMETER(EndSector);

    UNICODE_STRING msgTitle;
    RtlInitUnicodeString(&msgTitle, L"MBRFilter++ Critical Sector Protection");

    UNICODE_STRING msgText;
    RtlInitUnicodeString(&msgText,
        L"WARNING: ACCESS_VIOLATION.\n"
        L"An invalid buffer or length was passed to the disk stack.\n\n"
        L"This may indicate a malicious application attempting to bypass security checks. "
        L"If you recently ran an application capable of writing to critical boot sectors, "
        L"contact your administrator immediately.\n\n"
        L"Press OK to continue.");

    ULONG_PTR param[3];
    param[0] = (ULONG_PTR)&msgText;
    param[1] = (ULONG_PTR)&msgTitle;
    param[2] = 0x40;

    ULONG response = 0;
    ExRaiseHardError(STATUS_SERVICE_NOTIFICATION, 3, 3, param, 1, &response);
}


 /* -------------------------------------------------------------------------
  *  SCSI CDB cracking
  * ------------------------------------------------------------------------- */

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
        *SectorCount = MAXULONG64;
        return TRUE;

    case 0x85: 
    {
        if (CdbLength < 16) return FALSE;

      
        UCHAR protocol = (Cdb[1] >> 1) & 0x0F;
        UCHAR ataCommand = Cdb[14];

       
        if (protocol == 5 || protocol == 7 ||
            ataCommand == 0x30 || ataCommand == 0x35 || ataCommand == 0x24)
        {
            *StartSector = 0;
            *SectorCount = MAXULONG64;
            return TRUE;
        }
        return FALSE;
    }

    case 0xA1: // ATA PASS-THROUGH (12)
    {
        if (CdbLength < 12) return FALSE;

        UCHAR protocol = (Cdb[1] >> 1) & 0x0F;
        UCHAR ataCommand = Cdb[10]; 

        if (protocol == 5 || protocol == 7 ||
            ataCommand == 0x30 || ataCommand == 0x35 || ataCommand == 0x24)
        {
            *StartSector = 0;
            *SectorCount = MAXULONG64;
            return TRUE;
        }
        return FALSE;
    }


    default:
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 *  SCSI CDB extraction helpers
 * ------------------------------------------------------------------------- */

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
            return STATUS_INFO_LENGTH_MISMATCH;

        if (spt32->DataBufferOffset != 0 && spt32->DataTransferLength != 0)
        {
            if (spt32->DataBufferOffset >= InputBufferLength ||
                spt32->DataTransferLength > InputBufferLength - spt32->DataBufferOffset)
            {
                return STATUS_INVALID_PARAMETER;
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
            return STATUS_INFO_LENGTH_MISMATCH;

        if (spt->DataBufferOffset != 0 && spt->DataTransferLength != 0)
        {
            if (spt->DataBufferOffset >= InputBufferLength ||
                spt->DataTransferLength > InputBufferLength - spt->DataBufferOffset)
            {
                return STATUS_INVALID_PARAMETER;
            }
        }

        *OutCdb = spt->Cdb;
        *OutCdbLength = spt->CdbLength;
        return STATUS_SUCCESS;
    }
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
            return STATUS_INFO_LENGTH_MISMATCH;

        if (sptd32->DataBuffer != 0 && sptd32->DataTransferLength != 0)
        {
            PVOID dataPtr = (PVOID)(ULONG_PTR)sptd32->DataBuffer;
            __try
            {
                ProbeForRead(dataPtr, sptd32->DataTransferLength, sizeof(UCHAR));
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return STATUS_ACCESS_VIOLATION;
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
            return STATUS_INFO_LENGTH_MISMATCH;

        if (sptd->DataBuffer != NULL && sptd->DataTransferLength != 0)
        {
            __try
            {
                ProbeForRead(sptd->DataBuffer, sptd->DataTransferLength, sizeof(UCHAR));
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return STATUS_ACCESS_VIOLATION;
            }
        }

        *OutCdb = sptd->Cdb;
        *OutCdbLength = sptd->CdbLength;
        return STATUS_SUCCESS;
    }
}


/* -------------------------------------------------------------------------
 *  ATA passthrough cracking
 * ------------------------------------------------------------------------- */

static BOOLEAN
MBRFilterPP_CrackWriteATA(
    _In_  PVOID     AptBuffer,
    _Out_ PULONG64  StartSector,
    _Out_ PULONG64  SectorCount
)
{
    *StartSector = 0;
    *SectorCount = 0;

    PATA_PASS_THROUGH_EX apt = (PATA_PASS_THROUGH_EX)AptBuffer;
    UCHAR cmd = apt->CurrentTaskFile[6];

    switch (cmd)
    {
    case ATA_CMD_WRITE_DMA:
    case ATA_CMD_WRITE_DMA_EXT:
    case ATA_CMD_WRITE_DMA_QUEUED:
    case ATA_CMD_WRITE_DMA_QUEUED_EXT:
    case ATA_CMD_WRITE_FPDMA_QUEUED:
    case ATA_CMD_WRITE_SECTORS:
    case ATA_CMD_WRITE_SECTORS_EXT:
    case ATA_CMD_WRITE_VERIFY:
    case ATA_CMD_WRITE_VERIFY_EXT:
    {
        BOOLEAN isLBA = (apt->CurrentTaskFile[5] & 0x40) != 0;

        /* SECURITY FIX: CHS addressing — block entire device */
        if (!isLBA)
        {
            *StartSector = 0;
            *SectorCount = MAXULONG64;
            return TRUE;
        }

        ULONG64 lba28 = ((ULONG64)apt->CurrentTaskFile[2]) |
            ((ULONG64)apt->CurrentTaskFile[3] << 8) |
            ((ULONG64)apt->CurrentTaskFile[4] << 16) |
            (((ULONG64)apt->CurrentTaskFile[5] & 0x0F) << 24);

        ULONG64 lba48 = ((ULONG64)apt->CurrentTaskFile[2]) |
            ((ULONG64)apt->CurrentTaskFile[3] << 8) |
            ((ULONG64)apt->CurrentTaskFile[4] << 16) |
            ((ULONG64)apt->PreviousTaskFile[2] << 24) |
            ((ULONG64)apt->PreviousTaskFile[3] << 32) |
            ((ULONG64)apt->PreviousTaskFile[4] << 40);

        ULONG64 count = apt->CurrentTaskFile[1] |
            ((ULONG64)apt->PreviousTaskFile[1] << 8);

        BOOLEAN is48Bit = (apt->PreviousTaskFile[2] != 0) ||
            (apt->PreviousTaskFile[3] != 0) ||
            (apt->PreviousTaskFile[4] != 0) ||
            (apt->PreviousTaskFile[1] != 0);

        if (is48Bit)
        {
            *StartSector = lba48;
            if (count == 0) count = 65536;
        }
        else
        {
            *StartSector = lba28;
            if (count == 0) count = 256;
        }

        *SectorCount = count;
        return TRUE;
    }

    case ATA_CMD_SECURITY_ERASE_UNIT:
    case ATA_CMD_DOWNLOAD_MICROCODE:
    case ATA_CMD_DOWNLOAD_MICROCODE_DMA:
        *StartSector = 0;
        *SectorCount = MAXULONG64;
        return TRUE;

    default:
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 *  Main dispatch
 * ------------------------------------------------------------------------- */

NTSTATUS
MBRFilterPP_DispatchDeviceControl(
    _In_    PDEVICE_OBJECT  DeviceObject,
    _Inout_ PIRP            Irp
)
{
    PMBRFILTERPP_DEVICE_EXTENSION  devExt;
    PIO_STACK_LOCATION             stack;
    ULONG                          ioControlCode;
    ULONG                          inputBufferLength;
    PVOID                          systemBuffer;
    PUCHAR                         cdb = NULL;
    UCHAR                          cdbLength = 0;
    ULONG64                        startSector = 0;
    ULONG64                        sectorCount = 0;
    ULONG64                        endSector = 0;
    BOOLEAN                        isWrite = FALSE;
    NTSTATUS                       status;

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

    /* Fast-path exclusion block */
    if (ioControlCode != IOCTL_SCSI_PASS_THROUGH &&
        ioControlCode != IOCTL_SCSI_PASS_THROUGH_DIRECT &&
        ioControlCode != IOCTL_ATA_PASS_THROUGH &&
        ioControlCode != IOCTL_ATA_PASS_THROUGH_DIRECT)
    {
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(devExt->LowerDeviceObject, Irp);
    }

    /* SECURITY FIX: Pre-validation of buffer presence */
    if (systemBuffer == NULL)
    {
        status = STATUS_INVALID_PARAMETER;
        goto FailFastRequest;
    }

    /* Enforce minimum input buffer validation */
    switch (ioControlCode)
    {
    case IOCTL_SCSI_PASS_THROUGH:
        if (inputBufferLength < sizeof(SCSI_PASS_THROUGH))
        {
            status = STATUS_INFO_LENGTH_MISMATCH; goto FailFastRequest;
        }
        status = MBRFilterPP_ExtractCDB_Buffered(
            Irp, systemBuffer, inputBufferLength, &cdb, &cdbLength);
        if (!NT_SUCCESS(status)) goto FailFastRequest;
        isWrite = MBRFilterPP_CrackWriteCDB(cdb, cdbLength, &startSector, &sectorCount);
        break;

    case IOCTL_SCSI_PASS_THROUGH_DIRECT:
        if (inputBufferLength < sizeof(SCSI_PASS_THROUGH_DIRECT))
        {
            status = STATUS_INFO_LENGTH_MISMATCH; goto FailFastRequest;
        }
        status = MBRFilterPP_ExtractCDB_Direct(
            Irp, systemBuffer, inputBufferLength, &cdb, &cdbLength);
        if (!NT_SUCCESS(status)) goto FailFastRequest;
        isWrite = MBRFilterPP_CrackWriteCDB(cdb, cdbLength, &startSector, &sectorCount);
        break;

    case IOCTL_ATA_PASS_THROUGH:
    case IOCTL_ATA_PASS_THROUGH_DIRECT:
        if (inputBufferLength < sizeof(ATA_PASS_THROUGH_EX))
        {
            status = STATUS_INFO_LENGTH_MISMATCH; goto FailFastRequest;
        }
        isWrite = MBRFilterPP_CrackWriteATA(systemBuffer, &startSector, &sectorCount);
        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto FailFastRequest;
    }

    /* Non-write operations pass through */
    if (!isWrite)
    {
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(devExt->LowerDeviceObject, Irp);
    }

    if (sectorCount == 0)
    {
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(devExt->LowerDeviceObject, Irp);
    }

    /* Integer overflow protection */
    if (sectorCount > MAXULONG64 - startSector)
        endSector = MAXULONG64;
    else
        endSector = startSector + sectorCount - 1;

    /* Range check */
    status = MBRFilterPP_CheckSectorRange(devExt, startSector, endSector, DeviceObject);

    if (!NT_SUCCESS(status))
    {
        UCHAR opcode = 0;
        if (cdb != NULL && cdbLength > 0)
            opcode = cdb[0];

        KdPrint((
            "MBRFilter++: Passthrough write BLOCKED sectors [%llu, %llu] IOCTL 0x%08X opcode 0x%02X\n",
            startSector, endSector, ioControlCode, opcode
            ));

        
        Irp->IoStatus.Status = status;          // Will be STATUS_ACCESS_DENIED
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;                        
    }

    // Safe requests pass through normally
    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(devExt->LowerDeviceObject, Irp);



FailFastRequest:
    // If the request failed validation during buffer parsing, alert the user
    if (status == STATUS_INVALID_PARAMETER ||
        status == STATUS_INFO_LENGTH_MISMATCH ||
        status == STATUS_ACCESS_VIOLATION)
    {
        MBRFilterPPFailure(0, 0);
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}
