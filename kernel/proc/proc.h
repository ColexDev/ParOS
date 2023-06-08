#ifndef PROC_H
#define PROC_H

#include <stdint.h>

struct pcb
{
    uint8_t   pid;
    uint32_t* page_directory;
    void*     stack_ptr;
    void*     program_counter;
    uint32_t  eax;
    uint32_t  ebx;
    uint32_t  ecx;
    uint32_t  edx;
    uint32_t  esi;
    uint32_t  edi;
    uint32_t  ebp;
};

#endif
