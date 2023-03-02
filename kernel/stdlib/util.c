#include <stddef.h>
#include <stdint.h>

extern uint32_t curr_free_mem;

/* https://stackoverflow.com/questions/3982320/convert-integer-to-string-without-access-to-libraries */
char*
itoa(int val, int base) {

    static char buf[32] = {0};

    int i = 30;

    for(; val && i ; --i, val /= base)
        buf[i] = "0123456789abcdef"[val % base];

    return &buf[i+1];
}

size_t
strlen(const char* str) 
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
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

    while (value > 9){
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
