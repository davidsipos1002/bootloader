#include <efi.h>
#include <efilib.h>
#include <bootloader/graphics.h>
#include <bootloader/console.h>
#include <bootloader/paging.h>
#include <bootloader/elfloader.h>
#include <bootloader/memorymap.h>
#include <bootloader/kerneljump.h>
#include <bootloader/bootloadercfg.h>

EFI_STATUS die(EFI_SYSTEM_TABLE *ST, CHAR16 *Message)
{
    printString(ST, EFI_RED, Message);
    return WaitForKeyPress(ST);
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    EFI_SYSTEM_TABLE *ST = SystemTable;
    Status = ST->BootServices->SetWatchdogTimer(0, 0, 0, 0);

    if(EFI_ERROR(setConsoleMode(ST)))
        printString(ST, EFI_RED, L"Could not set console mode\r\n");

    printString(ST, EFI_YELLOW, L"David's Bootloader v");
    printIntegerInDecimal(ST, EFI_YELLOW, BOOTLOADER_MAJOR);
    printString(ST, EFI_YELLOW, L".");
    printIntegerInDecimal(ST, EFI_YELLOW, BOOTLOADER_MINOR);
    newLine(ST);

    uint64_t *pml4 = pagingInit(ST);
    if(!pml4) 
        return die(ST, L"Could not initialize paging\r\n");
    printString(ST, EFI_GREEN, L"Recursive paging initialized\r\n");
    printString(ST, EFI_GREEN, L"PML4 address ");
    printIntegerInHexadecimal(ST, EFI_GREEN, (uint64_t) pml4);
    newLine(ST);

    uint64_t kernelEntry;
    if(EFI_ERROR(loadKernel(ST, ImageHandle, pml4, &kernelEntry)))
        return die(ST, L"Could not load kernel\r\n");
    printString(ST, EFI_GREEN, L"Kernel succesfully loaded and mapped at ");
    printIntegerInHexadecimal(ST, EFI_GREEN, kernelEntry);
    newLine(ST);

    printString(ST, EFI_WHITE, L"BootInfo size: ");
    printIntegerInDecimal(ST, EFI_WHITE, sizeof(BootInfo));
    newLine(ST);
    BootInfo *bootInfo = allocateZeroedPages(ST, 1);
    if(bootInfo == NULL || !memoryMapPages(ST, pml4, (uint64_t) bootInfo, BOOTLOADER_BOOTINFO_ADDRESS, 1))
        return die(ST, L"Could not allocate bootinfo\r\n");

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = getGop(ST);
    if(gop == NULL)
        return die(ST, L"Could not get gop\r\n");
    UINT32 gopMode = obtainGraphicsMode(gop, &(bootInfo->framebuffer));
    if(gopMode == UINT32_MAX)
        return die(ST, L"Could not get gop mode\r\n");
    printString(ST, EFI_GREEN, L"Chosen gop mode\r\n");
    printFrameBufferInfo(ST, &(bootInfo->framebuffer));
    
    bootInfo->efi_system_table = (uint64_t) ST;
    bootInfo->page_table = (uint64_t) pml4;

    uint64_t kernelJump;
    if(EFI_ERROR(loadKernelJump(ST, pml4, &kernelJump)))
        return die(ST, L"Could not load KernelJump\r\n");
    printString(ST, EFI_GREEN, L"Successfully loaded KernelJump at ");
    printIntegerInHexadecimal(ST, EFI_GREEN, kernelJump);
    newLine(ST);

    printString(ST, EFI_YELLOW, L"Press any key to jump to kernel \r\n");
    WaitForKeyPress(ST);

    if(EFI_ERROR(gop->SetMode(gop, gopMode)))
        return die(ST, L"Could not set gop mode\r\n");

    UINTN MapKey = getMemoryMap(ST, &(bootInfo->memorymap)); 
    if(MapKey == UINT64_MAX)
        return EFI_LOAD_ERROR;
 
    while((Status = ST->BootServices->ExitBootServices(ImageHandle, MapKey)) != EFI_SUCCESS) 
        MapKey = getMemoryMap(ST, &(bootInfo->memorymap));

    KernelJump jump = (KernelJump) kernelJump;
    jump(BOOTLOADER_BOOTINFO_ADDRESS, (uint64_t) pml4, kernelEntry);
    return EFI_SUCCESS;
}
