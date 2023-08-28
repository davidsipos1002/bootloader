#include <bootloader/paging.h>
#include <bootloader/memset.h>
#include <bootloader/console.h>

void* allocateZeroedPages(EFI_SYSTEM_TABLE *ST, UINTN numberOfPages) {
    EFI_PHYSICAL_ADDRESS ret;
    EFI_STATUS Status = ST->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, numberOfPages, &ret);
    if(EFI_ERROR(Status))
        return NULL;
    memset((void *) ret, 0, numberOfPages * PAGE_SIZE);
    return (void *) ret;
}

uint64_t* pagingInit(EFI_SYSTEM_TABLE *ST) {
    uint64_t *ret = (uint64_t *) allocateZeroedPages(ST, 1);
    if(ret == NULL)
        return false;
    ret[511] = (uint64_t) ret | 0x3;
    return ret;
}

