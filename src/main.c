#include <efi.h>
#include <efilib.h>
#include <bootloader/graphics.h>
#include <bootloader/console.h>
#include <bootloader/paging.h>
#include <bootloader/elfloader.h>
#include <bootloader/memorymap.h>
#include <bootloader/kerneljump.h>
#include <bootloader/bootloadercfg.h>
#include <bootloader/bootcontext.h>

EFI_STATUS die(EFI_SYSTEM_TABLE *ST, CHAR16 *Message)
{
    printString(ST, EFI_RED, Message);
    return waitForKeyPress(ST);
}

void setupConsole(BootContext *bootContext)
{
    if(EFI_ERROR(setConsoleMode(bootContext->ST)))
        printString(bootContext->ST, EFI_RED, L"Could not set console mode\r\n");
    printString(bootContext->ST, EFI_YELLOW, L"David's Bootloader v");
    printIntegerInDecimal(bootContext->ST, EFI_YELLOW, BOOTLOADER_MAJOR);
    printString(bootContext->ST, EFI_YELLOW, L".");
    printIntegerInDecimal(bootContext->ST, EFI_YELLOW, BOOTLOADER_MINOR);
    newLine(bootContext->ST);
}

void initializePaging(BootContext *bootContext)
{
    uint64_t *pml4 = pagingInit(bootContext->ST);
    if(!pml4) 
        die(bootContext->ST, L"Could not initialize paging\r\n");
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_GREEN, L"Recursive paging initialized\r\n");
        printString(bootContext->ST, EFI_GREEN, L"PML4 address ");
        printIntegerInHexadecimal(bootContext->ST, EFI_GREEN, (uint64_t) pml4);
        newLine(bootContext->ST);
    #endif
    bootContext->pml4 = pml4;
}

void loadAndMapKernel(BootContext *bootContext)
{
    uint64_t kernelEntry;
    if(EFI_ERROR(loadKernel(bootContext->ST, bootContext->ImageHandle, bootContext->pml4, &kernelEntry)))
        die(bootContext->ST, L"Could not load kernel\r\n");
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_GREEN, L"Kernel succesfully loaded and mapped at ");
        printIntegerInHexadecimal(bootContext->ST, EFI_GREEN, kernelEntry);
        newLine(bootContext->ST);
    #endif
    bootContext->kernelEntry = kernelEntry;
}

void setupBootInfo(BootContext *bootContext)
{
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_WHITE, L"BootInfo size: ");
        printIntegerInDecimal(bootContext->ST, EFI_WHITE, sizeof(BootInfo));
        newLine(bootContext->ST);
    #endif
    BootInfo *bootInfo = allocateZeroedPages(bootContext->ST, 1);
    if(bootInfo == NULL || !memoryMapPages(bootContext->ST, bootContext->pml4, (uint64_t) bootInfo, BOOTLOADER_BOOTINFO_ADDRESS, 1))
        die(bootContext->ST, L"Could not allocate bootinfo\r\n");
    bootInfo->page_table = (uint64_t) bootContext->pml4;
    bootContext->bootInfo = bootInfo;
}

void setupGop(BootContext *bootContext)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = getGop(bootContext->ST);
    if(gop == NULL)
        die(bootContext->ST, L"Could not get gop\r\n");
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_GREEN, L"Obtained gop\r\n");
    #endif
    UINT32 gopMode = obtainGraphicsMode(gop, &(bootContext->bootInfo->framebuffer));
    if(gopMode == UINT32_MAX)
        die(bootContext->ST, L"Could not get gop mode\r\n");
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_GREEN, L"Chosen gop mode\r\n");
        printFrameBufferInfo(bootContext->ST, &(bootContext->bootInfo->framebuffer));
    #endif
    bootContext->gop = gop;
    bootContext->gopMode = gopMode;
}

void prepareKernelJump(BootContext *bootContext)
{
    uint64_t kernelJump;
    if(EFI_ERROR(loadKernelJump(bootContext->ST, bootContext->pml4, &kernelJump)))
        die(bootContext->ST, L"Could not load KernelJump\r\n");
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_GREEN, L"Successfully loaded KernelJump at ");
        printIntegerInHexadecimal(bootContext->ST, EFI_GREEN, kernelJump);
        newLine(bootContext->ST);
    #endif
    bootContext->kernelJumpAddress = kernelJump;
}

void obtainInitialMemoryMap(BootContext *bootContext)
{
    UINTN MapKey = getMemoryMap(bootContext->ST, &(bootContext->bootInfo->memorymap));
    if(MapKey == UINT64_MAX)
        die(bootContext->ST, L"Could not get initial memory map\r\n");
    bootContext->MemoryMapKey = MapKey;
    #ifdef BASIC_LOGGING
        printMemoryMapInfo(bootContext->ST, &(bootContext->bootInfo->memorymap));
    #endif
    #ifdef PRINT_MEMORY_MAP
        printMemoryMap(bootContext->ST, &(bootContext->bootInfo->memorymap));
    #endif
}

void displayFinalMessage(BootContext *bootContext)
{
    printString(bootContext->ST, EFI_GREEN, L"All looks good\r\n");
    printString(bootContext->ST, EFI_YELLOW, L"Press any key to jump to kernel...\r\n");
    waitForKeyPress(bootContext->ST);
}

void setDisplayMode(BootContext *bootContext)
{
    if(EFI_ERROR(bootContext->gop->SetMode(bootContext->gop, bootContext->gopMode)))
        die(bootContext->ST, L"Could not set gop mode\r\n");
}

void exitBootServices(BootContext *bootContext)
{
    UINTN MapKey = bootContext->MemoryMapKey;
    while(bootContext->ST->BootServices->ExitBootServices(bootContext->ImageHandle, MapKey) != EFI_SUCCESS) 
        MapKey = getMemoryMap(bootContext->ST, &(bootContext->bootInfo->memorymap));
}

void clearEfiSystemTable(BootContext *bootContext)
{
    bootContext->ST->ConsoleInHandle = NULL;
    bootContext->ST->ConIn = NULL;
    bootContext->ST->ConsoleOutHandle = NULL;
    bootContext->ST->ConOut = NULL;
    bootContext->ST->StandardErrorHandle = NULL;
    bootContext->ST->StdErr = NULL;
    bootContext->ST->BootServices = NULL;
}

void jumpToKernel(BootContext *bootContext)
{
    KernelJump jump = (KernelJump) bootContext->kernelJumpAddress;
    jump(BOOTLOADER_BOOTINFO_ADDRESS, (uint64_t) bootContext->pml4, bootContext->kernelEntry);
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, 0);

    BootContext bootContext;
    bootContext.ImageHandle = ImageHandle;
    bootContext.ST = SystemTable;

    setupConsole(&bootContext);

    initializePaging(&bootContext);

    loadAndMapKernel(&bootContext);

    setupBootInfo(&bootContext);

    setupGop(&bootContext); 

    prepareKernelJump(&bootContext);

    obtainInitialMemoryMap(&bootContext);

    displayFinalMessage(&bootContext);

    setDisplayMode(&bootContext);

    exitBootServices(&bootContext);
    
    clearEfiSystemTable(&bootContext);

    jumpToKernel(&bootContext);

    return EFI_SUCCESS;
}
