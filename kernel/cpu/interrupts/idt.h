#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define INT_GATE 0x8E /* p=1, dpl=0b00, type=0b1110 */

#define KERNEL_MODE_CODE_SEGMENT 0x08

struct idt_entry
{
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t  ist;   /* only first 3 bits (0-2) are the ist, rest held low */
    uint8_t  flags; /* bit 12 is held low */
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct idtr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void idt_init(void);
void idt_register_entry(uint8_t vector, void* handler, uint8_t flags);

#endif /* IDT_H */
