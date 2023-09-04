#include <bootloader/graphics.h>
#include <bootloader/console.h>

EFI_GRAPHICS_OUTPUT_PROTOCOL *getGop(EFI_SYSTEM_TABLE *ST)
{
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;       
    EFI_STATUS Status = ST->BootServices->LocateProtocol(&gopGuid, NULL, (void **) &gop);
    if(EFI_ERROR(Status)) 
        return NULL;
    return gop;
}

UINT32 obtainGraphicsMode(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, FrameBuffer *framebuffer) 
{
    UINT32 maxMode = gop->Mode->MaxMode;
    UINTN sizeInfo;
    UINT32 finalMode = UINT32_MAX;
    UINT32 vertical, horizontal;
    vertical = horizontal = 0;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    for(int i = 0;i < maxMode;i++) {
        gop->QueryMode(gop, i, &sizeInfo, &info);
        if(info->VerticalResolution > vertical) 
        {
            if(info->HorizontalResolution > horizontal && info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor) 
            {
                vertical = info->VerticalResolution;
                horizontal = info->HorizontalResolution;
                finalMode = i;
            }
        }
    }
    if(finalMode == UINT32_MAX)
        return UINT32_MAX;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *mode;
    gop->QueryMode(gop, finalMode, &sizeInfo, &mode);
    framebuffer->width = mode->HorizontalResolution;
    framebuffer->height = mode->VerticalResolution;
    framebuffer->pitch = mode->PixelsPerScanLine;
    framebuffer->pixel_size = sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
    framebuffer->base = gop->Mode->FrameBufferBase;
    return finalMode;
}

void printFrameBufferInfo(EFI_SYSTEM_TABLE *ST, FrameBuffer *framebuffer)
{
    printString(ST, EFI_WHITE, L"Framebuffer: ");
    printString(ST, EFI_WHITE, L"width: ");
    printIntegerInDecimal(ST, EFI_WHITE, framebuffer->width);
    printString(ST, EFI_WHITE, L" ");
    printString(ST, EFI_WHITE, L"height: ");
    printIntegerInDecimal(ST, EFI_WHITE, framebuffer->height);
    printString(ST, EFI_WHITE, L" ");
    printString(ST, EFI_WHITE, L"pitch: ");
    printIntegerInDecimal(ST, EFI_WHITE, framebuffer->pitch);
    printString(ST, EFI_WHITE, L" ");
    printString(ST, EFI_WHITE, L"pixel_size: ");
    printIntegerInDecimal(ST, EFI_WHITE, framebuffer->pixel_size);
    printString(ST, EFI_WHITE, L" ");
    printString(ST, EFI_WHITE, L"base: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, framebuffer->base);
    newLine(ST);
}