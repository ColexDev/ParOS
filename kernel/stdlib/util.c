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
kstrcat(char *dest, char *src) {
    char *rdest = dest;

    while (*dest)
      dest++;
    while (*dest++ = *src++);
    return rdest;
}

// void*
// kmalloc(size_t size)
// {
//     void* address;
//
//     memset((void*)curr_free_mem, 0, size);
//     address = (void*)curr_free_mem;
//     curr_free_mem += size;
//
//     return address;
// }

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

char*
strtok(char* str, const char* delim)
{
    static char* current_str = NULL;
    char* token;
    const char* d;
    uint8_t found_delim = 0;

    if (str != NULL) {
        current_str = str;
    }

    token = current_str;

    while (*current_str) {
        d = delim;

        while (*d) {
            if (*current_str == *d) {
                *current_str++ = '\0';
                found_delim = 1;

                break;
            }

            d++;
        }

        if (found_delim) {
            break;
        }

        current_str++;
    }

    if (found_delim && *token != '\0') {
        return token;
    }

    return NULL;
}

/* Taken from https://github.com/mpredfearn/simple-printf */
static void
simple_outputch(char **str, char c)
{
    if (str) {
        **str = c;
        ++(*str);
    } else {
        putch(c);
    }
}

enum flags {
    PAD_ZERO	= 1,
    PAD_RIGHT	= 2,
};

static int
prints(char **out, const char *string, int width, int flags)
{
    int pc = 0, padchar = ' ';

    if (width > 0) {
        int len = 0;
        const char *ptr;
        for (ptr = string; *ptr; ++ptr) ++len;
        if (len >= width) width = 0;
        else width -= len;
        if (flags & PAD_ZERO)
            padchar = '0';
    }
    if (!(flags & PAD_RIGHT)) {
        for ( ; width > 0; --width) {
            simple_outputch(out, padchar);
            ++pc;
        }
    }
    for ( ; *string ; ++string) {
        simple_outputch(out, *string);
        ++pc;
    }
    for ( ; width > 0; --width) {
        simple_outputch(out, padchar);
        ++pc;
    }

    return pc;
}

#define PRINT_BUF_LEN 64

static int
simple_outputi(char **out, long long i, int base, int sign, int width, int flags, int letbase)
{
    char print_buf[PRINT_BUF_LEN];
    char *s;
    int t, neg = 0, pc = 0;
    unsigned long long u = i;

    if (i == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return prints(out, print_buf, width, flags);
    }

    if (sign && base == 10 && i < 0) {
        neg = 1;
        u = -i;
    }

    s = print_buf + PRINT_BUF_LEN-1;
    *s = '\0';

    while (u) {
        t = u % base;
        if( t >= 10 )
            t += letbase - '0' - 10;
        *--s = t + '0';
        u /= base;
    }

    if (neg) {
        if( width && (flags & PAD_ZERO) ) {
            simple_outputch (out, '-');
            ++pc;
            --width;
        } else {
            *--s = '-';
        }
    }

    return pc + prints (out, s, width, flags);
}


