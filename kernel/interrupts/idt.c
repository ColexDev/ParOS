#include <stdint.h>
#include <stddef.h>

#include "idt.h"
#include "../io/printf.h"

struct idt_pointer idtp;
struct idt_entry idt[256];

isr_handler_t isrs[256];

void 
idt_set_gate(uint8_t num, interrupt_handler_t handler, uint16_t selector, uint8_t flags, int userspace)
{
	uintptr_t base = (uintptr_t)handler;
	idt[num].base_low  = (base) & 0xFFFF;
	idt[num].base_mid  = (base >> 16) & 0xFFFF;
	idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
	idt[num].selector = selector;
	idt[num].zero = 0;
	idt[num].pad = 0;
	idt[num].flags = flags | (userspace ? 0x60 : 0);
}

void
idt_reload(void)
{
	asm volatile (
		"lidt %0"
		: : "m"(idtp)
	);
}

void
idt_init(void)
{
    idtp.limit = sizeof(idt) - 1;
    idtp.base = (uintptr_t)&idt;

	idt_set_gate(0,  _isr0,  0x28, 0x8E, 0);
	idt_set_gate(1,  _isr1,  0x28, 0x8E, 0);
	idt_set_gate(2,  _isr2,  0x28, 0x8E, 0);
	idt_set_gate(3,  _isr3,  0x28, 0x8E, 0);
	idt_set_gate(4,  _isr4,  0x28, 0x8E, 0);
	idt_set_gate(5,  _isr5,  0x28, 0x8E, 0);
	idt_set_gate(6,  _isr6,  0x28, 0x8E, 0);
	idt_set_gate(7,  _isr7,  0x28, 0x8E, 0);
	idt_set_gate(8,  _isr8,  0x28, 0x8E, 0);
	idt_set_gate(9,  _isr9,  0x28, 0x8E, 0);
	idt_set_gate(10, _isr10, 0x28, 0x8E, 0);
	idt_set_gate(11, _isr11, 0x28, 0x8E, 0);
	idt_set_gate(12, _isr12, 0x28, 0x8E, 0);
	idt_set_gate(13, _isr13, 0x28, 0x8E, 0);
	idt_set_gate(14, _isr14, 0x28, 0x8E, 0);
	idt_set_gate(15, _isr15, 0x28, 0x8E, 0);
	idt_set_gate(16, _isr16, 0x28, 0x8E, 0);
	idt_set_gate(17, _isr17, 0x28, 0x8E, 0);
	idt_set_gate(18, _isr18, 0x28, 0x8E, 0);
	idt_set_gate(19, _isr19, 0x28, 0x8E, 0);
	idt_set_gate(20, _isr20, 0x28, 0x8E, 0);
	idt_set_gate(21, _isr21, 0x28, 0x8E, 0);
	idt_set_gate(22, _isr22, 0x28, 0x8E, 0);
	idt_set_gate(23, _isr23, 0x28, 0x8E, 0);
	idt_set_gate(24, _isr24, 0x28, 0x8E, 0);
	idt_set_gate(25, _isr25, 0x28, 0x8E, 0);
	idt_set_gate(26, _isr26, 0x28, 0x8E, 0);
	idt_set_gate(27, _isr27, 0x28, 0x8E, 0);
	idt_set_gate(28, _isr28, 0x28, 0x8E, 0);
	idt_set_gate(29, _isr29, 0x28, 0x8E, 0);
	idt_set_gate(30, _isr30, 0x28, 0x8E, 0);
	idt_set_gate(31, _isr31, 0x28, 0x8E, 0);

	idt_set_gate(32, _isr32, 0x28, 0x8E, 1); /* PIT System Clock IRQ 0 */
	idt_set_gate(33, _isr33, 0x28, 0x8E, 1); /* PS2 Keyboard IRQ */
	idt_set_gate(128, _isr128, 0x08, 0x8E, 1); /* Legacy system call entry point, called by userspace. */

    idt_reload();
}

void
isr_install(uint8_t index, isr_handler_t isr)
{
    isrs[index] = isr;
}

struct regs*
isr_handler(struct regs* r)
{
    isr_handler_t handler = isrs[r->int_no];

    /* Exception */
    if (r->int_no < 32) {
        if (handler != NULL) {
            handler(r);
        } else {
            kprintf("UNHANDLED EXCEPTION: %d\n", r->int_no);
        }
    }

    /* Interrupt */
    if (r->int_no >= 32) {
        if (handler != NULL) {
            handler(r);
        } else {
            kprintf("UNHANDLED INTERRUPT: %d\n", r->int_no);
        }
    }
}
