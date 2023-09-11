#ifndef STRING_H_INCL
#define STRING_H_INCL

#define MAX_STRING_LENGTH 250 

#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include <stdbool.h>

bool stringEquals(const char *str1, const char *str2, uint32_t count);
bool isWhitespace(const char c);
void toWidechar(const char *in, CHAR16 *out, uint32_t count);
uint64_t hexadecimalToInt(char *string, uint32_t count);

#endif