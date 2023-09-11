#ifndef ELFLOADER_H_INCL
#define ELFLOADER_H_INCL

#include <efi.h>
#include <efilib.h>

EFI_STATUS loadKernel(EFI_SYSTEM_TABLE *ST, EFI_FILE_HANDLE rootDirectory, CHAR16* kernelPath, uint64_t *pml4, uint64_t *kernelEntry);

#endif