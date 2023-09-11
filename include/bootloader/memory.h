#ifndef MEMORY_H_INCL
#define MEMORY_H_INCL

#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include <stdbool.h>

void memset(void *addr, const char value, uint64_t count);
void memcpy(void *dest, void *src, uint64_t count);
void* alloc(EFI_SYSTEM_TABLE *ST, UINTN size);
void* realloc(EFI_SYSTEM_TABLE *ST, UINTN size, UINTN new_size, void *buffer);
bool free(EFI_SYSTEM_TABLE *ST, void *buffer);

#endif