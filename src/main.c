#include <efi.h>
#include <efilib.h>
#include <bootloader/graphics.h>
#include <bootloader/console.h>
#include <bootloader/paging.h>
#include <bootloader/elfloader.h>

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    EFI_SYSTEM_TABLE *ST = SystemTable;
    Status = ST->BootServices->SetWatchdogTimer(0, 0, 0, 0);

    if(EFI_ERROR(setConsoleMode(ST)))
        printString(ST, EFI_RED, L"Could not set console mode\r\n");

    printString(ST, EFI_YELLOW, L"David's Bootloader\r\n");

    uint64_t *pml4 = pagingInit(ST);
    if(!pml4) 
    {
        printString(ST, EFI_RED, L"Could not initialize paging\r\n");
        return WaitForKeyPress(ST);
    }
    printString(ST, EFI_GREEN, L"Recursive paging initialized\r\n");
    printString(ST, EFI_GREEN, L"PML4 address ");
    printIntegerInHexadecimal(ST, EFI_GREEN, (uint64_t) pml4);
    newLine(ST);

    if(EFI_ERROR(loadKernel(ST, ImageHandle, pml4)))
    {
        printString(ST, EFI_RED, L"Could not load kernel\r\n");
        return WaitForKeyPress(ST);
    }
    printString(ST, EFI_GREEN, L"Kernel succesfully loaded and mapped\r\n");

    printString(ST, EFI_WHITE, L"BootInfo size: ");
    printIntegerInDecimal(ST, EFI_WHITE, sizeof(BootInfo));
    newLine(ST);

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = getGop(ST);
    if(gop == NULL)
    {
        printString(ST, EFI_RED, L"Could not get gop\r\n");
        return WaitForKeyPress(ST);
    }
    
    BootInfo *bootInfo = allocateZeroedPages(ST, 1);
    if(bootInfo == NULL || !memoryMapPages(ST, pml4, (uint64_t) bootInfo, 0xFFFEFFFFFFFFF000, 1))
    {
        printString(ST, EFI_RED, L"Could not allocate bootinfo\r\n");
        return WaitForKeyPress(ST); 
    }

    UINT32 gopMode = obtainGraphicsMode(gop, &(bootInfo->framebuffer));
    if(gopMode == UINT32_MAX)
    {
        printString(ST, EFI_RED, L"Could not get gop mode\r\n");
        return WaitForKeyPress(ST);
    }
    printString(ST, EFI_GREEN, L"Chosen gop mode\r\n");
    printFrameBufferInfo(ST, &(bootInfo->framebuffer));

    WaitForKeyPress(ST);
    return EFI_SUCCESS;
}
