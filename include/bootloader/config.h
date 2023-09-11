#ifndef CONFIG_H_INCL
#define CONFIG_H_INCL

#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include <stdbool.h>
#include <bootloader/string.h>

typedef struct 
{
    uint32_t filePathLength;
    char filePath[MAX_STRING_LENGTH + 1];
    uint64_t loadVirtualAddress;
} InitialDataFile;

typedef struct
{
    uint32_t configNameLength;
    char configName[MAX_STRING_LENGTH + 1];
    uint32_t kernelPathLength;
    char kernelPath[MAX_STRING_LENGTH + 1];
    uint64_t bootInfoVirtualAddress;
    uint32_t dataFileSize;
    uint32_t dataFileCount;
    InitialDataFile *dataFiles;
} BootConfig;

BootConfig* parseConfigurationFile(EFI_SYSTEM_TABLE *ST, EFI_FILE_HANDLE rootDirectory);
void printBootConfig(EFI_SYSTEM_TABLE *ST, BootConfig *bootConfig);
void freeBootConfig(EFI_SYSTEM_TABLE *ST, BootConfig *bootConfig);

#endif