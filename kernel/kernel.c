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
    char shell_buf[50] = {0};
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
    /* TODO: make a better kernel panic function */
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

    // create_file("test.txt");
    // create_file("test2.txt");
    // clear_sector(73);
    // clear_sector(NODES_LBA_OFFSET);
    // uint8_t* buffer = kmalloc(512);
    // ata_read_sector(NODES_LBA_OFFSET, buffer);
    // struct file_node* test = (struct file_node*)buffer;
    // kprintf("LBA: %d\n", test->start_lba);
    // kprintf("ID: %d\n", test->id);
    // kprintf("NAME: %s\n", test->name);

    // uint8_t* contents = kmalloc(512);
    // ata_read_sector(test->start_lba, contents);
    // kprintf("CONTENTS: %s\n", contents);
    // open_file("test.txt");
    // kprintf("Opening file: %s\n", "test2.txt");
    // open_file("test2.txt");

    read_fs_header();
    create_file("test.txt");
    create_file("test2.txt");
    create_file("test3.txt");
    open_file("test2.txt");
    write_fs_header();

    // contents[0] = 5;
    // contents[1] = 6;
    // kprintf("Address of contents: %p\n", &contents);
    // ata_write_sector(0, contents);
    // kprintf("TESTING\n");
    // uint32_t test1 = 6;

    // uint32_t test = 5;
    // kprintf("TEST 0x%x\n", &test);

    // uint8_t* contents = kmalloc(16);
    // uint8_t contents[512] = "HELLO\n";
    // kprintf("Address of contents: 0x%x\n", contents);
    // ata_write_sector(9, contents);

    /* I think this is just wrong? */
    // kprintf("Kernel Size: %d bytes\n", &kernel_end - &kernel_start);

    // uint32_t* ptr = kmalloc(8);
    // ptr[0] = 1;
    // ptr[1] = 2;
    // kprintf("kmalloc return addr: 0x%x\n", ptr);

    // uint32_t* ptr2 = kmalloc(8);
    // ptr2[0] = 3;
    // ptr2[1] = 4;
    // kprintf("kmalloc 2nd return addr: 0x%x\n", ptr2);

    /* overwrites ptr2 header */
    // ptr[2] = 8; /* overwrites ptr2 magic */
    // ptr[3] = 9; /* overwrites ptr2 size  */

    /* overwites ptr2 first index */
    // ptr[4] = 10;

    // for (uint32_t i = 0; i < 2; i++) {
    //     kprintf("ptr[%d] = %d\tptr2[%d] = %d\n", i, ptr[i], i, ptr2[i]);
    // }

    /* This causes a division by 0 fault if i < 4???
     * for both ptr and ptr2 */
    // for (uint32_t i = 0; i < 3; i++) {
    //     kprintf("&ptr[%d] = 0x%x\n", i, &ptr[i]);
    // }

    // kfree(ptr);
    // kfree(ptr2);

    run_shell(mbi);

    // print_header();
    // delay(1000);
    // puts("One second has passed...\n");

    // crash_me();

    for(;;);
}
