#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

char* itoa(int val, int base);
void* memset(void *dest, int val, size_t len);
void* memcpy(void *dest, const void *src, size_t len);
void* kmalloc(size_t size);
size_t strlen(const char* str);
int get_int_len(int value);

#endif
