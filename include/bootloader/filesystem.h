#ifndef FILESYS_H_INC
#define FILESYS_H_INC

#include <efi.h>
#include <efilib.h>

EFI_FILE_HANDLE getRootDirectory(EFI_HANDLE Image, EFI_SYSTEM_TABLE *ST);
EFI_STATUS openKernelImage(EFI_FILE_HANDLE rootDirectory, EFI_FILE_HANDLE *kernelImage); 

#endif