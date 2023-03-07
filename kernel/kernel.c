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
#include "multiboot.h"

void crash_me();
void kernel_panic();
extern uint32_t kernel_end;

#define FREE_MEM_START 0x100000

uint32_t curr_free_mem = FREE_MEM_START;

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
    clear_screen();
    print_header();

    uint32_t i = mbi->mmap_addr;
    char buf[100];
    while (i < mbi->mmap_addr + mbi->mmap_length) {
        memset(buf, 0, 100);
        multiboot_memory_map_t *me = (multiboot_memory_map_t*) i;
        puts("Starts at: ");
        itoa(me->addr_low, buf, 16);
        puts(buf);
        puts(" | ");
        itoa(me->len_low, buf, 16);
        puts(buf);
        puts(" bytes long | Size: ");
        itoa(me->size, buf, 16);
        puts(buf);
        puts(" | Type: ");
        itoa(me->type, buf, 10);
        puts(buf);
        puts("\n");

        i += me->size + sizeof(uint32_t);
    }
    puts("Kernel size: ");
    itoa((mbi->mem_upper - mbi->mem_lower + FREE_MEM_START) / 1000, buf, 10);
    puts(buf);
    puts("kb\n");
    curr_free_mem = mbi->mem_upper - mbi->mem_lower + FREE_MEM_START;

    int* buffer = (int*)kmalloc(1000000);
    buffer[0] = 5;
    buffer[1] = 19;
    buffer[2] = 20;
    buffer[3] = 6;

    /* WORLDS MOST BASIC SHELL */
    char shell_buf[50];
    puts("\n$ ");
    for (;;) {
        char c = 0;
        while (!c) {
            c = getchar();
        }
        if (c == '\n') {
            puts("\n");
            if (!kstrcmp(shell_buf, "ping"))
                puts("pong!\n");
            if (!kstrcmp(shell_buf, "clear"))
                clear_screen();
            if (!kstrcmp(shell_buf, "credits"))
                puts("\tParOS\n\tBy: ColexDev\n");
            if (!kstrcmp(shell_buf, "exit"))
                break;
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
