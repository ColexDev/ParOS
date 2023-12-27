#include <stdint.h>

#include "idt.h"

static struct idtr idt_reg;
static struct idt_entry idt[256];

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
    asm volatile ("sti"); /* enable interrupts */
}
