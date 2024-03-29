#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
void* memmove(void* dest, const void* src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
size_t strlen(const char* str);
int strncmp(const char* s1, const char* s2, size_t n);

#define ALIGN_UP(value, alignment) (((value) + ((alignment) - 1)) & ~((alignment) - 1))
#define ALIGN_DOWN(value, alignment) ((value) & ~((alignment) - 1))

#endif /* STRING_H */
