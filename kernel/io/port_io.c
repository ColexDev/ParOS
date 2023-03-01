#include <stdint.h>

/* Send Byte */
void
outb(uint16_t port, uint8_t data)
{
	asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
	return;
}

/* Receive Byte */
uint8_t
inb(uint16_t port)
{
	uint8_t res;
	asm volatile("inb %1, %0" : "=a"(res) : "Nd"(port));
	return res;
}

void
io_wait(void)
{
    outb(0x80, 0); /* 0x80 is an unused port */
}
