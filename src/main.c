#include <efi.h>
#include <efilib.h>
#include <bootloader/graphics.h>
#include <bootloader/console.h>
#include <bootloader/filesystem.h>
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
    if(pagingInit(ST) == NULL) {
        printString(ST, EFI_RED, L"Could not initialize paging\r\n");
        return WaitForKeyPress(ST);
    }
    EFI_FILE_HANDLE rootDirectory = getRootDirectory(ImageHandle, ST);
    if(rootDirectory == NULL) 
    {
        printString(ST, EFI_RED, L"Failed to open root directory\r\n");
        return WaitForKeyPress(ST);
    }
    EFI_FILE_HANDLE kernelImage;
    Status = openKernelImage(rootDirectory, &kernelImage);
    if(Status != EFI_SUCCESS) 
    {
        printString(ST, EFI_RED, L"Kernel binary could not be opened\r\n");
        return WaitForKeyPress(ST);
    }
    loadElf(ST, kernelImage);
    rootDirectory->Close(rootDirectory);
    WaitForKeyPress(ST);
    return EFI_SUCCESS;
}
