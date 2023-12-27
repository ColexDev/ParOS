#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

struct interrupt_frame
{
   uint64_t segment; /* Segment register (ds) */
   uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax; /* General purpose regs */
   uint64_t interrupt_number, error_code; /* Pushed by ISR before common handler */
   uint64_t rip, cs, rflags, rsp, ss; /* Pushed by CPU */
};

void interrupt_handler(struct interrupt_frame* registers);

#endif /* INTERRUPTS_H */
