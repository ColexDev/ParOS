#include <stddef.h>

#include "irq.h"
#include "pic.h"
#include "../io/port_io.h"
#include "../drivers/tty.h"
#include "../stdlib/util.h"

irq_handler_ptr irq_handlers[16];

void
irq_handler(struct registers* regs)
{
    int irq = regs->interrupt - 0x20;

    if (irq_handlers[irq] == NULL) {
        puts("Unhandled IRQ: ");
        // puts(itoa(irq, 10));
        puts("\n");
    } else {
        irq_handlers[irq](regs);
    }

    pic_send_EOI(irq);
}

void
irq_install(void) 
{
    pic_configure(0x20, 0x20 + 8);

    for (int i = 0; i < 16; i++) {
        isr_register_handler(0x20 + i, irq_handler);
    }

    enable_interrupts();
}

void irq_register_handler(int irq, irq_handler_ptr handler)
{
    irq_handlers[irq] = handler;
}
