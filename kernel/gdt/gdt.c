#include <stdint.h>

#include "gdt.h"

struct gdt_entry      gdts[3];
struct gdt_entry_ptr  gdt_ptr = {sizeof(gdts) - 1, gdts};

void
gdt_set_gate(uint8_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdts[num].base_low     = (base & 0xFFFF);
    gdts[num].base_middle  = (base >> 16) & 0xFF;
    gdts[num].base_high    = (base >> 24) & 0xFF;

    gdts[num].limit_low    = (limit & 0xFFFF);
    gdts[num].granularity  = (limit >> 16) & 0xFF;

    /* Set bits */
    gdts[num].granularity |= (gran & 0xF0);
    gdts[num].access       = access;
}

void
gdt_install(void)
{
    /* Null Descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* Code segment */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Data segment */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_flush();
}
