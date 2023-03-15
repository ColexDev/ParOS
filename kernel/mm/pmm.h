#ifndef PMM_H
#define PMM_H

#include <stdint.h>
void pmm_init();
void* pmm_request_page_frame();
uint32_t pmm_get_used_memory();
uint32_t pmm_get_reserved_memory();

#endif /* #ifndef PMM_H */
