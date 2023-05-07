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
#include "shell/shell.h"

void crash_me();
void kernel_panic();
void disable_blinking();
extern uint64_t kernel_end;
extern uint64_t kernel_start;

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
    write_fs_header();

    /* I think this is just wrong? */
    // kprintf("Kernel Size: %d bytes\n", &kernel_end - &kernel_start);

    shell_loop(mbi);

    for(;;);
}
