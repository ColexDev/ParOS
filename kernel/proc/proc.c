#include <stdint.h>

#include "proc.h"
#include "../mm/kheap.h"
#include "../mm/paging.h"
#include "../stdlib/util.h"
#include "../timer/timer.h"

uint8_t current_running_proc = 0;

struct pcb* processes[MAX_PROCESSES] = {0};

void
save_pcb_regs(struct general_registers* regs)
{
    asm("movl %%esp, %0" : "=r" (regs->esp));
    asm("movl $.,    %0" : "=r" (regs->eip));
    asm("movl %%eax, %0" : "=r" (regs->eax));
    asm("movl %%ebx, %0" : "=r" (regs->ebx));
    asm("movl %%ecx, %0" : "=r" (regs->ecx));
    asm("movl %%edx, %0" : "=r" (regs->edx));
    asm("movl %%esi, %0" : "=r" (regs->esi));
    asm("movl %%edi, %0" : "=r" (regs->edi));
    asm("movl %%ebp, %0" : "=r" (regs->ebp));
}

void
restore_pcb_regs(struct general_registers* regs)
{
    asm("mov %0, %%esp" : : "r"(regs->esp));
    // asm("mov %0, %%eip" : : "r"(regs->eip));
    asm("mov %0, %%eax" : : "r"(regs->eax));
    asm("mov %0, %%ebx" : : "r"(regs->ebx));
    asm("mov %0, %%ecx" : : "r"(regs->ecx));
    asm("mov %0, %%edx" : : "r"(regs->edx));
    asm("mov %0, %%esi" : : "r"(regs->esi));
    asm("mov %0, %%edi" : : "r"(regs->edi));
    asm("mov %0, %%ebp" : : "r"(regs->ebp));
}

uint8_t
find_next_pid(void)
{
    for (uint8_t i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i] == 0) {
            return i;
        }
    }

    /* FIXME: Change this */
    return MAX_PROCESSES + 1;
}

uint8_t
create_process(void (*runner)())
{
    struct pcb* proc = kmalloc(sizeof(struct pcb));
    struct general_registers* regs = kmalloc(sizeof(struct general_registers));

    memset(regs, 0, sizeof(struct general_registers));

    proc->pid = find_next_pid();
    proc->used_quantum_ms = 0;
    kprintf("Creating dir\n");
    proc->page_directory = create_page_directory();
    kprintf("after Creating dir\n");

    proc->runner = runner;

    proc->state = PROCESS_READY;

    proc->regs = regs;

    processes[proc->pid] = proc;
    
    return 0;
}

uint8_t
destroy_process(struct pcb* proc)
{
    struct general_registers* regs = proc->regs;
    uint8_t pid = proc->pid;

    kfree(regs);
    kfree(proc);

    processes[pid] = 0;

    return 0;
}

uint8_t
context_switch(struct pcb* old_proc, struct pcb* new_proc)
{
    kprintf("old ebx: 0x%x\tnew ebx: 0x%x\n", old_proc->regs->ebx, new_proc->regs->ebx);
    static int first = 1;
    kprintf("Saving old regs\n");
    save_pcb_regs(old_proc->regs);
    old_proc->state = PROCESS_READY;
    old_proc->used_quantum_ms = 0;
    kprintf("Loading new regs\n");
    if (first) {
        restore_pcb_regs(old_proc->regs);
        first = 0;
    }
    else
        restore_pcb_regs(new_proc->regs);
    kprintf("Running\n");
    new_proc->runner();
}

void
test()
{
    kprintf("Hello!\n");
}

void
test2()
{
    kprintf("Hello2!\n");
}

void
scheduler(void)
{
    create_process(test);
    create_process(test2);
    kprintf("Switching to new proc...\n");
    context_switch(processes[0], processes[1]);
    kprintf("Switching to back to kernel proc in 2...\n");
    delay(2000);
    kprintf("Switching to back to kernel proc now...\n");
    context_switch(processes[1], processes[0]);
}
