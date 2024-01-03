#ifndef MEMMAP_H
#define MEMMAP_H

#include <stdint.h>

uint64_t memmap_get_highest_usable_address(void);
void memmap_print(void);

#endif /* MEMMAP_H */
