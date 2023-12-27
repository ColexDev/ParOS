#include <stdint.h>

#include "interrupts.h"
#include "../../io/printf.h"

void
interrupt_handler(struct interrupt_frame* registers)
{
    kprintf("INTERRUPT No: %d; ERROR CODE: %d\n", 
            registers->interrupt_number, registers->error_code);
}
