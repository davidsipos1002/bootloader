#ifndef CONSOLE_H_INCL
#define CONSOLE_H_INCL

#include <efi.h>
#include <efilib.h>
#define newLine(ST) printString(ST, EFI_WHITE, L"\r\n")

EFI_STATUS setConsoleMode(EFI_SYSTEM_TABLE *ST);
EFI_STATUS clearScreen(EFI_SYSTEM_TABLE *ST);
EFI_STATUS printString(EFI_SYSTEM_TABLE *ST, UINTN COLOR, CHAR16 *String);
EFI_STATUS printIntegerInDecimal(EFI_SYSTEM_TABLE *ST, UINTN Color, UINTN Integer);
EFI_STATUS printIntegerInHexadecimal(EFI_SYSTEM_TABLE *ST, UINTN Color, UINTN Integer);
EFI_STATUS waitForKeyPress(EFI_SYSTEM_TABLE *ST);

#endif