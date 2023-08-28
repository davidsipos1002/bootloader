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
    if(!pml4) {
        printString(ST, EFI_RED, L"Could not initialize paging\r\n");
        return WaitForKeyPress(ST);
    }
    if(EFI_ERROR(loadKernel(ST, ImageHandle, pml4)))
        printString(ST, EFI_RED, L"Could not load kernel\r\n");
    WaitForKeyPress(ST);
    return EFI_SUCCESS;
}
