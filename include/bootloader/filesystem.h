#ifndef FILESYS_H_INCL
#define FILESYS_H_INCL

#include <efi.h>
#include <efilib.h>

EFI_FILE_HANDLE getRootDirectory(EFI_HANDLE Image, EFI_SYSTEM_TABLE *ST);
EFI_STATUS openFileForRead(EFI_FILE_HANDLE rootDirectory, CHAR16 *path, EFI_FILE_HANDLE *fileHandle); 
EFI_STATUS closeFileHandle(EFI_FILE_HANDLE fileHandle);

#endif