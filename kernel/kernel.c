#include <stdint.h>

#include "interrupts/pic.h"
#include "drivers/tty.h"
#include "stdlib/util.h"
#include "gdt/gdt.h"
#include "interrupts/idt.h"
#include "interrupts/isr.h"
#include "interrupts/irq.h"
#include "timer/timer.h"
#include "drivers/keyboard.h"
#include "drivers/vga.h"

void crash_me();

/* What value should this be? one less 0 made it so I could only "allocate" 978 bytes due to overwriting the kernel*/
#define FREE_MEM_START 0x1000000

uint32_t curr_free_mem = FREE_MEM_START;

void
kernel_main(void) 
{
    /* Initialize terminal interface */
    terminal_initialize();
    gdt_install();
    idt_install();
    isr_install();
    irq_install();
    timer_install();
    keyboard_install();
    clear_screen();
    print_header();

    int* buffer = (int*)kmalloc(10);
    buffer[0] = 5;
    buffer[1] = 19;
    buffer[2] = 20;
    buffer[3] = 6;
    // for (int i = 0; i <= 3; i++) {
    //     puts(itoa(buffer[i], 10));
    //     puts("\n");
    // }

    print_header();
    // delay(1000);
    // puts("One second has passed...\n");

    // crash_me();

    for(;;);
}
