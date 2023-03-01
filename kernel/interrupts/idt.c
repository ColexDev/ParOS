#include "idt.h"
#include <stdint.h>

static struct idt_entry idts[256];

static struct idt_entry_ptr idt_ptr = { sizeof(idts) - 1, idts };

void
idt_set_gate(int interrupt, void* base, uint16_t segment_descriptor, uint8_t flags)
{
    idts[interrupt].base_low = ((uint32_t)base) & 0xFFFF;
    idts[interrupt].segment_selector = segment_descriptor;
    idts[interrupt].reserved = 0;
    idts[interrupt].flags = flags;
    idts[interrupt].base_high = ((uint32_t)base >> 16) & 0xFFFF;
}

void
idt_enable_gate(int interrupt)
{
    idts[interrupt].flags |= IDT_FLAG_PRESENT;
}

void
idt_disable_gate(int interrupt)
{
    idts[interrupt].flags &= ~IDT_FLAG_PRESENT;
}

void
idt_install(void)
{
    idt_load(&idt_ptr);
}
