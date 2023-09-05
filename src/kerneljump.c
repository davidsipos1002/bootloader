#include <bootloader/kerneljump.h>
#include <bootloader/paging.h> 
#include <bootloader/console.h>

extern const char _KernelJumpStart[];
extern const char _KernelJumpEnd[];

EFI_STATUS loadKernelJump(EFI_SYSTEM_TABLE *ST, uint64_t *kernelJump) 
{
    uint64_t size = _KernelJumpEnd - _KernelJumpStart;
    uint64_t pageCount = size / 0x1000 + (size % 0x1000 != 0);
    void *ptr = allocateZeroedPages(ST, pageCount);
    if(!ptr)
        return EFI_LOAD_ERROR;
    for(int i = 0;i < size;i++)
        ((uint8_t *) ptr)[i] = _KernelJumpStart[i];
    *kernelJump = (uint64_t) ptr;
    return EFI_SUCCESS; 
}