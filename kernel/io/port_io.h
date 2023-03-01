#ifndef PORT_IO_H
#define PORT_IO_H

#include <stdint.h>

void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
void io_wait(void);

uint8_t __attribute__((cdecl)) enable_interrupts();
uint8_t __attribute__((cdecl)) disable_interrupts();

#endif /* #ifndef PORT_IO_H */
