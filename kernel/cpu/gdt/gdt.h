#ifndef GDT_H
#define GDT_H

#include <stdint.h>

struct gdtr
{
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

void gdt_init();

#endif /* GDT_H */
