#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

struct stack_frame
{
    struct stack_frame* prev_rbp;
    uint64_t return_addr;
};

void print_registers(void);
void printf_stack_trace(void);

#endif /* DEBUG_H */
