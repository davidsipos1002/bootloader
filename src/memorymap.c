#include <bootloader/memorymap.h>
#include <bootloader/console.h>
#include <bootloader/paging.h>

UINTN getMemoryMap(EFI_SYSTEM_TABLE *ST, MemoryMap *memoryMap)
{
    UINTN MemoryMapSize = PAGE_SIZE;
    EFI_MEMORY_DESCRIPTOR *MemoryMapPointer = (EFI_MEMORY_DESCRIPTOR *) memoryMap->map;
    if(!MemoryMapPointer)
        MemoryMapPointer = allocateZeroedPages(ST, EfiLoaderData, 1);
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
    uint32_t pageCount = 1;

    EFI_STATUS Status;
    while((Status = ST->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMapPointer, 
            &MapKey, &DescriptorSize, &DescriptorVersion)) == EFI_BUFFER_TOO_SMALL) 
    {
        ST->BootServices->FreePages((EFI_PHYSICAL_ADDRESS) MemoryMapPointer, pageCount);
        MemoryMapSize += PAGE_SIZE;
        pageCount++;
        MemoryMapPointer = allocateZeroedPages(ST, EfiLoaderData, pageCount);
    }

    if(Status != EFI_SUCCESS || DescriptorVersion != EFI_MEMORY_DESCRIPTOR_VERSION)
        return UINT64_MAX;

    memoryMap->descriptor_size = DescriptorSize;
    memoryMap->size = MemoryMapSize;
    memoryMap->map = (uint64_t) MemoryMapPointer;
    
    return MapKey;
}

void printMemoryMapInfo(EFI_SYSTEM_TABLE *ST, MemoryMap *memoryMap) 
{
    printString(ST, EFI_WHITE, L"MemoryMap: ");
    printString(ST, EFI_WHITE, L"descriptor_size: ");
    printIntegerInDecimal(ST, EFI_WHITE, memoryMap->descriptor_size);
    printString(ST, EFI_WHITE, L" ");
    printString(ST, EFI_WHITE, L"size: ");
    printIntegerInDecimal(ST, EFI_WHITE, memoryMap->size);
    printString(ST, EFI_WHITE, L" ");
    printString(ST, EFI_WHITE, L"map: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, memoryMap->map);
    printString(ST, EFI_WHITE, L" ");
    newLine(ST);
}

void printMemoryMap(EFI_SYSTEM_TABLE *ST, MemoryMap *memoryMap)
{
    uint64_t count = memoryMap->size / memoryMap->descriptor_size;
    for(int i = 0;i < count;i++)
    {
        EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *) ((uint8_t *) memoryMap->map + i * memoryMap->descriptor_size);
        printIntegerInDecimal(ST, EFI_WHITE, desc->Type);
        printString(ST, EFI_WHITE, L" ");
        printIntegerInHexadecimal(ST, EFI_WHITE, desc->PhysicalStart);
        printString(ST, EFI_WHITE, L" ");
        printIntegerInDecimal(ST, EFI_WHITE, desc->NumberOfPages);
        printString(ST, EFI_WHITE, L"; ");
    }
    newLine(ST);
}