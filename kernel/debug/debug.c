#include <stdint.h>

#include <io/printf.h>

#include "debug.h"

void
print_registers(void)
{
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    uint16_t cs, ds, es, fs, gs;

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

    kprintf("RAX: %016lx  RBX: %016lx  RCX: %016lx  RDX: %016lx\n", rax, rbx, rcx, rdx);
    kprintf("RSI: %016lx  RDI: %016lx  RBP: %016lx  RSP: %016lx\n", rsi, rdi, rbp, rsp);
    kprintf("R8 : %016lx  R9 : %016lx  R10: %016lx  R11: %016lx\n", r8, r9, r10, r11);
    kprintf("R12: %016lx  R13: %016lx  R14: %016lx  R15: %016lx\n", r12, r13, r14, r15);
    kprintf("CS : %04x  DS : %04x  ES : %04x  FS : %04x  GS : %04x\n", cs, ds, es, fs, gs);
}

void
print_stack_trace(void)
{
    struct stack_frame* stack;

    /* Get current rbp */
    asm volatile ("movq %%rbp, %0" : "=r"(stack));

    uint64_t i = 0;
    while (stack->prev_rbp != 0) {
        kprintf("#%d | RIP: %lx\t RBP: %lx\n", i, stack->return_addr, stack->prev_rbp);
        stack = stack->prev_rbp;
        i++;
    }
}

