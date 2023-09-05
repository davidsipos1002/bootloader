#ifndef KERNELJUMP_H_INCL
#define KERNELJUMP_H_INCL

#include <efi.h>
#include <efilib.h>

typedef void (*KernelJump) (uint64_t BootInfo, uint64_t PageTable, uint64_t KernelEntry);

EFI_STATUS loadKernelJump(EFI_SYSTEM_TABLE *ST, uint64_t *kernelJump); 

#endif