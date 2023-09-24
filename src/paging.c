#include <bootloader/paging.h>
#include <bootloader/memory.h>
#include <bootloader/console.h>

#define PML4_INDEX_MASK  0x00000FF8000000000 
#define PML4_INDEX_SHIFT 39
#define PDP_INDEX_MASK   0x00000007FC0000000
#define PDP_INDEX_SHIFT  30
#define PD_INDEX_MASK    0x0000000003FE00000
#define PD_INDEX_SHIFT   21
#define PT_INDEX_MASK    0x000000000001FF000
#define PT_INDEX_SHIFT   12
#define PAGE_OFFSET_MASK 0x00000000000000FFF

void* allocateZeroedPages(EFI_SYSTEM_TABLE *ST, EFI_MEMORY_TYPE memoryType, UINTN numberOfPages) 
{
    EFI_PHYSICAL_ADDRESS ret;
    EFI_STATUS Status = ST->BootServices->AllocatePages(AllocateAnyPages, memoryType, numberOfPages, &ret);
    if(EFI_ERROR(Status))
        return NULL;
    memset((void *) ret, 0, numberOfPages * PAGE_SIZE);
    return (void *) ret;
}

uint64_t* pagingInit(EFI_SYSTEM_TABLE *ST) {
    uint64_t *ret = (uint64_t *) allocateZeroedPages(ST, EfiReservedMemoryType, 1);
    if(ret == NULL)
        return false;
    ret[511] = (uint64_t) ret | 0x3;
    return ret;
}

bool memoryMapPage(EFI_SYSTEM_TABLE *ST, uint64_t *pml4, uint64_t paddr, uint64_t vaddr) 
{
    uint64_t i4 = (vaddr & PML4_INDEX_MASK) >> PML4_INDEX_SHIFT;
    uint64_t *pdp;
    if(!pml4[i4]) {
        pdp = allocateZeroedPages(ST, EfiReservedMemoryType, 1);
        #ifdef EXTENSIVE_LOGGING
            printString(ST, EFI_WHITE, L"Allocating new Page Directory Pointer Table at ");
            printIntegerInHexadecimal(ST, EFI_WHITE, (uint64_t) pdp);
            newLine(ST);
        #endif
        if(!pdp)
            return false;
        pml4[i4] = (uint64_t) pdp | 0x3;
    } else
        pdp = (uint64_t *) (pml4[i4] & (~0x3));

    uint64_t i3 = (vaddr & PDP_INDEX_MASK) >> PDP_INDEX_SHIFT;
    uint64_t *pd;
    if(!pdp[i3]) {
        pd = allocateZeroedPages(ST, EfiReservedMemoryType, 1);
        #ifdef EXTENSIVE_LOGGING
            printString(ST, EFI_WHITE, L"Allocating new Page Directory Table at ");
            printIntegerInHexadecimal(ST, EFI_WHITE, (uint64_t) pd);
            newLine(ST);
        #endif
        if(!pd)
            return false;
        pdp[i3] = (uint64_t) pd | 0x3;
    } else
        pd = (uint64_t *) (pdp[i3] & (~0x3));

    uint64_t i2 = (vaddr & PD_INDEX_MASK) >> PD_INDEX_SHIFT;
    uint64_t *pt;
    if(!pd[i2]) {
        pt = allocateZeroedPages(ST, EfiReservedMemoryType, 1);
        #ifdef EXTENSIVE_LOGGING
            printString(ST, EFI_WHITE, L"Allocating new Page Table at ");
            printIntegerInHexadecimal(ST, EFI_WHITE, (uint64_t) pt);
            newLine(ST);
        #endif
        if(!pt)
            return false;
        pd[i2] = (uint64_t) pt | 0x3;
    } else
        pt = (uint64_t *) (pd[i2] & (~0x3));
    
    uint64_t i1 = (vaddr & PT_INDEX_MASK) >> PT_INDEX_SHIFT;
    pt[i1] = paddr | 0x3;
    return true;
}

bool memoryMapPages(EFI_SYSTEM_TABLE *ST, uint64_t *pml4, uint64_t paddr, uint64_t vaddr, uint64_t numberOfPages) 
{
    for(uint64_t i = 0;i < numberOfPages;i++) {
        #ifdef EXTENSIVE_LOGGING
            printString(ST, EFI_WHITE, L"Mapping ");
            printIntegerInHexadecimal(ST, EFI_WHITE, paddr);
            printString(ST, EFI_WHITE, L" to ");
            printIntegerInHexadecimal(ST, EFI_WHITE, vaddr);
            newLine(ST);
        #endif
        if(!memoryMapPage(ST, pml4, paddr, vaddr))
            return false;
        paddr += PAGE_SIZE;
        vaddr += PAGE_SIZE;
    }
    return true;
}

uint64_t walkPageTables(uint64_t *pml4, uint64_t vaddr) 
{
    uint64_t i4 = (vaddr & PML4_INDEX_MASK) >> PML4_INDEX_SHIFT;
    uint64_t i3 = (vaddr & PDP_INDEX_MASK) >> PDP_INDEX_SHIFT;
    uint64_t i2 = (vaddr & PD_INDEX_MASK) >> PD_INDEX_SHIFT;
    uint64_t i1 = (vaddr & PT_INDEX_MASK) >> PT_INDEX_SHIFT;

    uint64_t *addr = (uint64_t *) (pml4[i4] & (~0x3));
    addr = (uint64_t *) (addr[i3] & (~0x3));
    addr = (uint64_t *) (addr[i2] & (~0x3));
    addr = (uint64_t *) (addr[i1] & (~0x3));
    uint64_t ret = (uint64_t) addr;
    ret |= vaddr & 0xFFF;
    return ret;
}

uint64_t getPageCount(uint64_t size)
{
    return size / PAGE_SIZE + (size % PAGE_SIZE != 0);
}

#ifdef DEBUG_BUILD
void testPaging(EFI_SYSTEM_TABLE *ST, uint64_t *pml4) 
{
    printString(ST, EFI_GREEN, L"PML4 ADDRESS ");
    printIntegerInHexadecimal(ST, EFI_GREEN, (uint64_t) pml4);
    newLine(ST);
    memoryMapPages(ST, pml4, 0xBAC000, 0xD2000, 10);
    newLine(ST);
    memoryMapPages(ST, pml4, 0xC000, 0xFFFFA000, 5);
    newLine(ST);
    printString(ST, EFI_WHITE, L"Walking page tables\r\n");
    newLine(ST);
    uint64_t paddr = walkPageTables(pml4, 0xDA123);
    printString(ST, EFI_WHITE, L"0xDA123 maps to ");
    printIntegerInHexadecimal(ST, EFI_WHITE, paddr);
    newLine(ST);
    paddr = walkPageTables(pml4, 0xFFFFC5DA);
    printString(ST, EFI_WHITE, L"0xFFFFC5DA maps to ");
    printIntegerInHexadecimal(ST, EFI_WHITE, paddr);
    newLine(ST);
}
#endif