#ifndef PAGING_H_INC
#define PAGING_H_INC

#include <efi.h>
#include <efilib.h>
#include <stdbool.h>
#include <stdint.h>

#define PAGE_SIZE 0x1000

void* allocateZeroedPages(EFI_SYSTEM_TABLE *ST, UINTN numberOfPages);
uint64_t* pagingInit(EFI_SYSTEM_TABLE *ST);
bool memoryMapPages(EFI_SYSTEM_TABLE *ST, uint64_t *pml4, uint64_t paddr, uint64_t vaddr, uint64_t numberOfPages);
uint64_t walkPageTables(uint64_t *pml4, uint64_t vaddr);
void testPaging(EFI_SYSTEM_TABLE *ST, uint64_t *pml4);
#endif