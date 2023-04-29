#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

#define WORD_LENGTH 8

#define WORD_OFFSET(b) ((b) / WORD_LENGTH)
#define BIT_OFFSET(b)  ((b) % WORD_LENGTH)

void set_bit(uint8_t* bitmap, uint32_t n);
void clear_bit(uint8_t* bitmap, uint32_t n);
uint8_t get_bit(uint8_t* bitmap, uint32_t n);
uint32_t find_first_free_bit(uint8_t* bitmap, uint32_t size);
uint32_t find_first_n_free_bits(uint8_t* bitmap, uint32_t size, uint32_t n);

#endif
