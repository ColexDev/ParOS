#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>

char* itoa(uint64_t value, char *str, size_t base);
void* memset(void *dest, int val, size_t len);
void* memcpy(void *dest, const void *src, size_t len);
// void* kmalloc(size_t size);
size_t strlen(const char* str);
int get_int_len(int value);
int kstrcmp(char s1[], char s2[]);
char* strtok(char* str, const char* delim);
char* kstrcat(char *a, char *b);
int kprintf(char* fmt, ...);
int ksprintf(char *buf, char *fmt, ...);
char* kstrtok(char *str, char *delim);
char* kgets(void);

#endif
