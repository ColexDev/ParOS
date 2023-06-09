#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#define MAX_PROCESSES 8

#define MAX_QUANTUM_MS 100

enum PROCESS_STATES
{
    PROCESS_RUNNING,
    PROCESS_READY,
    PROCESS_BLOCKED
};

struct general_registers
{
    void*    esp;
    void*    eip;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
};

struct pcb
{
    uint8_t   pid;
    uint8_t   used_quantum_ms;
    uint32_t* page_directory;

    void (*runner)(void);

    enum PROCESS_STATES state;

    struct general_registers* regs;
};

void scheduler(void);
uint8_t context_switch(struct pcb* old_proc, struct pcb* new_proc);
uint8_t create_process(void (*runner)());

#endif
