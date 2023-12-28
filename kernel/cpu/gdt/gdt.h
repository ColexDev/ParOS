#ifndef GDT_H
#define GDT_H

#include <stdint.h>

typedef struct {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) gdtr_t;

typedef struct {
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access;
    uint8_t limit1 : 4;
    uint8_t flags : 4;
    uint8_t base2;
} __attribute__((packed)) gdt_entry_t;

void gdt_init();

#endif /* GDT_H */
