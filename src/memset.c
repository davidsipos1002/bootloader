#include <bootloader/memset.h>

void memset(void *addr, const char value, uint64_t count) {
    char *buff = (char *) addr;
    for(uint64_t i = 0;i < count;i++)
        buff[i] = value;
} 