#include <bootloader/print.h>

EFI_STATUS print(EFI_SYSTEM_TABLE *ST) {
    return ST->ConOut->OutputString(ST->ConOut, L"Hello World\r\n");
}