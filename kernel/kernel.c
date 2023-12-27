#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <stdarg.h>

#include "io/serial.h"
#include "io/port_io.h"
#include "io/printf.h"
#include "cpu/interrupts/idt.h"


// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.
int kprintf(char* fmt, ...);

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
static void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

size_t
strlen(const char* str) 
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

struct regs*
div_by_zero_isr(struct regs* r)
{
    kprintf("DIV BY 0\n");

    return r;
}

void
_start(void) {
    idt_init();
    kprintf("BEFORE INTERRUPT\n");
    // asm("int $0x21");
    asm("int $0x80");
    kprintf("AFTER INTERRUPT\n");
    asm volatile ("movl %0, %%eax" : : "r"(0xDEADBEEF));

    // We're done, just hang...
    hcf();
}
