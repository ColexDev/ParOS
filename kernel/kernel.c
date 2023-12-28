#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <stdarg.h>

#include "io/serial.h"
#include "io/port_io.h"
#include "io/printf.h"
#include "cpu/interrupts/idt.h"
#include "cpu/gdt/gdt.h"


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

void print_registers() {
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    uint16_t cs, ds, es, fs, gs;

    // Inline assembly to load register values
    asm volatile (
        "movq %%rax, %0\n"
        "movq %%rbx, %1\n"
        "movq %%rcx, %2\n"
        "movq %%rdx, %3\n"
        "movq %%rsi, %4\n"
        "movq %%rdi, %5\n"
        "movq %%rbp, %6\n"
        "movq %%rsp, %7\n"
        "movq %%r8, %8\n"
        "movq %%r9, %9\n"
        "movq %%r10, %10\n"
        "movq %%r11, %11\n"
        "movq %%r12, %12\n"
        "movq %%r13, %13\n"
        "movq %%r14, %14\n"
        "movq %%r15, %15\n"
        "movw %%cs, %16\n"
        "movw %%ds, %17\n"
        "movw %%es, %18\n"
        "movw %%fs, %19\n"
        "movw %%gs, %20\n"
        : "=g"(rax), "=g"(rbx), "=g"(rcx), "=g"(rdx), "=g"(rsi), "=g"(rdi), "=g"(rbp), "=g"(rsp),
          "=g"(r8), "=g"(r9), "=g"(r10), "=g"(r11), "=g"(r12), "=g"(r13), "=g"(r14), "=g"(r15),
          "=g"(cs), "=g"(ds), "=g"(es), "=g"(fs), "=g"(gs)
        );

    // Print register values
    kprintf("RAX: %016lx  RBX: %016lx  RCX: %016lx  RDX: %016lx\n", rax, rbx, rcx, rdx);
    kprintf("RSI: %016lx  RDI: %016lx  RBP: %016lx  RSP: %016lx\n", rsi, rdi, rbp, rsp);
    kprintf("R8 : %016lx  R9 : %016lx  R10: %016lx  R11: %016lx\n", r8, r9, r10, r11);
    kprintf("R12: %016lx  R13: %016lx  R14: %016lx  R15: %016lx\n", r12, r13, r14, r15);
    kprintf("CS : %04x  DS : %04x  ES : %04x  FS : %04x  GS : %04x\n", cs, ds, es, fs, gs);
}


void
_start(void) {
    gdt_init();
    idt_init();
    kprintf("BEFORE INTERRUPT\n");
    print_registers();
    // dump_idt();
    asm ("int $0x00");
    kprintf("AFTER INTERRUPT\n");
    asm volatile ("movl %0, %%eax" : : "r"(0xDEADBEEF));

    // We're done, just hang...
    hcf();
}
