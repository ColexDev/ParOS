#ifndef BL_H
#define BL_H

#include <stdint.h>

uint64_t bl_get_hhdm_offset(void);
uint64_t bl_get_kernel_virt_addr(void);
uint64_t bl_get_kernel_phys_addr(void);
uint64_t bl_get_rsdp_addr(void);

#endif /* BL_H */
