#ifndef GRAPHICS_H_INC
#define GRPAHICS_H_INC

#include <efi.h>
#include <efilib.h>

EFI_STATUS setGraphicsMode(EFI_SYSTEM_TABLE *ST, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **mode);

#endif