#ifndef BOOTCONTEXT_H_INCL
#define BOOTCONTEXT_H_INCL

#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include <bootloader/bootinfo.h>

typedef struct 
{
    EFI_HANDLE ImageHandle;
    EFI_SYSTEM_TABLE *ST;
    uint64_t *pml4;
    uint64_t kernelEntry;
    BootInfo *bootInfo;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    UINT32 gopMode;
    uint64_t kernelJumpAddress;
    UINTN MemoryMapKey;
} BootContext;

#endif