#ifndef GRAPHICS_H_INC
#define GRPAHICS_H_INC

#include <efi.h>
#include <efilib.h>
#include <bootloader/bootinfo.h>

EFI_GRAPHICS_OUTPUT_PROTOCOL *getGop(EFI_SYSTEM_TABLE *ST);
UINT32 obtainGraphicsMode(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, FrameBuffer *framebuffer);
void printFrameBufferInfo(EFI_SYSTEM_TABLE *ST, FrameBuffer *framebuffer);

#endif