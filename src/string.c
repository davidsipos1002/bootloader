#include <bootloader/string.h>

bool stringEquals(const char *str1, const char *str2, uint32_t count)
{
    for(uint32_t i = 0;i < count;i++)
        if(str1[i] != str2[i])
            return false;
    return true;
}

bool isWhitespace(const char c)
{
    return c == 0x20 || c == 0x09 || c == 0x0D || c == 0x0A; 
}

void toWidechar(const char *in, CHAR16 *out, uint32_t count)
{
    for(uint32_t i = 0;i < count;i++)
        out[i] = in[i];
}

uint64_t hexadecimalToInt(char *string, uint32_t count)
{
    uint64_t ret = 0;
    uint64_t digit = 1;
    for(uint32_t i = 0;i < count;i++)
        if('A' <= string[i] && string[i] <= 'Z')
            string[i] ^= 0x20;
    for(uint32_t i = count - 1;i >= 0;i--)
    {
        if(string[i] == 'x')
            return ret; 
        if('0' <= string[i] && string[i] <= '9')
            ret += digit * (string[i] - '0');
        else if('a' <= string[i] && string[i] <= 'f')
            ret += digit * (string[i] - 'a' + 10);
        digit <<= 4;
    }
    return ret;
}