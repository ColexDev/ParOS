#include <stdint.h>

#include "bitmap.h"

void
set_bit(uint8_t* bitmap, uint32_t n)
{
    bitmap[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

void
clear_bit(uint8_t* bitmap, uint32_t n)
{
    bitmap[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n));
}

uint8_t
get_bit(uint8_t* bitmap, uint32_t n)
{
    uint8_t ret = bitmap[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n));
    return ret != 0;
}

/* TODO: Make a bitmap struct that holds size as well to avoid passing it */
uint32_t
find_first_free_bit(uint8_t* bitmap, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        uint8_t byte = bitmap[i];

        /* Move on if no 0 bits in byte */
        if (byte == 0xFF)
            continue;

        /* Get rightmost 0 bit (free frame) */
        uint8_t offset = __builtin_ctz(~byte);

        /* Finds the frame number */
        return i * WORD_LENGTH + offset;
    }

    return 0;
}

uint32_t
find_first_n_free_bits(uint8_t* bitmap, uint32_t size, uint32_t n)
{
    uint32_t start_bit = 0;
    uint32_t free_bits = 0;

    /* Goes through each individual bit */
    for (uint32_t i = 0; i < size * 8; i++) {
        /* The bit is already taken */
        if (get_bit(bitmap, i)) {
            start_bit = i + 1;
            free_bits = 0;
            continue;
        }

        free_bits++;

        /* Found it! */
        if (free_bits == n) {
            return (WORD_OFFSET(start_bit) << 3) + BIT_OFFSET(start_bit);
        }
    }

    return 0;
}
