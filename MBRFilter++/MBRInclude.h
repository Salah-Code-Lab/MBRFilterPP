#pragma once
#include <ntddk.h>
#include <wdm.h>
#include <ntdddisk.h>
#include <ntddscsi.h> 
#include <scsi.h> 
#include <ntintsafe.h>
extern PULONG InitSafeBootMode;

//   BUGCHECK CODES


#define BUGCHECK_FORBIDDEN_SECTOR_WRITE         0xE911  
#define BUGCHECK_SECTOR_BACKUP_WRITE_FORBIDDEN  0xE912  

// Protection Constants

#define CRITICAL_ZONE_END               63ULL
#define HEAD_PROTECT_START            64ULL
#define HEAD_PROTECT_END              5119ULL 
#define EXEMPT_MFTMIRR_START          2048ULL 
#define EXEMPT_MFTMIRR_END            2080ULL  
#define TAIL_PROTECTED_SECTOR_COUNT     64ULL 
#define SCSIOP_WRITE6           0x0A
#define SCSIOP_WRITE            0x2A    /* WRITE(10) */
#define SCSIOP_WRITE12          0xAA
#define SCSIOP_WRITE16          0x8A
#define SCSIOP_WRITE_VERIFY     0x2E
#define SCSIOP_WRITE_VERIFY12   0xAE
#define SCSIOP_WRITE_VERIFY16   0x8E
#define SCSIOP_WRITE_SAME       0x41
#define SCSIOP_WRITE_SAME16     0x93
#define SCSIOP_UNMAP            0x42
#define MBRFPP_WRITE_TO_END   MAXULONG64


#define MIN_ARMABLE_SECTORS           5185ULL

            
#define MBR_SIGNATURE_OFFSET          0x1FE
#define MBR_SIGNATURE_VALUE           0xAA55
#define MBR_PARTITION_TABLE_OFFSET    0x1BE
#define GPT_PROTECTIVE_TYPE           0xEE


#define MBRFPP_TAG                    'FMBR'

// Dev extension

typedef struct _MBRFILTERPP_DEVICE_EXTENSION
{
    PDEVICE_OBJECT  LowerDeviceObject;     
    ULONG64         BytesPerSector;        
    BOOLEAN         IsGPT;                 
    BOOLEAN         ProtectionEnabled;    
    ULONG64         TotalSectors;        
    ULONG64         TailStartSector;        

} MBRFILTERPP_DEVICE_EXTENSION, * PMBRFILTERPP_DEVICE_EXTENSION;



#pragma pack(push, 1)
typedef struct _MBR_PARTITION_ENTRY
{
    UCHAR   Status;
    UCHAR   FirstCHSHead;
    UCHAR   FirstCHSSector;
    UCHAR   FirstCHSCylinder;
    UCHAR   PartitionType;
    UCHAR   LastCHSHead;
    UCHAR   LastCHSSector;
    UCHAR   LastCHSCylinder;
    ULONG   FirstLBA;
    ULONG   SectorCount;
} MBR_PARTITION_ENTRY;
#pragma pack(pop)

// Moar and MOAR declarations yes MOAR not more

DRIVER_ADD_DEVICE   MBRFilterPP_AddDevice;
DRIVER_UNLOAD       MBRFilterPP_DriverUnload;

DRIVER_DISPATCH     MBRFilterPP_DispatchWrite;
DRIVER_DISPATCH     MBRFilterPP_DispatchPnP;
DRIVER_DISPATCH     MBRFilterPP_DispatchPower;
DRIVER_DISPATCH     MBRFilterPP_DispatchPassThrough;
DRIVER_DISPATCH MBRFilterPP_DispatchDeviceControl;

NTSTATUS
MBRFilterPP_CheckSectorRange(
    _In_ PMBRFILTERPP_DEVICE_EXTENSION  DevExt,
    _In_ ULONG64                        StartSector,
    _In_ ULONG64                        EndSector,
    _In_ PDEVICE_OBJECT                 DeviceObject
);
