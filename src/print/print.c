#include <bootloader/print.h>

EFI_STATUS print(EFI_SYSTEM_TABLE *ST) {
    return ST->ConOut->OutputString(ST->ConOut, L"Hello World, I am David's custom bootloader\r\n");
}