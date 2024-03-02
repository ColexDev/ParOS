#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

struct interrupt_frame
{
   uint64_t segment; /* Segment register (ds) */
   uint64_t r15, r14, r13, r12, r11, r10, r9, r8; /* General purpose regs */
   uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax; /* General purpose regs */
   uint64_t interrupt_number, error_code; /* Pushed by ISR before common handler */
   uint64_t rip, cs, rflags, rsp, ss; /* Pushed by CPU */
};

void register_interrupt_handler(const uint8_t vector, void (*isr)(struct interrupt_frame* frame));

#endif /* INTERRUPTS_H */
