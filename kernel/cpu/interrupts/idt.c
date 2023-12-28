#include <stdint.h>

#include "idt.h"
#include "../../io/printf.h"

static struct idtr idt_reg;
static struct idt_entry idt[256];

extern void isr0(void);

void dump_idt() {
    kprintf("IDT dump:\n");

    for (int i = 0; i < 1; i++) {
        struct idt_entry entry = idt[i];

        kprintf("Entry %d:\n", i);
        kprintf("  Offset Low:  %04x\n", entry.offset_low);
        kprintf("  Selector:    %04x\n", entry.segment_selector);
        kprintf("  IST:         %02x\n", entry.ist);
        kprintf("  Flags:       %02x\n", entry.flags);
        kprintf("  Offset Mid:  %04x\n", entry.offset_mid);
        kprintf("  Offset High: %08x\n", entry.offset_high);
        kprintf("  Reserved:    %08x\n", entry.reserved);
        kprintf("  isr:         %lx\n", isr0);
        kprintf("  idt:         %lx\n", idt);
        kprintf("\n");
    }
}

void
idt_register_entry(uint8_t vector, void* handler, uint8_t flags)
{
    idt[vector] = (struct idt_entry){
        .offset_low  = (uint64_t)handler & 0xFFFF,
        .offset_mid  = ((uint64_t)handler >> 16) & 0xFFFF,
        .offset_high = ((uint64_t)handler >> 32) & 0xFFFFFFFF,

        .flags = flags,
        .segment_selector = KERNEL_MODE_CODE_SEGMENT,

        .ist      = 0,
        .reserved = 0,
    };
}

static void
idt_reload(void)
{
    idt_reg = (struct idtr) {
        .base  = (uint64_t)idt,
        .limit = sizeof(idt) - 1
    };

    asm volatile ("lidt %0" :: "m"(idt_reg));
}


void
idt_init(void)
{
    idt_reload();
    idt_register_entry(0, isr0, INT_GATE);
    asm volatile ("sti"); /* enable interrupts */
}
