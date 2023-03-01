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

/* What value should this be? one less 0 made it so I could only "allocate" */
#define FREE_MEM_START 0x1000000

uint32_t curr_free_mem = FREE_MEM_START;

void
print_header()
{
    move_cursor(0, 0);
    uint32_t mem_used = curr_free_mem - FREE_MEM_START;
    /* Set to white background */
    terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE));
    puts("ParOS");
    for (int i = 5; i < (VGA_WIDTH - 20 - get_int_len(mem_used)); i++) {
        puts(" ");
    }
    puts("Memory Usage: ");
    puts(itoa(mem_used, 10));
    puts(" bytes");
    puts("\n\n");

    /* Set back to default */
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
}

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
