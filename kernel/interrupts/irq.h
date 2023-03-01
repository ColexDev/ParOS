#ifndef IRQ_H
#define IRQ_H

#include "isr.h"

typedef void (*irq_handler_ptr)(struct registers* regs);

void irq_install(void);
void irq_register_handler(int irq, irq_handler_ptr handler);

#endif /* #ifndef IRQ_H */