static int
simple_vsprintf(char **out, char *format, va_list ap)
{
    int width, flags;
    int pc = 0;
    char scr[2];
    union {
        char c;
        char *s;
        int i;
        unsigned int u;
        long li;
        unsigned long lu;
        long long lli;
        unsigned long long llu;
        short hi;
        unsigned short hu;
        signed char hhi;
        unsigned char hhu;
        void *p;
    } u;

    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            width = flags = 0;
            if (*format == '\0')
                break;
            if (*format == '%')
                goto out;
            if (*format == '-') {
                ++format;
                flags = PAD_RIGHT;
            }
            while (*format == '0') {
                ++format;
                flags |= PAD_ZERO;
            }
            if (*format == '*') {
                width = va_arg(ap, int);
                format++;
            } else {
                for ( ; *format >= '0' && *format <= '9'; ++format) {
                    width *= 10;
                    width += *format - '0';
                }
            }
            switch (*format) {
                case('d'):
                    u.i = va_arg(ap, int);
                    pc += simple_outputi(out, u.i, 10, 1, width, flags, 'a');
                    break;

                case('u'):
                    u.u = va_arg(ap, unsigned int);
                    pc += simple_outputi(out, u.u, 10, 0, width, flags, 'a');
                    break;

                case('x'):
                    u.u = va_arg(ap, unsigned int);
                    pc += simple_outputi(out, u.u, 16, 0, width, flags, 'a');
                    break;

                case('X'):
                    u.u = va_arg(ap, unsigned int);
                    pc += simple_outputi(out, u.u, 16, 0, width, flags, 'A');
                    break;

                case('c'):
                    u.c = va_arg(ap, int);
                    scr[0] = u.c;
                    scr[1] = '\0';
                    pc += prints(out, scr, width, flags);
                    break;

                case('s'):
                    u.s = va_arg(ap, char *);
                    // pc += prints(out, u.s ? u.s : "(null)", width, flags);
                    pc += prints(out, u.s, width, flags);
                    break;
                case('l'):
                    ++format;
                    switch (*format) {
                        case('d'):
                            u.li = va_arg(ap, long);
                            pc += simple_outputi(out, u.li, 10, 1, width, flags, 'a');
                            break;

                        case('u'):
                            u.lu = va_arg(ap, unsigned long);
                            pc += simple_outputi(out, u.lu, 10, 0, width, flags, 'a');
                            break;

                        case('x'):
                            u.lu = va_arg(ap, unsigned long);
                            pc += simple_outputi(out, u.lu, 16, 0, width, flags, 'a');
                            break;

                        case('X'):
                            u.lu = va_arg(ap, unsigned long);
                            pc += simple_outputi(out, u.lu, 16, 0, width, flags, 'A');
                            break;

                        case('l'):
                            ++format;
                            switch (*format) {
                                case('d'):
                                    u.lli = va_arg(ap, long long);
                                    pc += simple_outputi(out, u.lli, 10, 1, width, flags, 'a');
                                    break;

                                case('u'):
                                    u.llu = va_arg(ap, unsigned long long);
                                    pc += simple_outputi(out, u.llu, 10, 0, width, flags, 'a');
                                    break;

                                case('x'):
                                    u.llu = va_arg(ap, unsigned long long);
                                    pc += simple_outputi(out, u.llu, 16, 0, width, flags, 'a');
                                    break;

                                case('X'):
                                    u.llu = va_arg(ap, unsigned long long);
                                    pc += simple_outputi(out, u.llu, 16, 0, width, flags, 'A');
                                    break;

                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case('h'):
                    ++format;
                    switch (*format) {
                        case('d'):
                            u.hi = va_arg(ap, int);
                            pc += simple_outputi(out, u.hi, 10, 1, width, flags, 'a');
                            break;

                        case('u'):
                            u.hu = va_arg(ap, unsigned int);
                            pc += simple_outputi(out, u.lli, 10, 0, width, flags, 'a');
                            break;

                        case('x'):
                            u.hu = va_arg(ap, unsigned int);
                            pc += simple_outputi(out, u.lli, 16, 0, width, flags, 'a');
                            break;

                        case('X'):
                            u.hu = va_arg(ap, unsigned int);
                            pc += simple_outputi(out, u.lli, 16, 0, width, flags, 'A');
                            break;

                        case('h'):
                            ++format;
                            switch (*format) {
                                case('d'):
                                    u.hhi = va_arg(ap, int);
                                    pc += simple_outputi(out, u.hhi, 10, 1, width, flags, 'a');
                                    break;

                                case('u'):
                                    u.hhu = va_arg(ap, unsigned int);
                                    pc += simple_outputi(out, u.lli, 10, 0, width, flags, 'a');
                                    break;

                                case('x'):
                                    u.hhu = va_arg(ap, unsigned int);
                                    pc += simple_outputi(out, u.lli, 16, 0, width, flags, 'a');
                                    break;

                                case('X'):
                                    u.hhu = va_arg(ap, unsigned int);
                                    pc += simple_outputi(out, u.lli, 16, 0, width, flags, 'A');
                                    break;

                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        } else {
        out:
            simple_outputch (out, *format);
            ++pc;
        }
    }
    if (out) **out = '\0';
    return pc;
}

int
kprintf(char* fmt, ...)
{
    va_list ap;
    int r;

    va_start(ap, fmt);
    r = simple_vsprintf(NULL, fmt, ap);
    va_end(ap);

    return r;
}

int
ksprintf(char *buf, char *fmt, ...)
{
    va_list ap;
    int r;

    va_start(ap, fmt);
    r = simple_vsprintf(&buf, fmt, ap);
    va_end(ap);

    return r;
}
