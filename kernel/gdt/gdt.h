#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define GDT_CODE_SEGMENT 0x08
#define GDT_DATA_SEGMENT 0x10

struct __attribute__((packed)) gdt_entry
{
    uint16_t limit_low;      /* Bits 0-15 */
    uint16_t base_low;       /* Bits 0-15 */
    uint8_t  base_middle;    /* Bits 16-23 */
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;      /* Bits 24-31 */
};

struct __attribute__((packed)) gdt_entry_ptr
{
    uint16_t limit;
    struct gdt_entry* ptr;
};

void gdt_install(void);
extern void gdt_flush(void);

#endif /* #ifndef GDT_H */
