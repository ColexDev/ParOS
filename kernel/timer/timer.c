#include <stdint.h>

#include "../io/port_io.h"
#include "../interrupts/isr.h"
#include "../interrupts/irq.h"
#include "../drivers/tty.h"
#include "../stdlib/util.h"

#define FREQUENCY 100

uint64_t timer_ticks = 0;

void
set_timer_phase(uint32_t hz)
{
    uint32_t divisor = 1193180 / hz;
    outb(0x43, 0x36); /* 0x43 is the command register */
    outb(0x40, divisor & 0xFF); /* 0x40 is channel 0 of the PIT */
    outb(0x40, divisor >> 8);
}

void
timer_handler(struct registers* regs)
{
    timer_ticks++;
    if (timer_ticks % FREQUENCY == 0) {
        print_header();
    }
}

void
timer_install()
{
    set_timer_phase(FREQUENCY);
    irq_register_handler(0, timer_handler);
}

void
delay(uint32_t milliseconds)
{
    uint64_t end_time = timer_ticks + (milliseconds / 10);

    while (timer_ticks < end_time) {
        asm volatile ("hlt");
    };
}
