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
#include "proc/proc.h"

void crash_me();
void kernel_panic();
void disable_blinking();

void
text_editor(char* file)
{
    uint8_t fd = open_file(file, FILE_APPEND_FLAG);
    /* extra 1 for \0 */
    uint32_t size = get_file_size(fd) + 1 + 5;
    uint32_t old_size = size;
    char c = 0;
    uint8_t cursor_x, cursor_y;

    char* contents = kmalloc(sizeof(char) * size);

    read_file(fd, contents, size);
    clear_screen();
    kprintf("%s", contents);

    for(;;) {
        const uint16_t index = (get_cursor_y() - 1) * VGA_WIDTH + get_cursor_x();
        c = 0;
        while (!c) {
            c = getchar();
        }
        switch (c) {
            case 'j':
                move_cursor_down(1);
                break;
            case 'k':
                move_cursor_up(1);
                break;
            case 'h':
                move_cursor_left(1);
                break;
            case 'l':
                move_cursor_right(1);
                break;
            default:
                cursor_x = get_cursor_x();
                cursor_y = get_cursor_y();

                for (int i = size - 1; i >= index; i--) {
                    contents[i + 1] = contents[i];
                }

                contents[index] = c;
                size++;
                break;
        }
        if (size != old_size) {
            clear_screen();
            kprintf("%s", contents);
            kprintf("%d\n", index);
            move_cursor(cursor_x, cursor_y);
        }

        old_size = size;
    };
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
    pmm_init(mbi);
    init_paging();

    // kprintf("VGA BIT AFTER INIT: %d\n", pmm_get_frame(0xB8));
    // for (uint32_t i = 0; i < 0x100; i++)
    //     if (pmm_alloc_frame() == 0xB8000)
    //         kprintf("AHHHHHHHH BADDDDDDDDDDDDDDDDDDDDD\n");
    // pmm_set_frame(0xB8);
    // kprintf("BIT BEFORE BEFORE of addr 0x%x: %d\n", (0xB8 * 0x1000), pmm_get_frame(0xB8));
    // pmm_clear_frame(0xB8);
    // kprintf("BIT AFTER CLEAR of addr 0x%x: %d\n", (0xB8 * 0x1000), pmm_get_frame(0xB8));
    // for (uint32_t i = 0; i < 0xB9; i++) {
    //     uint32_t frame = pmm_alloc_frame();
    //     if (frame == 0xB8000) {
    //         kprintf("BIT BEFORE of addr 0x%x: %d\n", frame, pmm_get_frame(frame / 0x1000));
    //     }
    //     kprintf("BIT AFTER of addr 0x%x: %d\n", frame, pmm_get_frame(frame / 0x1000));
    //     // kprintf("Allocing pmm frame 0x%x\n", frame);
    // }

    /* I think this is just wrong? */
    // kprintf("Kernel Size: %d bytes\n", &kernel_end - &kernel_start);

    // scheduler();
    shell_loop(mbi);

    for(;;);
}
