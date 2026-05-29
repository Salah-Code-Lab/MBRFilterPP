#include "MBRInclude.h"

VOID
MBRFilterPP_BugCheckCallback(
    _In_ KBUGCHECK_CALLBACK_REASON Reason,
    _In_ PKBUGCHECK_REASON_CALLBACK_RECORD Record,
    _Inout_ PVOID ReasonSpecificData,
    _In_ ULONG ReasonSpecificDataLength
)
{
    UNREFERENCED_PARAMETER(ReasonSpecificDataLength);
    UNREFERENCED_PARAMETER(Record);
    // The latest WDK uses this specific pointer cast
    PKBUGCHECK_SECONDARY_DUMP_DATA dumpData = (PKBUGCHECK_SECONDARY_DUMP_DATA)ReasonSpecificData;

    if (Reason == KbCallbackSecondaryDumpData)
    {
        const char* msg = "\r\n[MBRFilter++]: MBR/GPT Protection Violation. Sector write blocked.\r\n";
        ULONG len = 0;
        while (msg[len] != '\0') len++;

        // Check if the buffer is valid and has enough space
        if (dumpData->OutBuffer != NULL && len <= dumpData->OutBufferLength)
        {
            RtlCopyMemory(
                dumpData->OutBuffer,
                msg,
                len
            );
            dumpData->OutBufferLength = len;
        }
    }
}