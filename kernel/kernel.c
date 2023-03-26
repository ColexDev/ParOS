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
#include "mm/paging.h"

void crash_me();
void kernel_panic();
void disable_blinking();
extern uint64_t kernel_end;

uint32_t curr_free_mem;

void
run_shell(multiboot_info_t* mbi)
{
    char shell_buf[50] = {0};

    puts("$ ");

    for (;;) {
        char c = 0;

        /* TODO: Create fgets function */
        while (!c) {
            c = getchar();
        }

        if (c == '\n') {
            puts("\n");
            if (!kstrcmp(shell_buf, "credits")) {
                puts("\tParOS\n\tBy: ColexDev\n");
            } else if (!kstrcmp(shell_buf, "ping")) {
                puts("pong!\n");
            } else if (!kstrcmp(shell_buf, "panic")) {
                kprintf("Okay... You asked for it...\nPANIC\n");
                kernel_panic();
            } else if (!kstrcmp(shell_buf, "clear")) {
                clear_screen();
            } else if (!kstrcmp(shell_buf, "time")) {
                char* time;
                get_time_string(time);
                kprintf("%s\n", time);
            } else if (!kstrcmp(shell_buf, "date")) {
                char* date;
                get_date_string(date);
                kprintf("%s\n", date);
            } else if (!kstrcmp(shell_buf, "memmap")) {
                parse_multiboot_mmap(mbi);
            } else if (!kstrcmp(shell_buf, "memused")) {
                char buf[32] = {0};
                itoa(pmm_get_used_memory(), buf, 10);
                puts(buf);
                puts(" bytes\n");
            } else if (!kstrcmp(shell_buf, "memreserved")) {
                char buf[32] = {0};
                itoa(pmm_get_reserved_memory(), buf, 10);
                puts(buf);
                puts(" bytes\n");
            } else if (!kstrcmp(shell_buf, "exit")) {
                break;
            } else {
                puts("Error: Command not found\n");
            }
            memset(shell_buf, 0, strlen(shell_buf));
            puts("$ ");
        } else if (c == 8) { /* Backspace */
            shell_buf[strlen(shell_buf) - 1] = '\0';
            delch();
        } else if (c) {
            shell_buf[strlen(shell_buf)] = c;
            putch(c);
        }
    }
}

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
    disable_blinking();
    clear_screen();
    pmm_init();

    kprintf("Kernel Size: %d bytes\n", &kernel_end - (uint64_t*)0x100000);

    init_paging();
    run_shell(mbi);

    // print_header();
    // delay(1000);
    // puts("One second has passed...\n");

    // crash_me();

    for(;;);
}
