#include <efi.h>
#include <efilib.h>
#include <bootloader/graphics.h>
#include <bootloader/console.h>
#include <bootloader/paging.h>
#include <bootloader/elfloader.h>
#include <bootloader/memorymap.h>
#include <bootloader/kerneljump.h>
#include <bootloader/version.h>
#include <bootloader/bootcontext.h>
#include <bootloader/filesystem.h>
#include <bootloader/config.h>
#include <bootloader/memory.h>

EFI_STATUS die(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST, CHAR16 *Message)
{
    printString(ST, EFI_RED, Message);
    waitForKeyPress(ST);
    ST->BootServices->Exit(ImageHandle, EFI_LOAD_ERROR, 0, NULL);
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

void obtainBootConfiguration(BootContext *bootContext)
{
    BootConfig *bootConfig = parseConfigurationFile(bootContext->ST, bootContext->rootDirectory);
    if(bootConfig == NULL)
        die(bootContext->ImageHandle, bootContext->ST, L"Could not parse boot configuration file\r\n");
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_GREEN, L"Boot configuration file successfully parsed\r\n");
    #endif
    #ifdef PRINT_CONFIG
        printString(bootContext->ST, EFI_GREEN, L"Chosen boot configuration\r\n");
        printBootConfig(bootContext->ST, bootConfig);
    #endif
    bootContext->bootConfig = bootConfig;
    bootContext->bootInfoAddress = bootConfig->bootInfoVirtualAddress;
}

void initializePaging(BootContext *bootContext)
{
    uint64_t *pml4 = pagingInit(bootContext->ST);
    if(!pml4) 
        die(bootContext->ImageHandle, bootContext->ST, L"Could not initialize paging\r\n");
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_GREEN, L"Recursive paging initialized\r\n");
        printString(bootContext->ST, EFI_GREEN, L"PML4 address ");
        printIntegerInHexadecimal(bootContext->ST, EFI_GREEN, (uint64_t) pml4);
        newLine(bootContext->ST);
    #endif
    bootContext->pml4 = pml4;
    if (!memoryMapPages(bootContext->ST, pml4, 0, 0, 1))
        die(bootContext->ImageHandle, bootContext->ST, L"Could not memory map scratchpad\r\n");
}

void loadAndMapKernel(BootContext *bootContext)
{
    CHAR16 *kernelPath = alloc(bootContext->ST, bootContext->bootConfig->kernelPathLength * sizeof(CHAR16));
    toWidechar(bootContext->bootConfig->kernelPath, kernelPath, bootContext->bootConfig->kernelPathLength);
    uint64_t kernelEntry;
    if(EFI_ERROR(loadKernel(bootContext->ST, bootContext->rootDirectory, kernelPath, bootContext->pml4, &kernelEntry)))
        die(bootContext->ImageHandle, bootContext->ST, L"Could not load kernel\r\n");
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_GREEN, L"Kernel succesfully loaded and mapped at ");
        printIntegerInHexadecimal(bootContext->ST, EFI_GREEN, kernelEntry);
        newLine(bootContext->ST);
    #endif
    free(bootContext->ST, kernelPath);
    bootContext->kernelEntry = kernelEntry;
}

void loadInitialDataFiles(BootContext *bootContext)
{
    InitialDataFile *currentFile;
    for(uint32_t i = 0;i < bootContext->bootConfig->dataFileCount;i++)
    {
        currentFile = &(bootContext->bootConfig->dataFiles[i]);
        CHAR16 *filePath = alloc(bootContext->ST, currentFile->filePathLength * sizeof(CHAR16));
        toWidechar(currentFile->filePath, filePath, currentFile->filePathLength);
        uint64_t pageCount;
        uint64_t filePhysicalAddress = loadFileToMemory(bootContext->ST, bootContext->rootDirectory, filePath, &pageCount);
        if(!filePhysicalAddress)
            die(bootContext->ImageHandle, bootContext->ST, L"Could not load initial data file\r\n");
        if(!memoryMapPages(bootContext->ST, bootContext->pml4, filePhysicalAddress, currentFile->loadVirtualAddress, pageCount))
            die(bootContext->ImageHandle, bootContext->ST, L"Could not memory map initial data file\r\n");
        #ifdef BASIC_LOGGING
            printString(bootContext->ST, EFI_GREEN, L"File ");
            printString(bootContext->ST, EFI_GREEN, filePath);
            printString(bootContext->ST, EFI_GREEN, L" loaded and mapped at ");
            printIntegerInHexadecimal(bootContext->ST, EFI_GREEN, currentFile->loadVirtualAddress);
            newLine(bootContext->ST);
        #endif
        free(bootContext->ST, filePath);
    }
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_GREEN, L"Initial data files loaded and mapped successfully\r\n");
    #endif
}

