#ifndef ELFLOADER_H_INC
#define ELFLOADER_H_INC

#include <efi.h>
#include <efilib.h>

EFI_STATUS loadKernel(EFI_SYSTEM_TABLE *ST, EFI_HANDLE ImageHandle, uint64_t *pml4);
EFI_STATUS loadElf(EFI_SYSTEM_TABLE *ST, EFI_FILE_HANDLE kernelImage, uint64_t *pml4);

#endif