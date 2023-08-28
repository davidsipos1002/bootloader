#ifndef PAGING_H_INC
#define PAGING_H_INC

#include <efi.h>
#include <efilib.h>
#include <stdbool.h>
#include <stdint.h>

#define PAGE_SIZE 0x1000

void* allocateZeroedPages(EFI_SYSTEM_TABLE *ST, UINTN numberOfPages);
uint64_t* pagingInit(EFI_SYSTEM_TABLE *ST);

#endif