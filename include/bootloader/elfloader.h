#ifndef ELFLOADER_H_INC
#define ELFLOADER_H_INC

#include <efi.h>
#include <efilib.h>

EFI_STATUS loadElf(EFI_SYSTEM_TABLE *ST, EFI_FILE_HANDLE kernelImage);

#endif