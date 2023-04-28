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
#include "mm/kheap.h"
#include "io/port_io.h"
#include "drivers/disk.h"
#include "fs/fs.h"

void crash_me();
void kernel_panic();
void disable_blinking();
extern uint64_t kernel_end;
extern uint64_t kernel_start;

uint32_t curr_free_mem;

void
run_shell(multiboot_info_t* mbi)
{
    char shell_buf[100] = {0};
    uint8_t prompt_start = 0;

    puts("$ ");
    prompt_start = get_cursor_x();

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
            } else if (!kstrcmp(shell_buf, "ls")) {
                list_files();
            } else if (!kstrcmp(shell_buf, "exit")) {
                break;
            } else {
                puts("Error: Command not found\n");
            }
            memset(shell_buf, 0, strlen(shell_buf));
            puts("$ ");
        } else if (c == 8) { /* Backspace */
            /* Cannot delete prompt */
            if (get_cursor_x() == prompt_start)
                continue;
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
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        kernel_panic();

    terminal_initialize();
    clear_screen();
    gdt_install();
    idt_install();
    isr_install();
    irq_install();
    timer_install();
    keyboard_install();
    // disable_blinking();
    pmm_init();
    print_header();
    init_paging();

    uint8_t contents[512];
    uint8_t string[50] = "THIS IS A TEST OF THE FILE SYSTEM";

    read_fs_header();
    create_file("test.txt");
    create_file("test2.txt");
    create_file("test3.txt");
    kprintf("OPENING %s...\n", "test2.txt");
    uint32_t fd = open_file("test2.txt");
    write_file(fd, string, strlen(string));
    read_file(fd, contents, get_file_size(fd));
    kprintf("CONTENTS OF FILE %s:\n\t %s\n", "test2.txt", contents);
    write_fs_header();

    // char* output;
    // output = kstrtok(string, " ");
    //
    // while (output != NULL) {
    //     kprintf(" %s\n", output);
    //     output = kstrtok(NULL, " ");
    // }

    /* I think this is just wrong? */
    // kprintf("Kernel Size: %d bytes\n", &kernel_end - &kernel_start);

    run_shell(mbi);

    for(;;);
}
