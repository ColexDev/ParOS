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
#include "drivers/cmos.h"
#include "multiboot.h"
#include "mm/mmap.h"
#include "mm/pmm.h"

void crash_me();
void kernel_panic();
void disable_blinking();
extern uint32_t kernel_end;

uint32_t curr_free_mem;

void
kernel_main(multiboot_info_t* mbi, uint32_t magic) 
{
    /* TODO: make a better kernel panic function */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        kernel_panic();

    /* Initialize terminal interface */
    terminal_initialize();
    gdt_install();
    idt_install();
    isr_install();
    irq_install();
    timer_install();
    keyboard_install();
    print_mmap(mbi); /* This currently is needed to init free mem */
    disable_blinking();
    clear_screen();

    // int* buffer = (int*)kmalloc(1000000);
    // buffer[0] = 5;
    // buffer[1] = 19;
    // buffer[2] = 20;
    // buffer[3] = 6;

    char shell_buf[50];
    puts("$ ");
    for (;;) {
        char c = 0;
        while (!c) {
            c = getchar();
        }
        if (c == '\n') {
            puts("\n");
            if (!kstrcmp(shell_buf, "credits")) {
                puts("\tParOS\n\tBy: ColexDev\n");
            } else if (!kstrcmp(shell_buf, "ping")) {
                puts("pong!\n");
            } else if (!kstrcmp(shell_buf, "clear")) {
                clear_screen();
            } else if (!kstrcmp(shell_buf, "mmap")) {
                print_mmap(mbi);
            } else if (!kstrcmp(shell_buf, "time")) {
                print_time();
                puts("\n");
            } else if (!kstrcmp(shell_buf, "date")) {
                print_date();
                puts("\n");
            } else if (!kstrcmp(shell_buf, "exit")) {
                break;
            } else {
                puts("Error: Command not found\n");
            }
            memset(shell_buf, 0, strlen(shell_buf));
            puts("$ ");
        } else if (c) {
            shell_buf[strlen(shell_buf)] = c;
            putch(c);
        }
    }


    // print_header();
    // delay(1000);
    // puts("One second has passed...\n");

    // crash_me();

    for(;;);
}
