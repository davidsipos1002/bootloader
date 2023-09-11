#include <bootloader/memory.h>

void memset(void *addr, const char value, uint64_t count) 
{
    char *buff = (char *) addr;
    for(uint64_t i = 0;i < count;i++)
        buff[i] = value;
} 

void memcpy(void *dest, void *src, uint64_t count)
{
    char *source = (char *) src;
    char *destination = (char *) dest;
    for(uint64_t i = 0;i < count;i++)
        destination[i] = source[i];
}

void* alloc(EFI_SYSTEM_TABLE *ST, UINTN size)
{
    void *buffer;
    if(EFI_ERROR(ST->BootServices->AllocatePool(EfiLoaderData, size, (void **) &buffer)))
        return (void *) NULL;
    return buffer;
}

void* realloc(EFI_SYSTEM_TABLE *ST, UINTN size, UINTN new_size, void *buffer)
{
    void *new_buffer = alloc(ST, new_size);
    if(!new_buffer)
        return (void *) NULL;
    memcpy(new_buffer, buffer, size);
    free(ST, buffer);
    return new_buffer;
}

bool free(EFI_SYSTEM_TABLE *ST, void *buffer)
{
    if(EFI_ERROR(ST->BootServices->FreePool(buffer)))
        return false;
    return true;
}