#include <stdint.h>

#include "interrupts.h"
#include "../../io/printf.h"

void
dump_interrupt_frame(const struct interrupt_frame* frame)
{
    kprintf("Interrupt Frame Dump:\n");
    kprintf("  Segment:            %016lx\n", frame->segment);
    kprintf("  General Purpose Registers:\n");
    kprintf("    RDI:              %016lx\n", frame->rdi);
    kprintf("    RSI:              %016lx\n", frame->rsi);
    kprintf("    RBP:              %016lx\n", frame->rbp);
    kprintf("    RBX:              %016lx\n", frame->rbx);
    kprintf("    RDX:              %016lx\n", frame->rdx);
    kprintf("    RCX:              %016lx\n", frame->rcx);
    kprintf("    RAX:              %016lx\n", frame->rax);
    kprintf("    R8:               %016lx\n", frame->r8);
    kprintf("    R9:               %016lx\n", frame->r9);
    kprintf("    R10:              %016lx\n", frame->r10);
    kprintf("    R11:              %016lx\n", frame->r11);
    kprintf("    R12:              %016lx\n", frame->r12);
    kprintf("    R13:              %016lx\n", frame->r13);
    kprintf("    R14:              %016lx\n", frame->r14);
    kprintf("    R15:              %016lx\n", frame->r15);
    kprintf("  Interrupt Number:   %016lx\n", frame->interrupt_number);
    kprintf("  Error Code:         %016lx\n", frame->error_code);
    kprintf("  RIP:                %016lx\n", frame->rip);
    kprintf("  CS:                 %016lx\n", frame->cs);
    kprintf("  RFLAGS:             %016lx\n", frame->rflags);
    kprintf("  RSP:                %016lx\n", frame->rsp);
    kprintf("  SS:                 %016lx\n", frame->ss);
    kprintf("\n");
}

void
interrupt_handler(struct interrupt_frame* registers)
{
    dump_interrupt_frame(registers);
    // kprintf("INTERRUPT No: %d; ERROR CODE: %d\n", 
            // registers->interrupt_number, registers->error_code);
}
