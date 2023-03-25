#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "../stdlib/util.h"
#include "../drivers/tty.h"

extern uint32_t curr_free_mem;

size_t
strlen(const char* str) 
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

char*
itoa(uint64_t value, char *str, size_t base) {
    memset(str, 0, strlen(str));
    char *ptr = str;

    do {
        size_t mod = value % base;
        unsigned char start = '0';
        if ((base == 16) && (mod > 9)) {
            start = 'a';
            mod -= 10;
        }
        *ptr++ = start + mod;
    } while ((value /= base) > 0);
    *ptr = '\0';

    size_t len = strlen(str);

    for (int i = 0; i < len / 2; i++) {
        char c = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = c;
    }

    return str;
}

void*
memcpy(void *dest, const void *src, size_t len)
{
    char *d = dest;
    const char *s = src;
    while (len--)
        *d++ = *s++;
    return dest;
}

void*
memset(void *dest, int val, size_t len)
{
    unsigned char *ptr = dest;
    while (len-- > 0)
        *ptr++ = val;
    return dest;
}

char* 
kstrcat(char *a, char *b) {
    while (*a++);
    a--;
    while (*a++ = *b++);
    return a;
}

void*
kmalloc(size_t size)
{
    void* address;

    memset((void*)curr_free_mem, 0, size);
    address = (void*)curr_free_mem;
    curr_free_mem += size;

    return address;
}

int
get_int_len(int value)
{
    int l = 1;

    while (value > 9) {
        l++;
        value/=10;
    }

    return l;
}

int
kstrcmp(char s1[], char s2[])
{
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

void
kprintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == 'd' || *format == 'i') {
                int value = va_arg(args, int);
                char buffer[32] = {0};
                itoa(value, buffer, 10);
                puts(buffer);
            } else if (*format == 'u') {
                unsigned int value = va_arg(args, unsigned int);
                char buffer[32] = {0};
                itoa(value, buffer, 10);
                puts(buffer);
            } else if (*format == 'x' || *format == 'X') {
                unsigned int value = va_arg(args, unsigned int);
                char buffer[32] = {0};
                itoa(value, buffer, 16);
                puts(buffer);
            } else if (*format == 's') {
                char* str = va_arg(args, char*);
                puts(str);
            } else if (*format == 'c') {
                char c = (char)va_arg(args, int);
                putch(c);
            } else if (*format == 'p') {
                unsigned long value = va_arg(args, unsigned long);
                char buffer[32] = {0};
                itoa(value, buffer, 16);
                puts("0x");
                puts(buffer);
            } else {
                putch('%');
                putch(*format);
            }
        } else {
            putch(*format);
        }
        format++;
    }

    va_end(args);
}

