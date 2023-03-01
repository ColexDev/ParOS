#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct __attribute__((packed)) idt_entry
{
    uint16_t base_low;
    uint16_t segment_selector;
    uint8_t  reserved;
    uint8_t  flags;
    uint16_t base_high;
};

struct __attribute__((packed)) idt_entry_ptr
{
    uint16_t limit;
    struct idt_entry* ptr;
};

typedef enum
{
    IDT_FLAG_GATE_TASK              = 0x5,
    IDT_FLAG_GATE_16BIT_INT         = 0x6,
    IDT_FLAG_GATE_16BIT_TRAP        = 0x7,
    IDT_FLAG_GATE_32BIT_INT         = 0xE,
    IDT_FLAG_GATE_32BIT_TRAP        = 0xF,

    IDT_FLAG_RING0                  = (0 << 5),
    IDT_FLAG_RING3                  = (3 << 5),

    IDT_FLAG_PRESENT                = 0x80,
} IDT_FLAGS;

void idt_install(void);
void idt_disable_gate(int interrupt);
void idt_enable_gate(int interrupt);
void idt_set_gate(int interrupt, void* base, uint16_t segment_descriptor, uint8_t flags);
void __attribute__((cdecl)) idt_load(struct idt_entry_ptr* idt_ptr);

#endif /* #ifndef IDT_H */
