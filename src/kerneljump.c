#include <bootloader/kerneljump.h>
#include <bootloader/paging.h> 
#include <bootloader/console.h>
#include <bootloader/bootloadercfg.h>

extern const char _KernelJumpStart[];
extern const char _KernelJumpEnd[];

EFI_STATUS loadKernelJump(EFI_SYSTEM_TABLE *ST, uint64_t* pml4, uint64_t *kernelJump) 
{
    uint64_t size = _KernelJumpEnd - _KernelJumpStart;
    uint64_t pageCount = getPageCount(size);
    void *ptr = allocateZeroedPages(ST, EfiLoaderCode, pageCount);
    if(!ptr)
        return EFI_LOAD_ERROR;
    for(int i = 0;i < size;i++)
        ((uint8_t *) ptr)[i] = _KernelJumpStart[i];
    *kernelJump = (uint64_t) ptr;
    if(memoryMapPages(ST, pml4, (uint64_t) ptr, (uint64_t) ptr, pageCount))
        return EFI_SUCCESS; 
    return EFI_LOAD_ERROR;
}