#include <stdint.h>

#include "vga.h"
#include "tty.h"
#include "cmos.h"
#include "../stdlib/util.h"
#include "../io/port_io.h"

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
volatile uint16_t* terminal_buffer;

static const uint16_t* VGA_MEMORY_START = (uint16_t*) 0xB8000;

/* FIX: Just VGA_HEIGHT * VGA_WIDTH is NOT enough space,
 * the 8 is a temp fix (probs too big) until I do the math */
struct tty
{
    uint8_t buf[VGA_WIDTH * VGA_HEIGHT * 8];
    uint8_t cursor_x;
    uint8_t cursor_y;
};

static struct tty ttys[3] = {{.cursor_x = 0, .cursor_y = 1}, {.cursor_x = 0, .cursor_y = 1}, {.cursor_x = 0, .cursor_y = 1}};
uint8_t current_tty = 1; /* NOT zero indexed */

void
switch_tty(uint8_t tty)
{
    /* Save current cursor position */
    ttys[current_tty - 1].cursor_x = terminal_column;
    ttys[current_tty - 1].cursor_y = terminal_row;

    /* Flush video memory to current tty buffer */
    memcpy(ttys[current_tty - 1].buf, VGA_MEMORY_START, VGA_WIDTH * VGA_HEIGHT * 8);

    /* Clear video memory */
    clear_screen();

    /* Fill video memory with new tty buffer if its not empty.
     * If the buffer is empty is will write zeros instead of an
     * actual vga entry, this will cause the cursor to not 
     * work correctly */
    if (strlen((char*)ttys[tty - 1].buf) != 0)
        memcpy((void*)VGA_MEMORY_START, ttys[tty - 1].buf, VGA_WIDTH * VGA_HEIGHT * 8);

    current_tty = tty;
    print_header();

    /* Set new tty cursor position */
    terminal_column = ttys[tty - 1].cursor_x;
    terminal_row    = ttys[tty - 1].cursor_y;
    update_cursor();
}

/* FIX: Change this to edit video memory directly */
void
print_header()
{
    // uint8_t old_x = terminal_column;
    // uint8_t old_y = terminal_row;

    move_cursor(0, 0);
    // uint32_t mem_used = curr_free_mem - FREE_MEM_START;
    /* Set to white background */
    terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE));
    puts("ParOS");
    if (current_tty == 1) {
        puts(" [1] 2 3");
    } else if (current_tty == 2) {
        puts(" 1 [2] 3");
    } else if (current_tty == 3) {
        puts(" 1 2 [3]");
    }
    /* 20 is the length of the memory usage string.
     * This makes the top bar white and leaves enough room 
     * for displaying memory used.
     * TODO: Make tty driver better to avoid stuff like this */
    // for (int i = 5; i < (VGA_WIDTH - 20 - get_int_len(mem_used)); i++) {
    for (int i = 13; i < (VGA_WIDTH - 16); i++) {
        // terminal_putentryat(' ', vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE), i, 0);
        puts(" ");
    }

    print_date();
    puts(" ");
    print_time();
    // move_cursor(0, VGA_HEIGHT - 1);
    // for (int i = 0; i < VGA_WIDTH - 1; i++) {
    //     puts(" ");
    // }
    // move_cursor(0, 1);
    // puts("Memory Usage: ");
    // puts(itoa(mem_used, 10));
    // puts(" bytes");
    // puts("\n\n");

    /* Set back to default */
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    // move_cursor(old_x, old_y);
}

void
terminal_initialize(void) 
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_buffer = (volatile uint16_t*) VGA_MEMORY_START;
    /* Sets the hardware cursor to be a block instead of underscore*/
    outb(0x3D4, 0xa);
    outb(0x3D5, 0x0);
    clear_screen();
}

void
terminal_setcolor(uint8_t color) 
{
    terminal_color = color;
}

void
terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

/* Scrolls the terminal by one line */
void
terminal_scroll()
{
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
}

void
clear_screen()
{
    terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_putentryat(' ', terminal_color, x, y);
        }
    }
    move_cursor(0, 1);
    print_header();
}

/* Credit to: https://stackoverflow.com/questions/35601175/adding-support-for-newlines-n-in-c */
void
putch(int c) {
    switch (c) {
        case '\n':
            terminal_column = 0;
            terminal_row++;
            break;
        case '\r':
            terminal_column = 0;
            break;
        case '\t':
            /* Add option to change tab width */
            terminal_column = (terminal_column + 4) / 4 * 4;
            if (terminal_column >= VGA_WIDTH) {
                terminal_column -= VGA_WIDTH;
                terminal_row++;
            }
            break;
        case '\f':
            /* Erase Screen */
            clear_screen();
            terminal_column = terminal_row = 0;
            break;
        case '\a':
            /* Beep */
            break;
        case 0x08: /* Backspace */
            delch();
            break;
        default:
            terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
            if (++terminal_column == VGA_WIDTH) {
                terminal_column = 0;
                ++terminal_row;
            }
            break;
    }
    if (terminal_row == VGA_HEIGHT) {
        terminal_scroll();
        --terminal_row;
    }
    update_cursor();
}

/* TODO: Maybe return the character deleted? */
void
delch(void)
{
    if (terminal_column != 0) {
        move_cursor(terminal_column - 1, terminal_row);
    } else {
        move_cursor(VGA_WIDTH - 1, terminal_row - 1);
    }
    terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
}

void
delrow(void)
{
    size_t old_column = terminal_column;

    if (old_column == 0 && terminal_row != 0) {
        /* TODO: Find where end of text is on each line and make the x position that */
        move_cursor(terminal_column, terminal_row - 1);
    }

    for (size_t i = 0; i < old_column; i++) {
        delch();
    }
}

void
puts(const char* data) 
{
    for (size_t i = 0; i < strlen(data); i++)
        putch(data[i]);
}

void
update_cursor(void)
{
    size_t index = terminal_row * VGA_WIDTH + terminal_column;

    outb(0x3D4, 14);
    outb(0x3D5, index >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, index);
}

void
move_cursor(int x, int y)
{
    terminal_column = x;
    terminal_row    = y;
    update_cursor();
}

void
move_cursor_up(int num)
{
    move_cursor(terminal_column, terminal_row -= num);
}

void
move_cursor_down(int num)
{
    move_cursor(terminal_column, terminal_row += num);
}

void
move_cursor_left(int num)
{
    move_cursor(terminal_column -= num, terminal_row);
}

void
move_cursor_right(int num)
{
    move_cursor(terminal_column += num, terminal_row);
}
