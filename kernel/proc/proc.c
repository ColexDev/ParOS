#include <stdint.h>

#include "proc.h"

void
fill_pcb_regs(struct pcb* pcb)
{
    asm("movl %%eax, %0" : "=r" (pcb->eax));
    asm("movl %%ebx, %0" : "=r" (pcb->ebx));
    asm("movl %%ecx, %0" : "=r" (pcb->ecx));
    asm("movl %%edx, %0" : "=r" (pcb->edx));
    asm("movl %%esi, %0" : "=r" (pcb->esi));
    asm("movl %%edi, %0" : "=r" (pcb->edi));
    asm("movl %%ebp, %0" : "=r" (pcb->ebp));
}
