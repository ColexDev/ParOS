#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

#define PORT 0x3f8  /* COM1 */

uint8_t serial_init();
void serial_write(const uint8_t* str);
void serial_write_char(uint8_t a);

#endif 
