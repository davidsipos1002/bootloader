#ifndef MEMORYMAP_H_INCL
#define MEMORYMAP_H_INCL

#include <efi.h>
#include <efilib.h>
#include <bootloader/bootinfo.h> 

UINTN getMemoryMap(EFI_SYSTEM_TABLE *ST, MemoryMap *memoryMap);
void printMemoryMapInfo(EFI_SYSTEM_TABLE *ST, MemoryMap *memoryMap);
void printMemoryMap(EFI_SYSTEM_TABLE *ST, MemoryMap *memoryMap);

#endif