void setupBootInfo(BootContext *bootContext)
{
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_WHITE, L"BootInfo size: ");
        printIntegerInDecimal(bootContext->ST, EFI_WHITE, sizeof(BootInfo));
        newLine(bootContext->ST);
    #endif
    BootInfo *bootInfo = allocateZeroedPages(bootContext->ST, EfiReservedMemoryType, getPageCount(sizeof(BootInfo)));
    if(bootInfo == NULL || !memoryMapPages(bootContext->ST, bootContext->pml4, (uint64_t) bootInfo, bootContext->bootConfig->bootInfoVirtualAddress, 1))
        die(bootContext->ImageHandle, bootContext->ST, L"Could not allocate bootinfo\r\n");
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_GREEN, L"BootInfo successfully allocated and mapped at ");
        printIntegerInHexadecimal(bootContext->ST, EFI_GREEN, bootContext->bootConfig->bootInfoVirtualAddress);
        newLine(bootContext->ST);
    #endif
    bootContext->bootInfo = bootInfo;
}

void setupGop(BootContext *bootContext)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = getGop(bootContext->ST);
    if(gop == NULL)
        die(bootContext->ImageHandle, bootContext->ST, L"Could not get gop\r\n");
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_GREEN, L"Obtained gop\r\n");
    #endif
    UINT32 gopMode = obtainGraphicsMode(gop, &(bootContext->bootInfo->framebuffer));
    if(gopMode == UINT32_MAX)
        die(bootContext->ImageHandle, bootContext->ST, L"Could not get gop mode\r\n");
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
        die(bootContext->ImageHandle, bootContext->ST, L"Could not load KernelJump\r\n");
    #ifdef BASIC_LOGGING
        printString(bootContext->ST, EFI_GREEN, L"KernelJump successfully loaded and mapped at ");
        printIntegerInHexadecimal(bootContext->ST, EFI_GREEN, kernelJump);
        newLine(bootContext->ST);
    #endif
    bootContext->kernelJumpAddress = kernelJump;
}

void obtainInitialMemoryMap(BootContext *bootContext)
{
    UINTN MapKey = getMemoryMap(bootContext->ST, &(bootContext->bootInfo->memorymap));
    if(MapKey == UINT64_MAX)
        die(bootContext->ImageHandle, bootContext->ST, L"Could not get initial memory map\r\n");
    bootContext->MemoryMapKey = MapKey;
    #ifdef BASIC_LOGGING
        printMemoryMapInfo(bootContext->ST, &(bootContext->bootInfo->memorymap));
        printString(bootContext->ST, EFI_GREEN, L"Memory map successfully obtained\r\n");
    #endif
    #ifdef PRINT_MEMORY_MAP
        printMemoryMap(bootContext->ST, &(bootContext->bootInfo->memorymap));
    #endif
}

void displayFinalMessage(BootContext *bootContext)
{
    printString(bootContext->ST, EFI_YELLOW, L"Press any key to jump to kernel...\r\n");
    waitForKeyPress(bootContext->ST);
}

void setDisplayMode(BootContext *bootContext)
{
    if(EFI_ERROR(bootContext->gop->SetMode(bootContext->gop, bootContext->gopMode)))
        die(bootContext->ImageHandle, bootContext->ST, L"Could not set gop mode\r\n");
}

void exitBootServices(BootContext *bootContext)
{
    free(bootContext->ST, bootContext->bootConfig);
    closeFileHandle(bootContext->rootDirectory);
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
    jump(bootContext->bootInfoAddress, (uint64_t) bootContext->pml4, bootContext->kernelEntry);
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, 0);

    BootContext bootContext;
    bootContext.ImageHandle = ImageHandle;
    bootContext.ST = SystemTable;
    bootContext.rootDirectory = getRootDirectory(ImageHandle, SystemTable);

    setupConsole(&bootContext);

    obtainBootConfiguration(&bootContext);

    initializePaging(&bootContext);

    loadAndMapKernel(&bootContext);

    loadInitialDataFiles(&bootContext);

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
