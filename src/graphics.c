#include <bootloader/graphics.h>
#include <bootloader/console.h>

EFI_STATUS setGraphicsMode(EFI_SYSTEM_TABLE *ST, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **mode) 
{
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;       
    EFI_STATUS Status = ST->BootServices->LocateProtocol(&gopGuid, NULL, (void **) &gop);
    if(EFI_ERROR(Status)) 
        return EFI_DEVICE_ERROR;
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
        return EFI_DEVICE_ERROR;
    gop->QueryMode(gop, finalMode, &sizeInfo, mode);
    return gop->SetMode(gop, finalMode);
}