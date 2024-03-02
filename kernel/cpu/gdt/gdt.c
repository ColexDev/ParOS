#include <stdint.h>

#include "gdt.h"

#define GDT_ENTRIES 5

static struct gdtr gdtr;
static uint64_t gdt[GDT_ENTRIES];

extern void gdt_load(struct gdtr* gdtr);

void 
gdt_init()
{
    /* Using precalculated values instead of writing
     * a function to calculate them */
    gdt[0] = 0x0000000000000000; /* Null */
    gdt[1] = 0x00af9b000000ffff; /* 64-bit kernel code */
    gdt[2] = 0x00af93000000ffff; /* 64-bit kernel data */
    gdt[3] = 0x00affb000000ffff; /* 64-bit user code */
    gdt[4] = 0x00aff3000000ffff; /* 64-bit user data */

    gdtr.offset = (uint64_t)gdt;
    gdtr.size = sizeof(gdt) - 1;

    gdt_load(&gdtr);
}
